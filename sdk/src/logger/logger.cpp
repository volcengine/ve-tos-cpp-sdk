#include "logger/logger.h"
#include <iomanip>  // 用于时间格式化
#include <ctime>    // 用于时间转换
#include <sstream>
#include <thread>
#include <fstream>
#include <filesystem>  // 用于文件操作（C++17及以上）

using namespace VolcengineTos;
#ifdef _WIN32
#include <windows.h>  // Windows文件时间获取
#else
#include <sys/stat.h>  // Linux/macOS文件属性获取
#endif

namespace VolcengineTos {

// 单例初始化（C++11静态局部变量初始化线程安全）
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// 构造函数：初始化配置+启动后台线程（新增分片相关初始化）
Logger::Logger()
        : current_level_(INFO),
          running_(true),
          max_queue_size_(10000),                      // 默认最大队列长度10000
          roll_type_(LogRollType::SIZE_BASED),         // 默认大小分片
          max_file_size_(50 * 1024 * 1024),            // 默认单个文件最大50MB
          time_roll_interval_(TimeRollInterval::DAY),  // 默认每天分片
          current_file_size_(0)                        // 当前日志文件大小（字节）
{
    worker_thread_ = std::thread(&Logger::workerThread, this);
}

// 析构函数：优雅退出+资源清理（无修改）
Logger::~Logger() {
    // 1. 通知工作线程退出
    running_ = false;
    queue_cv_.notify_one();  // 唤醒可能阻塞的工作线程
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    // 2. 清理队列中剩余的日志消息（避免内存泄漏）
    LogMessage* remaining_msg = nullptr;
    while ((remaining_msg = log_queue_.pop()) != nullptr) {
        delete remaining_msg;
    }

    // 3. 关闭文件流
    std::lock_guard<std::mutex> file_lock(file_mutex_);
    if (log_file_stream_.is_open()) {
        log_file_stream_.close();
    }
}

// 设置日志级别（线程安全，无修改）
void Logger::setAsyncLogLevel(AsyncLogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

// 设置日志文件路径（线程安全，新增分片初始化逻辑）
void Logger::setLogFile(const std::string& file_path) {
    std::lock_guard<std::mutex> file_lock(file_mutex_);
    log_file_path_ = file_path;

    // 关闭已打开的文件流
    if (log_file_stream_.is_open()) {
        log_file_stream_.close();
    }

    // 以追加模式打开文件（支持多线程写入）
    log_file_stream_.open(file_path, std::ios::app | std::ios::out);
    if (!log_file_stream_.is_open()) {
        std::cerr << "[Logger Error] Failed to open log file: " << file_path << std::endl;
        current_file_size_ = 0;
        return;
    }

    // 初始化当前文件信息：文件大小 + 创建时间
    initCurrentFileInfo();

    std::cout << "[Logger Info] Log file initialized: " << file_path << ", current size: " << current_file_size_
              << " bytes" << std::endl;
}

// 设置最大队列长度（线程安全，无修改）
void Logger::setMaxQueueSize(const size_t max_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_queue_size_ = max_size;
}

// -------------------------- 新增分片相关接口 --------------------------
// 设置日志分片类型（线程安全）
void Logger::setLogRollType(LogRollType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    roll_type_ = type;
    std::cout << "[Logger Info] Log roll type set to: "
              << (type == LogRollType::NONE ? "NONE" : (type == LogRollType::SIZE_BASED ? "SIZE_BASED" : "TIME_BASED"))
              << std::endl;
}

// 设置按大小分片的最大文件大小（单位：MB，线程安全）
void Logger::setLogMaxFileSize(size_t max_size_mb) {
    if (max_size_mb == 0) {
        std::cerr << "[Logger Warning] Max file size cannot be 0, using default 100MB" << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    max_file_size_ = max_size_mb * 1024 * 1024;  // 转换为字节
    std::cout << "[Logger Info] Max log file size set to: " << max_size_mb << "MB" << std::endl;
}

// 设置按时间分片的间隔（线程安全）
void Logger::setLogTimeRollInterval(TimeRollInterval interval) {
    std::lock_guard<std::mutex> lock(mutex_);
    time_roll_interval_ = interval;
    std::string interval_str;
    switch (interval) {
        case TimeRollInterval::SECOND:
            interval_str = "1s";
            break;
        case TimeRollInterval::MINUTE:
            interval_str = "1min";
            break;
        case TimeRollInterval::HOUR:
            interval_str = "1h";
            break;
        case TimeRollInterval::DAY:
            interval_str = "1day";
            break;
        case TimeRollInterval::WEEK:
            interval_str = "1week";
            break;
        default:
            interval_str = "unknown";
    }
    std::cout << "[Logger Info] Log time roll interval set to: " << interval_str << std::endl;
}
// ----------------------------------------------------------------------

// 日志级别转字符串（静态工具函数，无修改）
std::string Logger::AsyncLogLevelToString(const AsyncLogLevel level) {
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

// 获取当前时间（精确到毫秒，跨平台线程安全，无修改）
std::string Logger::getCurrentTimeWithMs() {
    const auto now = std::chrono::system_clock::now();

    // 转换为毫秒级时间戳
    const auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    const auto epoch_ms = now_ms.time_since_epoch();
    const auto sec = std::chrono::duration_cast<std::chrono::seconds>(epoch_ms);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch_ms - sec);

    // 转换为本地时间（线程安全版本）
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);  // Windows线程安全
#else
    localtime_r(&t, &tm_buf);  // Linux/macOS线程安全
#endif

    // 格式化：YYYY-MM-DD HH:MM:SS.sss
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}

// 初始化当前日志文件信息（大小+创建时间）
void Logger::initCurrentFileInfo() {
    current_file_size_ = 0;
#ifdef _WIN32
    // Windows：用GetFileSize获取文件大小（线程安全）
    HANDLE hFile = CreateFileA(log_file_path_.c_str(), GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,  // 允许其他进程读写
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD file_size = GetFileSize(hFile, NULL);
        if (file_size != INVALID_FILE_SIZE) {
            current_file_size_ = static_cast<size_t>(file_size);
        }
        CloseHandle(hFile);
    }
#else
    // Linux/macOS：用stat获取文件大小（线程安全）
    struct stat file_stat;
    if (stat(log_file_path_.c_str(), &file_stat) == 0) {
        current_file_size_ = static_cast<size_t>(file_stat.st_size);
    }
#endif

    // 2. 获取文件创建时间（原有逻辑不变，保持跨平台兼容）
    current_file_create_time_ = std::chrono::system_clock::now();
#ifdef _WIN32
    HANDLE hFileCreate = CreateFileA(log_file_path_.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFileCreate != INVALID_HANDLE_VALUE) {
        FILETIME createTime;
        if (GetFileTime(hFileCreate, &createTime, NULL, NULL)) {
            ULARGE_INTEGER uli;
            uli.LowPart = createTime.dwLowDateTime;
            uli.HighPart = createTime.dwHighDateTime;
            auto file_time_us = std::chrono::microseconds(uli.QuadPart / 10);
            current_file_create_time_ = std::chrono::system_clock::time_point(file_time_us);
        }
        CloseHandle(hFileCreate);
    }
#else
    struct stat file_statw;
    if (stat(log_file_path_.c_str(), &file_statw) == 0) {
        current_file_create_time_ = std::chrono::system_clock::from_time_t(file_stat.st_ctime);
    }
#endif
}

// 生成分片文件名（格式：基名_YYYYMMDD_HHMMSS.ext）
std::string Logger::generateRollFileName() const {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    // 格式化时间戳：YYYYMMDD_HHMMSS（避免文件名重复）
    std::stringstream timestamp_ss;
    timestamp_ss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    std::string timestamp = timestamp_ss.str();

    // 分离原文件名的基名和扩展名（例：log.txt -> 基名log，扩展名txt）
    std::string base_name = log_file_path_;
    std::string ext = "";
    size_t dot_pos = log_file_path_.find_last_of('.');
    if (dot_pos != std::string::npos) {
        base_name = log_file_path_.substr(0, dot_pos);
        ext = log_file_path_.substr(dot_pos);  // 包含小数点
    }

    // 生成最终分片文件名
    return base_name + "_" + timestamp + ext;
}

// 检查是否需要分片并执行切换（线程安全，需在file_mutex_保护下调用）
bool Logger::checkAndRollFile() {
    if (roll_type_ == LogRollType::NONE || !log_file_stream_.is_open()) {
        return false;
    }

    bool need_roll = false;
    auto now = std::chrono::system_clock::now();

    // 1. 按大小分片检查
    if (roll_type_ == LogRollType::SIZE_BASED) {
        if (current_file_size_ >= max_file_size_) {
            need_roll = true;
            std::cout << "[Logger Info] Log file size reaches limit (" << max_file_size_ / 1024 / 1024
                      << "MB), ready to roll" << std::endl;
        }
    }
    // 2. 按时间分片检查
    else if (roll_type_ == LogRollType::TIME_BASED) {
        auto interval_sec = static_cast<int64_t>(time_roll_interval_);
        auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(now - current_file_create_time_).count();
        if (elapsed_sec >= interval_sec) {
            need_roll = true;
            std::cout << "[Logger Info] Log time interval reaches limit (" << interval_sec << "s), ready to roll"
                      << std::endl;
        }
    }

    // 执行分片切换
    if (need_roll) {
        // 关闭当前文件
        log_file_stream_.close();

        // 生成新分片文件并打开
        std::string new_file_name = generateRollFileName();
        log_file_stream_.open(new_file_name, std::ios::app | std::ios::out);
        if (!log_file_stream_.is_open()) {
            std::cerr << "[Logger Error] Failed to open rolled log file: " << new_file_name << std::endl;
            return false;
        }

        // 更新当前文件信息
        current_file_create_time_ = now;
        current_file_size_ = 0;

        std::cout << "[Logger Info] Log file rolled successfully to: " << new_file_name << std::endl;
    }

    return true;
}
// ----------------------------------------------------------------------

// 后台工作线程：无锁出队+日志输出（修改文件写入部分，增加分片检查）
void Logger::workerThread() {
    while (running_) {
        // 1. 无锁出队（非阻塞，空则返回nullptr）
        LogMessage* msg = log_queue_.pop();

        if (msg != nullptr) {
            // 2. 格式化最终日志内容
            std::stringstream ss;
            ss << "[" << msg->time << "] "
               << "[" << msg->thread_id << "] "
               << "[" << AsyncLogLevelToString(msg->level) << "] " << msg->content << std::endl;
            std::string log_str = ss.str();

            // 3. 输出到控制台
            std::cout << log_str;

            // 4. 输出到文件（线程安全，加锁保护）
            std::lock_guard<std::mutex> file_lock(file_mutex_);
            if (log_file_stream_.is_open()) {
                // 先检查是否需要分片（核心修改）
                checkAndRollFile();

                // 写入文件并更新当前文件大小
                log_file_stream_ << log_str;
                current_file_size_ += log_str.size();  // 累加写入字节数
                log_file_stream_.flush();              // 强制刷盘，避免日志丢失
            }

            // 5. 释放日志消息内存
            delete msg;
        } else {
            // 队列为空，等待新日志或退出信号（100ms超时避免永久阻塞）
            std::unique_lock<std::mutex> lock(mutex_);
            queue_cv_.wait_for(lock, std::chrono::milliseconds(100),
                               [this]() { return !running_ || !log_queue_.unsafeEmpty(); });
        }
    }

    // 退出前处理队列中剩余的日志（确保不丢失）
    LogMessage* remaining_msg = nullptr;
    while ((remaining_msg = log_queue_.pop()) != nullptr) {
        std::stringstream ss;
        ss << "[" << remaining_msg->time << "] "
           << "[" << remaining_msg->thread_id << "] "
           << "[" << AsyncLogLevelToString(remaining_msg->level) << "] " << remaining_msg->content << std::endl;
        std::string log_str = ss.str();

        std::cout << log_str;
        std::lock_guard<std::mutex> file_lock(file_mutex_);
        if (log_file_stream_.is_open()) {
            checkAndRollFile();  // 剩余日志写入前也检查分片
            log_file_stream_ << log_str;
            current_file_size_ += log_str.size();
            log_file_stream_.flush();
        }

        delete remaining_msg;
    }
}

}  // namespace VolcengineTos