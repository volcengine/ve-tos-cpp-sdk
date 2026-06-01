#pragma once

#include <memory>
#include <string>

#include "HttpRequest.h"
#include "utils/BaseUtils.h"

namespace VolcengineTos {
class CurlStats {
public:
    // 默认构造：保持原有字段默认初始化逻辑
    CurlStats() = default;
    // 拷贝构造（按需添加，支持对象拷贝）
    CurlStats(const CurlStats& other) = default;
    // 移动构造（优化性能，支持对象转移）
    CurlStats(CurlStats&& other) noexcept = default;
    // 析构函数（默认即可，无动态内存需要手动释放）
    ~CurlStats() = default;
    // 拷贝赋值运算符
    CurlStats& operator=(const CurlStats& other) = default;
    // 移动赋值运算符
    CurlStats& operator=(CurlStats&& other) noexcept = default;

    // 可修改引用：用于赋值（如 curl_easy_getinfo 传地址）
    double& getTotalTime() {
        return total_time_;
    }
    double& getRedirectTime() {
        return redirect_time_;
    }
    double& getStartTransferTime() {
        return starttransfer_time_;
    }
    double& getNamelookupTime() {
        return namelookup_time_;
    }
    double& getTlsConnectTime() {
        return tlsconnect_time_;
    }
    double& getConnectTime() {
        return connect_time_;
    }
    long& getPrimaryPort() {
        return primary_port_;
    }
    long& getLocalPort() {
        return local_port_;
    }
    long& getNumConnects() {
        return num_connects_;
    }
    long& getSslVerifyResult() {
        return ssl_verify_result_;
    }
    long& getRetryAfter() {
        return retry_after_;
    }
    double& getSizeDownload() {
        return size_download_;
    }
    double& getSizeUpload() {
        return size_upload_;
    }
    double& getHeaderSize() {
        return header_size_;
    }
    double& getRequestSize() {
        return request_size_;
    }
    double& getContentLengthDownload() {
        return content_length_download_;
    }
    double& getContentLengthUpload() {
        return content_length_upload_;
    }
    double& getPreTransTime() {
        return pretrans_time_;
    }
    int& getStatusCode() {
        return status_code;
    }

    // const 只读引用：用于外部读取（不修改场景）
    double getTotalTime() const {
        return total_time_;
    }
    double getRedirectTime() const {
        return redirect_time_;
    }
    double getStartTransferTime() const {
        return starttransfer_time_;
    }
    double getNamelookupTime() const {
        return namelookup_time_;
    }
    double getTlsConnectTime() const {
        return tlsconnect_time_;
    }
    double getConnectTime() const {
        return connect_time_;
    }
    double getSizeDownload() const {
        return size_download_;
    }
    double getSizeUpload() const {
        return size_upload_;
    }
    double getHeaderSize() const {
        return header_size_;
    }
    double getRequestSize() const {
        return request_size_;
    }
    double getContentLengthDownload() const {
        return content_length_download_;
    }
    double getContentLengthUpload() const {
        return content_length_upload_;
    }
    const std::string& getPrimaryIp() const {
        return primary_ip_;
    }
    long getPrimaryPort() const {
        return primary_port_;
    }
    const std::string& getLocalIp() const {
        return local_ip_;
    }
    long getLocalPort() const {
        return local_port_;
    }
    long getNumConnects() const {
        return num_connects_;
    }
    long getSslVerifyResult() const {
        return ssl_verify_result_;
    }
    long getRetryAfter() const {
        return retry_after_;
    }
    double getPreTransTime() const {
        return pretrans_time_;
    }
    int getStatusCode() const {
        return status_code;
    }

    void setNamelookupTime(uint64_t namelookup_time) {
        namelookup_time_ = namelookup_time;
    }
    void setConnectTime(uint64_t connect_time) {
        connect_time_ = connect_time;
    }
    void setPrimaryIp(const std::string& primary_ip) {
        primary_ip_ = primary_ip;
    }
    void setPrimaryIp(std::string&& primary_ip) noexcept {
        primary_ip_ = std::move(primary_ip);
    }
    void setLocalIp(const std::string& local_ip) {
        local_ip_ = local_ip;
    }
    void setLocalIp(std::string&& local_ip) noexcept {
        local_ip_ = std::move(local_ip);
    }

    std::string toString() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(5);  // 所有double保留2位小数

        // 1. 时间统计（毫秒）
        ss << "CurlStats{" << std::endl;
        ss << "  [action] " << std::endl;
        ss << "    调用方: " << action_ << std::endl;
        ss << "  [时间统计] " << std::endl;
        ss << "    e2e耗时: " << e2e_time_ << " ms" << std::endl;
        ss << "    总耗时: " << total_time_ * 1000 << " ms" << std::endl;
        ss << "    重定向耗时: " << redirect_time_ * 1000 << " ms" << std::endl;
        ss << "    首字节响应耗时: " << starttransfer_time_ * 1000 << " ms" << std::endl;
        ss << "    域名解析耗时: " << namelookup_time_ * 1000 << " ms" << std::endl;
        ss << "    TCP连接耗时: " << connect_time_ * 1000 << " ms" << std::endl;
        ss << "    TLS连接耗时: " << tlsconnect_time_ * 1000 << " ms" << std::endl;
        ss << "    预处理耗时（连接后→传输前）: " << pretrans_time_ * 1000 << " ms" << std::endl;

        // 2. 字节统计（字节）
        ss << "  [字节统计] " << std::endl;
        ss << "    下载字节数（响应体）: " << size_download_ << " B" << std::endl;
        ss << "    上传字节数（请求体）: " << size_upload_ << " B" << std::endl;
        ss << "    响应头部字节数: " << header_size_ << " B" << std::endl;
        ss << "    请求总字节数（头+体）: " << request_size_ << " B" << std::endl;
        ss << "    服务器声明响应体长度（Content-Length）: " << content_length_download_ << " B" << std::endl;
        ss << "    上传请求体长度: " << content_length_upload_ << " B" << std::endl;

        // 3. 连接信息
        ss << "  [连接信息] " << std::endl;
        ss << "    远程主IP: " << (primary_ip_.empty() ? "\"\"" : primary_ip_) << std::endl;
        ss << "    远程端口: " << primary_port_ << std::endl;
        ss << "    本地出口IP: " << (local_ip_.empty() ? "\"\"" : local_ip_) << std::endl;
        ss << "    本地临时端口: " << local_port_ << std::endl;
        ss << "    连接创建总数: " << num_connects_ << " 次" << std::endl;

        // 4. SSL/TLS相关
        ss << "  [SSL相关] " << std::endl;
        ss << "    SSL证书验证结果: " << ssl_verify_result_ << " (0=成功，非0=错误码)" << std::endl;

        // 5. 其他信息
        ss << "  [其他信息] " << std::endl;
        ss << "    服务器建议重试延迟（秒）: " << retry_after_ << " s" << std::endl;
        ss << "}";

        return ss.str();
    }

    void setAction(const std::string& action) {
        action_ = action;
    }

    std::string& getAction() {
        return action_;
    }

    void setE2eStartTime(const int64_t time) {
        e2e_start_time_ = time;
    }

    int64_t getE2eStartTime() const {
        return e2e_start_time_;
    }

    void setE2eTime(const double time) {
        e2e_time_ = time;
    }

    double getE2eTime() const {
        return e2e_time_;
    }

private:
    std::string action_;
    int64_t e2e_start_time_;
    int status_code;

    // 时间类（毫秒）
    double e2e_time_ = 0.0;            // 请求进入时打点
    double total_time_ = 0.0;          // 数据传输耗时
    double redirect_time_ = 0.0;       // 重定向总耗时
    double starttransfer_time_ = 0.0;  // 首字节响应耗时
    double namelookup_time_ = 0;       // 域名解析高精度时间
    double tlsconnect_time_ = 0;
    double connect_time_ = 0;   // 连接高精度时间
    double pretrans_time_ = 0;  // 发起请求前耗时

    // 字节统计类（字节）
    double size_download_ = 0.0;            // 下载响应体总字节数
    double size_upload_ = 0.0;              // 上传请求体总字节数
    double header_size_ = 0.0;              // 响应头部总字节数
    double request_size_ = 0.0;             // 请求总字节数（头部+体）
    double content_length_download_ = 0.0;  // 服务器声明的响应体长度（Content-Length）
    double content_length_upload_ = 0.0;    // 上传请求体长度

    // 连接信息类
    std::string primary_ip_;  // 远程服务器主IP
    long primary_port_ = 0;   // 远程服务器端口
    std::string local_ip_;    // 本地出口IP
    long local_port_ = 0;     // 本地临时端口
    long num_connects_ = 0;   // 本次请求创建的连接总数

    // SSL/TLS 相关
    long ssl_verify_result_ = 0;  // SSL证书验证结果（0=成功）

    // 其他
    long retry_after_ = 0;  // 请求重试次数
};
}  // namespace VolcengineTos
