#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace VolcengineTos {

class AsyncHttpClient;

// 事件模型：用于在异步 HTTP 读/写回调中传递暂停/恢复信号
// 线程安全说明：
//  - state_ 与 resume_at_ms_ 使用 std::atomic 保证跨线程可见性；
//  - reason_ 使用互斥锁保护，避免数据竞争；
class AsyncEvent {
public:
    friend class AsyncHttpClient;

    // 定义最大保留的原因数量（可按需调整）
    static constexpr size_t MAX_REASON_COUNT = 10;

    // Triggered -> Stopping -> Waiting -> Resumed -> Triggered
    // Triggered -> Waiting -> Resumed -> Triggered
    enum class State {
        Triggered = 0,  // 当前回调被触发中（默认状态）
        Waiting,        // 需要等待到某时刻，用户设置
        Stopping,       // 永久等待，用户设置
        Resumed,        // 从恢复队列取出
        Cancelled       // 取消 todo：实现为客户端可以主动优雅断链
    };

    AsyncEvent() : state_(State::Triggered), resume_at_ms_(0) {
    }

    using NotifierFn = void (*)(void* user_data);

    // stopping在暂停队列中被发现，有数据则触发callback，消费完毕后还会持续等待信号
    void markStoping() {
        state_.store(State::Stopping, std::memory_order_release);
        notify();
    }

    void Pause() {
        markStoping();
    }

    // 将事件标记为等待状态
    // waiting在暂停队列中被发现，有数据则触发callback，一旦消费完缓存的数据则恢复
    void markWaiting() {
        state_.store(State::Waiting, std::memory_order_release);
        notify();
    }

    void PauseFor(std::int64_t delay_ms) {
        markWaiting();
        if (delay_ms > 0) {
            setResumeAt(NowMs() + delay_ms);
        } else {
            setResumeAt(0);
        }
    }

    void markResumed() {
        state_.store(State::Resumed, std::memory_order_release);
        notify();
    }

    void Resume() {
        markResumed();
    }

    // TODO(review): 事件驱动链路里需要区分“暂时没数据/写不下”和“已经出现终态错误”。
    // 这里单独保留 failed 标记，并把状态切到 `Resumed` 主动唤醒 curl worker，
    // 避免请求永远卡在 paused queue 里等待一个永远不会满足的恢复条件。
    void markFailed(const std::string& reason = std::string()) {
        if (!reason.empty()) {
            setReason(reason);
        }
        failed_.store(true, std::memory_order_release);
        resume_at_ms_.store(0, std::memory_order_release);
        state_.store(State::Resumed, std::memory_order_release);
        notify();
    }

    void Fail(const std::string& reason = std::string()) {
        markFailed(reason);
    }

    // 声明该事件已准备就绪，可以恢复
    void notifyReady() {
        state_.store(State::Waiting, std::memory_order_release);
        notify();
    }

    // 是否处于等待状态
    bool isWaiting() const {
        return state_.load(std::memory_order_acquire) == State::Waiting;
    }

    bool isStopping() const {
        return state_.load(std::memory_order_acquire) == State::Stopping;
    }

    bool isResumed() const {
        return state_.load(std::memory_order_acquire) == State::Resumed;
    }

    bool isPaused() const {
        const State state = state_.load(std::memory_order_acquire);
        return state == State::Waiting || state == State::Stopping;
    }

    bool isFailed() const {
        return failed_.load(std::memory_order_acquire);
    }

    // 取消语义与失败语义统一走同一条 terminal fast-fail 路径，确保 AsyncHttpClient 能及时退出。
    void cancel() {
        markFailed("cancelled");
    }

    State getState() const {
        return state_.load(std::memory_order_acquire);
    }

    // 设置/获取推荐恢复时间（毫秒级时间戳，统一使用 system_clock epoch 毫秒）
    void setResumeAt(std::int64_t resume_at_ms) {
        resume_at_ms_.store(resume_at_ms, std::memory_order_release);
        // resumeAt 变化可能影响 event loop 的等待时长
        notify();
    }

    std::int64_t getResumeAt() const {
        return resume_at_ms_.load(std::memory_order_acquire);
    }

    // 追加原因到vector末尾，超过10个时删除最旧的
    void setReason(const std::string& reason) {
        // TODO(review): reason 仅服务调试信息，不能阻塞真正的数据发送/接收回调；
        // 这里用 try-lock，避免日志侧长时间占锁时把 async curl worker 卡住。
        std::unique_lock<std::mutex> lock(reason_mutex_, std::try_to_lock);
        if (!lock.owns_lock()) {
            return;
        }
        reasons_.push_back(reason);
        if (reasons_.size() > MAX_REASON_COUNT) {
            reasons_.erase(reasons_.begin());
        }
    }

    // 获取所有原因的vector（最多10个，按新增顺序：旧→新）
    std::vector<std::string> getReasons() const {
        return CopyReasons();
    }

    // 兼容原有接口：获取拼接后的所有原因（用分号分隔）
    std::string getReason() const {
        const auto reasons = CopyReasons();
        std::ostringstream oss;
        for (size_t i = 0; i < reasons.size(); ++i) {
            if (i > 0)
                oss << "; ";
            oss << reasons[i];
        }
        return oss.str();
    }

    // 格式化输出AsyncEvent的所有状态信息（修复time_point构造错误）
    std::string toString() const {
        std::ostringstream oss;

        // 1. 状态转字符串
        std::string stateStr;
        switch (getState()) {
            case State::Triggered:
                stateStr = "Triggered";
                break;
            case State::Waiting:
                stateStr = "Waiting";
                break;
            case State::Stopping:
                stateStr = "Stopping";
                break;
            case State::Resumed:
                stateStr = "Resumed";
                break;
            case State::Cancelled:
                stateStr = "Cancelled";
                break;
            default:
                stateStr = "Unknown";
                break;
        }

        // 2. 恢复时间（转成可读格式，同时保留原始毫秒数）
        auto resumeAtMs = getResumeAt();
        std::string resumeAtStr = "0 (not set)";
        if (resumeAtMs > 0) {
            // 【核心修复】正确构造time_point：基准时间点 + 毫秒时长
            std::chrono::system_clock::time_point resumeTime =
                    std::chrono::system_clock::time_point() + std::chrono::milliseconds(resumeAtMs);

            // 兼容不同平台的localtime线程安全问题（可选优化）
            std::time_t resumeTt = std::chrono::system_clock::to_time_t(resumeTime);
            // 临时缓冲区避免localtime返回的指针被覆盖
            char timeBuf[64] = {0};
            std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&resumeTt));

            resumeAtStr = std::to_string(resumeAtMs) + " (" + timeBuf + ")";
        }

        // 3. 拼接所有信息
        oss << "AsyncEvent {" << std::endl
            << "  State: " << stateStr << std::endl
            << "  Failed: " << (isFailed() ? "true" : "false") << std::endl
            << "  ResumeAtMs: " << resumeAtStr << std::endl
            << "  Reasons (max " << MAX_REASON_COUNT << "): [";

        // 4. 原因列表
        auto reasons = CopyReasons();
        for (size_t i = 0; i < reasons.size(); ++i) {
            if (i > 0)
                oss << ", ";
            oss << "\"" << reasons[i] << "\"";
        }
        oss << "]" << std::endl << "}";

        return oss.str();
    }

    void reset() {
        state_.store(State::Triggered, std::memory_order_release);
        failed_.store(false, std::memory_order_release);
        resume_at_ms_.store(0, std::memory_order_release);
        // 清空原因列表
        //        std::lock_guard<std::mutex> lock(reason_mutex_);
        //        reasons_.clear();
    }

private:
    // 设置跨线程唤醒回调（仅供 AsyncHttpClient 绑定 worker 唤醒链路，不能暴露给业务层覆盖）
    void setNotifier(NotifierFn fn, void* user_data) {
        notifier_fn_.store(fn, std::memory_order_release);
        notifier_user_data_.store(user_data, std::memory_order_release);
    }

    static std::int64_t NowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    std::vector<std::string> CopyReasons() const {
        std::unique_lock<std::mutex> lock(reason_mutex_, std::try_to_lock);
        if (!lock.owns_lock()) {
            return {};
        }
        return reasons_;
    }

    void notify() const {
        auto fn = notifier_fn_.load(std::memory_order_acquire);
        if (fn == nullptr) {
            return;
        }
        fn(notifier_user_data_.load(std::memory_order_acquire));
    }

    std::atomic<State> state_;
    std::atomic<bool> failed_{false};
    std::atomic<std::int64_t> resume_at_ms_;

    std::atomic<NotifierFn> notifier_fn_{nullptr};
    std::atomic<void*> notifier_user_data_{nullptr};

    mutable std::mutex reason_mutex_;
    // 存储最多10个最新的原因（顺序：旧→新）
    std::vector<std::string> reasons_;
};

}  // namespace VolcengineTos
