#pragma once

#include "utils/LockFreeQueue.h"

#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <sstream>
#include <iostream>
#include <fstream>

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

    // 设置最大队列长度（超过则忙等等待）
    void setMaxQueueSize(size_t max_size);

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
    void log(AsyncLogLevel level, Args&&... args) {
        if (level < current_level_) {
            return;  // 低于当前日志级别，直接过滤
        }

        // 1. 格式化日志内容
        std::stringstream ss;
        appendLogArgs(ss, std::forward<Args>(args)...);

        try {
            // 2. 构造日志消息（动态分配，通过无锁队列传递指针）
            auto* msg = new LogMessage();
            msg->level = level;
            msg->time = getCurrentTimeWithMs();
            msg->thread_id = std::this_thread::get_id();
            msg->content = ss.str();

            // 3. 无锁入队（队列满则忙等，yield减少CPU占用）
            while (log_queue_.unsafeSize() >= max_queue_size_) {
                std::this_thread::yield();  // 让出CPU，避免忙等消耗
                if (!running_) {            // 线程退出信号，释放内存
                    delete msg;
                    return;
                }
            }
            log_queue_.push(msg);
            queue_cv_.notify_one();  // 唤醒后台工作线程
        } catch (const std::exception& e) {
            // 捕获标准异常，打印详细错误信息
            std::cout << "unknown log error:" << e.what() << std::endl;
        } catch (...) {
            // 捕获所有非标准异常，防止程序崩溃
            std::cout << "unknown log error." << std::endl;
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
    AsyncLogLevel current_level_;          // 当前日志级别
    mutable std::mutex mutex_;             // 保护日志级别/队列配置的修改
    LockFreeQueue<LogMessage> log_queue_;  // 核心：无锁日志队列
    std::condition_variable queue_cv_;     // 唤醒工作线程（避免空轮询）
    std::atomic<bool> running_;            // 工作线程运行标志
    std::thread worker_thread_;            // 后台日志处理线程
    size_t max_queue_size_;                // 队列最大长度（默认10000）

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
