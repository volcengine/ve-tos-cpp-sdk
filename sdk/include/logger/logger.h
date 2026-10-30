#pragma once

#include "utils/ConcurrentQueue.h"

#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>

namespace VolcengineTos {

enum AsyncLogLevel { DEBUG, INFO, WARN, ERROR };

enum class LogRollType { NONE, SIZE_BASED, TIME_BASED };

enum class TimeRollInterval { SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400, WEEK = 604800 };

class Logger {
public:
    // 单例模式，确保线程安全
    static Logger& getInstance();

    // 禁止拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // 设置日志级别
    void setAsyncLogLevel(AsyncLogLevel level);

    // 设置日志文件路径
    void setLogFile(const std::string& file_path);

    // Saturation drops messages instead of blocking a network worker. Zero disables enqueue.
    void setMaxQueueSize(size_t max_size);
    bool enabled(AsyncLogLevel level) const noexcept {
        return running_.load(std::memory_order_relaxed) &&
               level >= current_level_.load(std::memory_order_relaxed);
    }
    uint64_t droppedMessages() const noexcept { return dropped_messages_.load(std::memory_order_relaxed); }
    template <typename MakeMessage>
    void debugLazy(MakeMessage make_message) noexcept {
        if (!enabled(DEBUG)) return;
        try { make_message(*this); } catch (...) { ++dropped_messages_; }
    }
    template <typename MakeMessage>
    void infoLazy(MakeMessage make_message) noexcept {
        if (!enabled(INFO)) return;
        try { make_message(*this); } catch (...) { ++dropped_messages_; }
    }

    // 分片相关接口
    void setLogRollType(LogRollType type);
    void setLogMaxFileSize(size_t max_size_mb);
    void setLogTimeRollInterval(TimeRollInterval interval);

    // 日志打印接口（支持可变参数）
    template <typename... Args>
    void debug(Args&&... args) {
        log(DEBUG, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(Args&&... args) {
        log(INFO, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(Args&&... args) {
        log(WARN, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(Args&&... args) {
        log(ERROR, std::forward<Args>(args)...);
    }

    // 销毁日志器（确保后台线程退出+资源清理）
    ~Logger();

private:
    Logger();

    // 日志消息结构体（用于无锁队列存储）
    struct LogMessage {
        AsyncLogLevel level;
        std::string time;  // 毫秒级时间戳
        std::thread::id thread_id;
        std::string content;

        ~LogMessage() = default;
    };

    // 格式化日志内容（核心入队逻辑）
    template <typename... Args>
    void log(AsyncLogLevel level, Args&&... args) noexcept {
        if (!enabled(level)) return;
        try {
            const size_t limit = max_queue_size_.load(std::memory_order_relaxed);
            if (log_queue_.unsafeSize() >= limit) {
                ++dropped_messages_;
                return;
            }
            std::stringstream ss;
            appendLogArgs(ss, std::forward<Args>(args)...);
            std::unique_ptr<LogMessage> msg(new LogMessage());
            msg->level = level;
            msg->time = getCurrentTimeWithMs();
            msg->thread_id = std::this_thread::get_id();
            msg->content = ss.str();

            if (!log_queue_.tryPush(msg.get(), limit)) {
                ++dropped_messages_;
                return;
            }
            msg.release();
            queue_cv_.notify_one();  // 唤醒后台工作线程
        } catch (...) {
            // No allocation, recursive logging, or blocking output on failure.
            ++dropped_messages_;
        }
    }

    static void appendLogArgs(std::stringstream&) {}

    template <typename T, typename... Args>
    static void appendLogArgs(std::stringstream& ss, T&& arg, Args&&... args) {
        ss << std::forward<T>(arg);
        appendLogArgs(ss, std::forward<Args>(args)...);
    }

    // 工具函数：获取当前时间（精确到毫秒，线程安全）
    static std::string getCurrentTimeWithMs();

    // 工具函数：日志级别转字符串
    static std::string AsyncLogLevelToString(AsyncLogLevel level);

    // 后台线程：无锁出队+日志输出（控制台+文件）
    void workerThread();

    // 分片相关工具函数
    void initCurrentFileInfo();
    std::string generateRollFileName() const;
    bool checkAndRollFile();

    // 成员变量
    std::atomic<AsyncLogLevel> current_level_;
    mutable std::mutex mutex_;             // 保护日志级别/队列配置的修改
    ConcurrentQueue<LogMessage> log_queue_;
    std::atomic<uint64_t> dropped_messages_{0};
    std::condition_variable queue_cv_;     // 唤醒工作线程（避免空轮询）
    std::atomic<bool> running_;            // 工作线程运行标志
    std::thread worker_thread_;            // 后台日志处理线程
    std::atomic<size_t> max_queue_size_;

    // 文件输出相关
    std::string log_file_path_;      // 日志文件路径
    std::ofstream log_file_stream_;  // 文件输出流
    mutable std::mutex file_mutex_;  // 保护文件流操作（线程安全）

    // 分片相关成员变量
    LogRollType roll_type_;
    size_t max_file_size_;
    TimeRollInterval time_roll_interval_;
    std::chrono::system_clock::time_point current_file_create_time_;
    size_t current_file_size_;
};

}  // namespace VolcengineTos
