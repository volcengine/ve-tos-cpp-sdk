#pragma once
#include "../../../../include/common/Common.h"
#include "../../../../include/transport/TransportConfig.h"
#include "../../../../include/transport/http/HttpResponse.h"
#include "../../../../include/utils/CachedQueue.h"
#include "../../../../include/utils/ConcurrentCachedBlockQueue.h"
#include "utils/CurlCache.h"
#include "../../../../include/utils/LinkedBufferQueue.h"
#include "../../../../include/utils/LockFreeQueue.h"

#include <curl/curl.h>
#include <atomic>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <unistd.h>

namespace VolcengineTos {

// 修改成其他上下文
// 不要前向声明
extern std::atomic<uint64_t> g_req_ctx_create_cnt;
extern std::atomic<uint64_t> g_req_ctx_destroy_cnt;

class AsyncHttpClient {
 public:
    // 构造函数，默认使用CPU核心数的线程
    explicit AsyncHttpClient(TransportConfig config, size_t thread_count = 0, bool crc_enable_ = false);

    AsyncHttpClient(const AsyncHttpClient&) = delete;
    AsyncHttpClient& operator=(const AsyncHttpClient&) = delete;

    AsyncHttpClient(AsyncHttpClient&&) = delete;
    AsyncHttpClient& operator=(AsyncHttpClient&&) = delete;

    static void initCurl();
    static void cleanupCurl();
    void closeClient();
    bool isClosed() const { return closed_.load(std::memory_order_acquire); }

    ~AsyncHttpClient();

    void enable_crc() { enable_crc_ = true; }

    // 携带 AsyncEvent 的回调版本
    std::future<std::shared_ptr<HttpResponse>> send(
        const std::shared_ptr<HttpRequest>& request, const OnDataReceiveWithEvent& on_data_receive,
        const OnDataSendWithEvent& on_data_send,
        std::function<void(std::shared_ptr<HttpResponse>)> on_request_finished,
        OnRequestStart on_request_start, OnHttpStatusSet on_http_status_set,
        OnContentLengthSet on_content_length_set) const;

 private:
    struct CurlMultiHandle;

    // 内部结构体定义
    struct RequestContext {
        CurlMultiHandle* parent = nullptr;

        std::string url;
        std::shared_ptr<HttpRequest> request;
        std::promise<std::shared_ptr<HttpResponse>> promise;

        CURLM* multi_handle = nullptr;
        CURL* curl_handle = nullptr;

        curl_slist* headers = nullptr;
        bool is_read = false;
        size_t needSkip = 0;
        size_t receive = 0;
        size_t send = 0;
        std::atomic<bool> in_paused_queue{false};

        // 传输模式
        bool is_chunked = false;

        // crc64
        bool enable_crc_ = false;
        uint64_t send_crc64_value = 0;
        uint64_t recv_crc64_value = 0;

        // 限流
        std::shared_ptr<RateLimiter> rate_limiter;

        // 下游更强情况下：靠用户自行暂停，然后通知恢复。看是否以后能将客户的事件注册进来，实现事件共池
        std::shared_ptr<AsyncEvent> event_;

        // 上游更强情况下缓存：链表缓冲区
        LinkedBufferQueue cached_buffer_queue_;

        // 响应相关
        std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>();
        bool completed = false;
        bool headers_completed_ = false;  // 新增：标记响应头是否接收完成

        // 重试相关
        int64_t not_send_util_ = 0;

        // 外部出入的钩子
        OnContentLengthSet on_content_length_set_;
        OnHttpStatusSet on_http_status_set_;
        OnRequestStart on_request_start_;
        OnDataReceiveWithEvent on_data_receive_with_event_;
        OnDataSendWithEvent on_data_send_with_event_;
        std::function<void(std::shared_ptr<HttpResponse> response)> on_request_finished_;

        explicit RequestContext(std::shared_ptr<HttpRequest> req) : request(std::move(req)) {
            ++g_req_ctx_create_cnt;
        }

        ~RequestContext() {
            if (headers) {
                curl_slist_free_all(headers);
            }
            if (curl_handle) {
                parent->curl_cache.Release(curl_handle);
                curl_handle = nullptr;
            }
            request = nullptr;
            ++g_req_ctx_destroy_cnt;
        }

        bool operator==(const RequestContext& other) const {
            return (this->request == other.request) && (this->curl_handle == other.curl_handle);
        }
    };

    struct CurlMultiHandle {
        CURLM* multi_handle = curl_multi_init();  // 析构清理
        std::thread thread;
        int max_connection_num = 0;

        // async-only cutover 后，worker 的活跃态等待从“sleep/condvar 驱动”改成了
        // `curl_multi_wait + eventfd` 模式；这里的 fd 就是给新请求入队、AsyncEvent 恢复、
        // 以及 shutdown 这些跨线程事件准备的唤醒通道。
        // 保留这段说明是为了后续 review 时区分：它解决的是“活跃态如何尽快打断 wait”，
        // 不是“空闲态如何取到新请求”——后者还受 queue_cv 路径影响。
        int wake_fd = -1;

        // 向 wake_fd 写入事件，唤醒 workerThread
        void Notify();

        // 清空 wake_fd 上的事件计数
        void DrainWakeFd();

        std::atomic<bool> running{false};
        std::atomic<bool> running_finish{false};
        // idle / paused-only 路径上的 queue_cv_ 只是“边沿触发”的唤醒原语，本身不会记住
        // 一次 notify_one()。因此这里额外维护一个 generation counter：
        //   1) waitForWorkOrResume() 在准备睡眠前先拍下当前 wake_seq；
        //   2) 任意需要 worker 重新检查状态的事件（send/resume/shutdown）都会先 ++wake_seq；
        //   3) queue_cv_.wait(..., pred) 通过比较 wake_seq 是否变化来判断“自从我决定要睡之后，
        //      是否已经发生过新的唤醒事件”。
        // 这解决的是经典 lost wakeup 竞态：
        //   waiter 判断自己可以睡 -> producer 入队新工作并 notify -> waiter 尚未真正阻塞，
        //   于是这次 notify 边沿被错过 -> waiter 随后睡下去，明明队列里已经有工作，却要等到
        //   下一次无关事件/超时才会醒。
        // 对比之下，活跃态的 curl_multi_wait 走的是 wake_fd(eventfd) 路径，eventfd 自带计数语义，
        // 内核会“记住”之前写入过的唤醒，所以不需要额外的软件计数器；wake_seq 是专门补给
        // queue_cv_ 这条路径的“状态记忆”。
        std::atomic<uint64_t> wake_seq{0};

        // todo：大中小IO拆分
        LockFreeQueue<RequestContext> request_queue_;
        CachedQueue<RequestContext> paused_queue_;
        std::set<RequestContext*> active_set_;
        std::atomic<uint64_t> active_count{0};
        std::atomic<int64_t> next_request_not_before_ms{-1};
        int curl_multi_wait_timeout_ms = 50;

        CurlCache curl_cache;
        std::vector<curl_waitfd> extra_fds;

        // 线程本地时间缓存（毫秒级），在curl_multi_wait后更新
        std::atomic<time_t> current_ms{0};
        std::atomic<time_t> last_print_ms{0};

        std::mutex queue_mutex_;
        // 空闲态 / paused-only 态的等待原语。当前设计将活跃 I/O 的阻塞固定收敛到
        // curl_multi_wait，而把无 runnable active 请求时的阻塞统一放在这里。
        // 注意：queue_cv_ 必须与 wake_seq 配合使用；单独的 notify_one() 只有边沿，没有记忆。
        std::condition_variable queue_cv_;

        CurlMultiHandle(const TransportConfig& transport_config, const int max_connection_num)
            : max_connection_num(max_connection_num),
              curl_multi_wait_timeout_ms(transport_config.getCurlMultiWaitTimeoutMs()),
              curl_cache(transport_config, max_connection_num),
              current_ms(0),
              last_print_ms(0) {}

        ~CurlMultiHandle() {
            int wait_count = 50;
            while (!running_finish && wait_count > 0) {
                wait_count--;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // 清理队列中未处理的请求（避免内存泄漏 + 通知调用者）
            while (!request_queue_.unsafeEmpty()) {
                RequestContext* ctx = request_queue_.pop();

                // Notify pipeline/caller before destroying context so that higher-level
                // resources (e.g. pooled pipelines / request bodies) can be released.
                if (ctx && ctx->response) {
                    ctx->response->setStatus(http::otherErr);
                    ctx->response->setStatusMsg("AsyncHttpClient destroyed before request processing");
                }
                if (ctx && ctx->on_request_finished_) {
                    try {
                        ctx->on_request_finished_(ctx->response);
                    } catch (...) {
                    }
                }

                try {
                    ctx->promise.set_exception(std::make_exception_ptr(
                        std::runtime_error("AsyncHttpClient destroyed before request processing")));
                } catch (...) { /* 防止set_exception重复调用崩溃 */
                }

                delete ctx;  // 释放请求上下文内存
            }

            while (!paused_queue_.empty()) {
                RequestContext* ctx = paused_queue_.pop();

                if (ctx && ctx->response) {
                    ctx->response->setStatus(http::otherErr);
                    ctx->response->setStatusMsg("AsyncHttpClient destroyed before request processing");
                }
                if (ctx && ctx->on_request_finished_) {
                    try {
                        ctx->on_request_finished_(ctx->response);
                    } catch (...) {
                    }
                }

                try {
                    ctx->promise.set_exception(std::make_exception_ptr(
                        std::runtime_error("AsyncHttpClient destroyed before request processing")));
                } catch (...) { /* 防止set_exception重复调用崩溃 */
                }

                delete ctx;  // 释放请求上下文内存
            }

            auto iter = active_set_.begin();
            while (iter != active_set_.end()) {
                RequestContext* ctx = *iter;  // 获取当前指针
                if (ctx != nullptr) {
                    try {
                        ctx->promise.set_exception(std::make_exception_ptr(
                            std::runtime_error("AsyncHttpClient destroyed at request processing")));
                    } catch (...) { /* 防止set_exception重复调用崩溃 */
                    }

                    if (ctx->response) {
                        ctx->response->setStatus(http::otherErr);
                        ctx->response->setStatusMsg("AsyncHttpClient destroyed at request processing");
                    }
                    if (ctx->on_request_finished_) {
                        try {
                            ctx->on_request_finished_(ctx->response);
                        } catch (...) {
                        }
                    }

                    if (multi_handle != nullptr && ctx->curl_handle != nullptr) {
                        curl_multi_remove_handle(multi_handle, ctx->curl_handle);
                    }
                    delete ctx;
                }
                // 3. erase 当前元素，返回下一个有效迭代器（直接赋值给 iter，无需 iter++）
                iter = active_set_.erase(iter);
            }

            if (multi_handle) {
                curl_multi_cleanup(multi_handle);
                multi_handle = nullptr;
            }

            if (wake_fd >= 0) {
                ::close(wake_fd);
                wake_fd = -1;
            }
        }
    };

    // 内部方法

    static RequestContext* getRequest(CurlMultiHandle& multi_handle, int64_t* next_request_not_before_ms);
    CURLcode addRequestToEventor(CURLM* multi_handle, RequestContext* ctx) const;
    static void processCompleteActiveFds(CURLM* multi_handle, std::thread::id& thread_id,
                                         std::vector<curl_waitfd>& extra_fds,
                                         const std::set<RequestContext*>& active_ctxs);
    void processRequestDone(CurlMultiHandle& multi_handle, CurlCache& curl_cache, const CURLMsg* msg,
                            int& messages_left) const;
    void processCompletedRequests(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                  CurlCache& curl_cache, std::vector<curl_waitfd>& extra_fds) const;
    static void updateCurrentTime(CurlMultiHandle& multi_handle);
    void tryPrintLog(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                     const CurlCache& curl_cache) const;
    void processNewRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                           CurlCache& curl_cache) const;
    static void processActiveFds(const CurlMultiHandle& multi_handle, std::vector<curl_waitfd>& extra_fds,
                                 const std::set<RequestContext*>& active_ctxs);
    static bool hasRunnableActive(const CurlMultiHandle& multi_handle);
    static int64_t getNextIdleWakeupMs(const CurlMultiHandle& multi_handle);
    static void processActiveRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                     std::vector<curl_waitfd>& extra_fds);
    static void waitForWorkOrResume(CurlMultiHandle& multi_handle);
    static void processPausedRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id);
    static void enqueuePausedRequest(CurlMultiHandle& multi_handle, RequestContext* ctx);
    static void removeFromPausedQueue(CurlMultiHandle& multi_handle, RequestContext* target_ctx);
    void workerThread(CurlMultiHandle& multi_handle) const;

    // AsyncEvent notifier: wakeup curl_multi_wait in target CurlMultiHandle
    static void WakeupMultiHandleFromEvent(void* user_data);

    struct PauseDecision {
        bool should_pause = false;
        int64_t resume_at_ms = 0;
    };

    struct DownloadConsumeResult {
        size_t consumed = 0;
        bool invalid_result = false;
        bool terminal_failure = false;
    };

    // 回调函数
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static PauseDecision buildPauseDecision(RequestContext* ctx, size_t bytes_used, bool force_pause);
    static void applyPauseState(RequestContext* ctx, int64_t resume_at_ms);
    static size_t finalizeUploadCallback(RequestContext* ctx, size_t bytes_read);
    static size_t finalizeDownloadCallback(RequestContext* ctx, char* ptr, size_t data_len, size_t written,
                                           size_t skipped);
    static DownloadConsumeResult consumeDownloadData(RequestContext* ctx, char* ptr, size_t data_len,
                                                    AsyncEvent* event);
    static size_t readCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata);
    static int debugCallback(CURL* curl, curl_infotype type, char* data, size_t size, void* userdata);

    // 成员变量
    // 全局引用计数：跟踪活跃的 AsyncHttpClient 实例数
    static std::atomic_uint curl_global_ref_count;
    std::vector<std::shared_ptr<CurlMultiHandle>> multi_handles_;
    size_t thread_count_;
    std::atomic<bool> running_{false};
    std::atomic<bool> closed_{false};

    TransportConfig transport_config_;
    bool enable_crc_ = false;

    // todo：DNS支持手动淘汰
    static size_t writeCallbackImpl(RequestContext* ctx, char* ptr, size_t data_len);
    static size_t readCallbackImpl(RequestContext* ctx, char* ptr, size_t data_len);
};

}  // namespace VolcengineTos
