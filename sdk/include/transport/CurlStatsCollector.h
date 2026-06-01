#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "transport/http/CurlStats.h"
#include "utils/LockFreeQueue.h"  // 依赖提供的无锁队列

namespace VolcengineTos {

// 对象大小分组枚举（1M、10M 为分界）
enum class ObjectSizeTier {
    SMALL,   // < 1MB
    MEDIUM,  // 1MB ~ 10MB
    LARGE,   // > 10MB
    UNKNOWN  // 无上传/下载数据
};

// 时延统计分组键（action + 大小分组 + 传输方向）
struct LatencyGroupKey {
    std::string action;                                // 调用方 action
    ObjectSizeTier size_tier = ObjectSizeTier::SMALL;  // 对象大小分组
    std::string transfer_dir;                          // 传输方向（upload/download/unknown）

    bool operator==(const LatencyGroupKey& other) const {
        return action == other.action && size_tier == other.size_tier && transfer_dir == other.transfer_dir;
    }

    bool operator<(const LatencyGroupKey& other) const {
        if (action != other.action) return action < other.action;
        if (size_tier != other.size_tier) return size_tier < other.size_tier;
        return transfer_dir < other.transfer_dir;
    }
};

// 连接统计分组键（源IP:端口 -> 目的IP:端口）
struct ConnectionKey {
    std::string local_ip_port;   // 本地 IP:端口（如 "192.168.1.1:54321"）
    std::string remote_ip_port;  // 远程 IP:端口（如 "8.8.8.8:443"）

    bool operator==(const ConnectionKey& other) const {
        return local_ip_port == other.local_ip_port && remote_ip_port == other.remote_ip_port;
    }

    bool operator<(const ConnectionKey& other) const {
        if (local_ip_port != other.local_ip_port) return local_ip_port < other.local_ip_port;
        return remote_ip_port < other.remote_ip_port;
    }
};

// 状态码统计键（仅基于action）
struct StatusCodeKey {
    std::string action;

    bool operator<(const StatusCodeKey& other) const { return action < other.action; }
};

// 单时间字段的统计指标（平均、最大、最小、P99）
struct SingleTimeMetric {
    std::vector<std::pair<int64_t, double>> records;  // (时间戳ms, 时间值ms)
    uint64_t count = 0;
    double avg = 0.0;
    double min = 0.0;
    double max = 0.0;
    double p99 = 0.0;

    // 清理过期记录
    void pruneExpired(int64_t current_time_ms) {
        const int64_t EXPIRY_MS = 10 * 60 * 1000;  // 10分钟
        auto it = records.begin();
        while (it != records.end()) {
            if (current_time_ms - it->first > EXPIRY_MS) {
                it = records.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 添加记录
    void addRecord(int64_t timestamp_ms, double time_val) { records.emplace_back(timestamp_ms, time_val); }

    // 计算指标
    void updateMetrics() {
        if (records.empty()) {
            count = 0;
            avg = min = max = p99 = 0.0;
            return;
        }

        count = records.size();
        double sum = 0.0;
        min = records[0].second;
        max = records[0].second;
        std::vector<double> values;
        values.reserve(count);

        for (std::vector<std::pair<int64_t, double>>::const_iterator it = records.begin(); it != records.end();
             ++it) {
            const double val = it->second;
            sum += val;
            values.push_back(val);
            min = std::min(min, val);
            max = std::max(max, val);
        }

        avg = sum / count;
        std::sort(values.begin(), values.end());
        size_t p99_idx = static_cast<size_t>(std::ceil(count * 0.99)) - 1;
        p99_idx = std::min(p99_idx, values.size() - 1);
        p99 = values[p99_idx];
    }
};

// 多时间字段的统计集合（对应8个需要统计的时间）
struct MultiTimeStats {
    SingleTimeMetric e2e;            // 请求进入时打点
    SingleTimeMetric total;          // 数据传输耗时
    SingleTimeMetric redirect;       // 重定向总耗时
    SingleTimeMetric starttransfer;  // 首字节响应耗时
    SingleTimeMetric namelookup;     // 域名解析高精度时间
    SingleTimeMetric tlsconnect;     // TLS连接耗时
    SingleTimeMetric connect;        // TCP连接高精度时间
    SingleTimeMetric pretrans;       // 发起请求前耗时

    // 清理所有时间字段的过期记录
    void pruneExpired(int64_t current_time_ms) {
        e2e.pruneExpired(current_time_ms);
        total.pruneExpired(current_time_ms);
        redirect.pruneExpired(current_time_ms);
        starttransfer.pruneExpired(current_time_ms);
        namelookup.pruneExpired(current_time_ms);
        tlsconnect.pruneExpired(current_time_ms);
        connect.pruneExpired(current_time_ms);
        pretrans.pruneExpired(current_time_ms);
    }

    // 更新所有时间字段的统计指标
    void updateMetrics() {
        e2e.updateMetrics();
        total.updateMetrics();
        redirect.updateMetrics();
        starttransfer.updateMetrics();
        namelookup.updateMetrics();
        tlsconnect.updateMetrics();
        connect.updateMetrics();
        pretrans.updateMetrics();
    }

    // 判断是否无有效记录
    bool isEmpty() const {
        return e2e.records.empty() && total.records.empty() && redirect.records.empty() &&
               starttransfer.records.empty() && namelookup.records.empty() && tlsconnect.records.empty() &&
               connect.records.empty() && pretrans.records.empty();
    }
};

// 连接统计（仅计数，不统计时延）
struct ConnectionMetric {
    std::vector<int64_t> timestamps;  // 记录每次使用的时间戳（用于过期淘汰）
    uint64_t count = 0;               // 连接使用次数（10分钟内有效记录数）

    // 清理过期记录
    void pruneExpired(int64_t current_time_ms) {
        const int64_t EXPIRY_MS = 10 * 60 * 1000;  // 10分钟
        auto it = timestamps.begin();
        while (it != timestamps.end()) {
            if (current_time_ms - *it > EXPIRY_MS) {
                it = timestamps.erase(it);
            } else {
                ++it;
            }
        }
        count = timestamps.size();  // 剩余记录数即为有效使用次数
    }

    // 添加连接使用记录（仅存时间戳）
    void addRecord(int64_t timestamp_ms) {
        timestamps.push_back(timestamp_ms);
        count = timestamps.size();  // 实时更新计数（惰性计算也可，此处简化）
    }

    // 判断是否无有效记录
    bool isEmpty() const { return timestamps.empty(); }
};

struct StatusCodeMetric {
    std::map<int, size_t> code_count;  // 状态码 -> 计数映射
    size_t total_count = 0;            // 该action的总请求数

    void addRecord(int status_code) {
        code_count[status_code]++;
        total_count++;
    }

    void pruneExpired(int64_t current_time) {
        // 状态码统计跟随action的过期一起清理，这里无需单独处理
        // 实际过期逻辑在StatusCodeStats的pruneExpired中
    }

    bool isEmpty() const { return total_count == 0; }
};

// 单例统计管理器（多时间字段+连接仅计数）
class CurlStatsCollector {
 public:
    // 单例获取接口（C++11 静态局部变量线程安全）
    static CurlStatsCollector& getInstance() {
        static CurlStatsCollector instance;
        return instance;
    }

    // 禁止拷贝和赋值（确保全局唯一）
    CurlStatsCollector(const CurlStatsCollector&) = delete;
    CurlStatsCollector& operator=(const CurlStatsCollector&) = delete;

    // 添加 CurlStats 记录（仅入队，无其他计算）
    void addCurlStats(const CurlStats& stats) const {
        // 队列满则丢弃（原子变量判断，无需加锁，不严格限制）
        if (data_queue_.unsafeSize() >= MAX_QUEUE_SIZE) {
            return;
        }
        // 入队（无锁操作，队列存储对象指针）
        data_queue_.push(new CurlStats(stats));
    }

    // 生成统计字符串（触发统计计算+过期淘汰）
    std::string toString() const {
        std::lock_guard<std::mutex> lock(mutex_);  // 保护统计结果结构
        std::stringstream ss;
        ss << std::fixed << std::setprecision(5);  // 保留5位小数

        const auto current_time = getCurrentTimeMs();

        // 1. 消费队列中所有未处理的原始数据
        consumeQueueData();

        // 2. 清理所有过期记录（10分钟）
        pruneAllExpired(current_time);

        // 3. 计算所有统计指标
        updateAllMetrics();

        // 4. 格式化输出统计结果
        printMultiTimeStats(ss);
        printConnectionStats(ss);
        printStatusCodeStats(ss);

        return ss.str();
    }

 private:
    // 私有构造（单例禁止外部创建）
    CurlStatsCollector() = default;
    // 私有析构（单例生命周期由系统管理）
    ~CurlStatsCollector() { clearPendingQueue(); }

    // 队列最大长度（10万条）
    static constexpr size_t MAX_QUEUE_SIZE = 10000;
    // 无锁队列（存储未处理的 CurlStats 指针）
    mutable LockFreeQueue<CurlStats> data_queue_;
    // 统计结果保护锁（仅在 toString 时使用）
    mutable std::mutex mutex_;
    // 多时间字段分组统计结果
    mutable std::map<LatencyGroupKey, MultiTimeStats> multi_time_stats_;
    // 连接统计结果（仅计数）
    mutable std::map<ConnectionKey, ConnectionMetric> connection_stats_;
    // 基于action的状态码统计结果
    mutable std::map<StatusCodeKey, StatusCodeMetric> status_code_stats_;

    // 获取当前毫秒级时间戳
    static int64_t getCurrentTimeMs() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }

    // 转换 ObjectSizeTier 为可读字符串
    static std::string sizeTierToString(ObjectSizeTier tier) {
        switch (tier) {
            case ObjectSizeTier::SMALL:
                return "小(<1MB)";
            case ObjectSizeTier::MEDIUM:
                return "中(1MB~10MB)";
            case ObjectSizeTier::LARGE:
                return "大(>10MB)";
            default:
                return "未知";
        }
    }

    // 根据文件大小判断分组
    static ObjectSizeTier getSizeTier(double size_bytes) {
        const double ONE_MB = 1024 * 1024;  // 1MB = 1048576 字节
        const double TEN_MB = 10 * ONE_MB;  // 10MB = 10485760 字节
        if (size_bytes <= 0) return ObjectSizeTier::UNKNOWN;
        if (size_bytes < ONE_MB) return ObjectSizeTier::SMALL;
        if (size_bytes <= TEN_MB) return ObjectSizeTier::MEDIUM;
        return ObjectSizeTier::LARGE;
    }

    // 判断传输方向（upload/download）
    static std::string getTransferDirection(const CurlStats& stats) {
        if (stats.getSizeUpload() > stats.getSizeDownload()) {
            return "upload";
        }
        return "download";
    }

    // 构建连接键（本地IP:端口 + 远程IP:端口）
    static ConnectionKey buildConnectionKey(const CurlStats& stats) {
        ConnectionKey key;
        // 本地 IP:端口（处理 IP 为空的情况）
        key.local_ip_port = stats.getLocalIp().empty() ? "unknown" : stats.getLocalIp();
        key.local_ip_port += ":" + std::to_string(stats.getLocalPort());
        // 远程 IP:端口（处理 IP 为空的情况）
        key.remote_ip_port = stats.getPrimaryIp().empty() ? "unknown" : stats.getPrimaryIp();
        key.remote_ip_port += ":" + std::to_string(stats.getPrimaryPort());
        return key;
    }

    // 消费队列中所有数据，添加到统计结构
    void consumeQueueData() const {
        while (true) {
            // 无锁出队（队列为空时返回 nullptr）
            CurlStats* data = data_queue_.pop();
            if (!data) {
                break;
            }
            // 处理多时间字段统计
            processMultiTimeStats(*data);
            // 处理连接统计（仅计数）
            processConnectionStats(*data);
            // 处理状态码统计
            processStatusCodeStats(*data);
            // 释放内存（避免泄漏）
            delete data;
        }
    }

    void clearPendingQueue() const {
        while (true) {
            CurlStats* data = data_queue_.pop();
            if (!data) {
                break;
            }
            delete data;
        }
    }

    void processStatusCodeStats(CurlStats& stats) const {
        StatusCodeKey key;
        key.action = stats.getAction();  // 按action分组统计状态码
        const int status_code = stats.getStatusCode();

        // 过滤无效状态码（0通常表示未获取到状态码）
        if (status_code > 0) {
            status_code_stats_[key].addRecord(status_code);
        }
    }

    // 处理单条数据的多时间字段统计
    void processMultiTimeStats(CurlStats& stats) const {
        LatencyGroupKey group_key;
        group_key.action = stats.getAction();
        group_key.transfer_dir = getTransferDirection(stats);

        // 确定对象大小分组
        double object_size = 0.0;
        if (group_key.transfer_dir == "upload") {
            object_size = stats.getSizeUpload();
        } else if (group_key.transfer_dir == "download") {
            object_size = stats.getSizeDownload();
        }
        group_key.size_tier = getSizeTier(object_size);

        // 获取时间戳（统一使用 e2e_start_time_）
        const int64_t timestamp = stats.getE2eStartTime();

        // 为8个时间字段分别添加记录（注意单位转换：curl返回的是秒，转成毫秒）
        auto& stats_entry = multi_time_stats_[group_key];
        stats_entry.e2e.addRecord(timestamp, stats.getE2eTime());  // 已为ms
        stats_entry.total.addRecord(timestamp, stats.getTotalTime() * 1000);
        stats_entry.redirect.addRecord(timestamp, stats.getRedirectTime() * 1000);
        stats_entry.starttransfer.addRecord(timestamp, stats.getStartTransferTime() * 1000);
        stats_entry.namelookup.addRecord(timestamp, stats.getNamelookupTime() * 1000);
        stats_entry.tlsconnect.addRecord(timestamp, stats.getTlsConnectTime() * 1000);
        stats_entry.connect.addRecord(timestamp, stats.getConnectTime() * 1000);
        stats_entry.pretrans.addRecord(timestamp, stats.getPreTransTime() * 1000);
    }

    // 处理单条数据的连接统计（仅计数）
    void processConnectionStats(const CurlStats& stats) const {
        // 构建连接键
        const ConnectionKey conn_key = buildConnectionKey(stats);
        // 添加连接使用记录（时间戳为 e2e_start_time_）
        connection_stats_[conn_key].addRecord(stats.getE2eStartTime());
    }

    // 清理所有过期记录
    void pruneAllExpired(int64_t current_time) const {
        // 清理多时间字段统计
        auto time_it = multi_time_stats_.begin();
        while (time_it != multi_time_stats_.end()) {
            time_it->second.pruneExpired(current_time);
            if (time_it->second.isEmpty()) {
                time_it = multi_time_stats_.erase(time_it);
            } else {
                ++time_it;
            }
        }

        // 清理连接统计
        auto conn_it = connection_stats_.begin();
        while (conn_it != connection_stats_.end()) {
            conn_it->second.pruneExpired(current_time);
            if (conn_it->second.isEmpty()) {
                conn_it = connection_stats_.erase(conn_it);
            } else {
                ++conn_it;
            }
        }

        // 清理状态码统计（通过关联的multi_time_stats判断是否过期）
        auto code_it = status_code_stats_.begin();
        while (code_it != status_code_stats_.end()) {
            const std::string& action = code_it->first.action;
            bool has_valid_records = false;

            // 检查该action是否还有有效的时间统计记录
            for (std::map<LatencyGroupKey, MultiTimeStats>::const_iterator it = multi_time_stats_.begin();
                 it != multi_time_stats_.end(); ++it) {
                const LatencyGroupKey& time_key = it->first;
                const MultiTimeStats& time_stats = it->second;
                if (time_key.action == action && !time_stats.isEmpty()) {
                    has_valid_records = true;
                    break;
                }
            }

            // 如果没有有效记录，删除该action的状态码统计
            if (!has_valid_records) {
                code_it = status_code_stats_.erase(code_it);
            } else {
                ++code_it;
            }
        }
    }

    // 更新所有统计指标
    void updateAllMetrics() const {
        for (std::map<LatencyGroupKey, MultiTimeStats>::iterator it = multi_time_stats_.begin();
             it != multi_time_stats_.end(); ++it) {
            it->second.updateMetrics();
        }
        // 连接统计无需额外计算，prune时已更新count
        // 状态码统计无需额外计算，addRecord时已实时更新计数
    }

    // 打印多时间字段分组统计
    void printMultiTimeStats(std::stringstream& ss) const {
        ss << "\n======================= Curl 多时间字段分组统计 =======================" << std::endl;
        if (multi_time_stats_.empty()) {
            ss << "  [暂无有效统计数据]" << std::endl;
            return;
        }

        for (std::map<LatencyGroupKey, MultiTimeStats>::const_iterator it = multi_time_stats_.begin();
             it != multi_time_stats_.end(); ++it) {
            const LatencyGroupKey& key = it->first;
            const MultiTimeStats& stats = it->second;
            // 跳过无有效记录的分组
            if (stats.isEmpty()) continue;

            ss << "------------------------------------------------------------" << std::endl;
            ss << "  分组信息：" << std::endl;
            ss << "    Action: " << key.action << std::endl;
            ss << "    传输方向: " << key.transfer_dir << std::endl;
            ss << "    对象大小: " << sizeTierToString(key.size_tier) << std::endl;
            ss << "  统计指标（单位：ms）：" << std::endl;

            // 按顺序打印8个时间字段的统计结果
            printSingleTimeMetric(ss, "请求进入总耗时(e2e)", stats.e2e);
            printSingleTimeMetric(ss, "数据传输耗时(total)", stats.total);
            printSingleTimeMetric(ss, "重定向总耗时(redirect)", stats.redirect);
            printSingleTimeMetric(ss, "首字节响应耗时(starttransfer)", stats.starttransfer);
            printSingleTimeMetric(ss, "域名解析耗时(namelookup)", stats.namelookup);
            printSingleTimeMetric(ss, "TLS连接耗时(tlsconnect)", stats.tlsconnect);
            printSingleTimeMetric(ss, "TCP连接耗时(connect)", stats.connect);
            printSingleTimeMetric(ss, "发起请求前耗时(pretrans)", stats.pretrans);
        }
    }

    // 打印单个时间字段的统计（辅助函数）
    static void printSingleTimeMetric(std::stringstream& ss, const std::string& name,
                                      const SingleTimeMetric& metric) {
        if (metric.count == 0) {
            ss << "    " << name << ": 无有效记录" << std::endl;
            return;
        }
        ss << "    " << name << ":" << std::endl;
        ss << "      记录数: " << metric.count << " | 平均: " << metric.avg << " | 最小: " << metric.min
           << std::endl;
        ss << "      最大: " << metric.max << " | P99: " << metric.p99 << std::endl;
    }

    // 打印连接统计（仅计数）
    void printConnectionStats(std::stringstream& ss) const {
        ss << std::endl
           << "======================= Curl 连接统计（仅计数） =======================" << std::endl;
        if (connection_stats_.empty()) {
            ss << "  [暂无有效统计数据]" << std::endl;
            return;
        }

        for (std::map<ConnectionKey, ConnectionMetric>::const_iterator it = connection_stats_.begin();
             it != connection_stats_.end(); ++it) {
            const ConnectionKey& key = it->first;
            const ConnectionMetric& metric = it->second;
            if (metric.count == 0) continue;
            ss << "------------------------------------------------------------" << std::endl;
            ss << "  连接信息：" << std::endl;
            ss << "    本地 -> 远程: " << key.local_ip_port << " -> " << key.remote_ip_port << std::endl;
            ss << "  统计指标：" << std::endl;
            ss << "    10分钟内使用次数: " << metric.count << " 次" << std::endl;
        }
        ss << "============================================================" << std::endl;
    }

    // 输出状态码统计
    void printStatusCodeStats(std::stringstream& ss) const {
        ss << "\n=== 基于Action的状态码统计 ===" << std::endl;
        if (status_code_stats_.empty()) {
            ss << "无状态码统计数据" << std::endl;
            return;
        }

        for (std::map<StatusCodeKey, StatusCodeMetric>::const_iterator it = status_code_stats_.begin();
             it != status_code_stats_.end(); ++it) {
            const StatusCodeKey& key = it->first;
            const StatusCodeMetric& metric = it->second;
            ss << "Action: " << key.action << ", 总请求数: " << metric.total_count << std::endl;

            // 输出各状态码的计数和占比
            for (std::map<int, size_t>::const_iterator code_it = metric.code_count.begin();
                 code_it != metric.code_count.end(); ++code_it) {
                const int code = code_it->first;
                const size_t count = code_it->second;
                const double ratio = (metric.total_count > 0) ? (count * 100.0 / metric.total_count) : 0.0;
                ss << "  状态码 " << code << ": " << count << "次 (" << ratio << "%)" << std::endl;
            }
            ss << std::endl;
        }
    }
};

}  // namespace VolcengineTos
