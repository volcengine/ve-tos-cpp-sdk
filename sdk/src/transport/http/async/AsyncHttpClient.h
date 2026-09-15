#pragma once
#include "../../../../include/common/Common.h"
#include "../../../../include/transport/TransportConfig.h"
#include "../../../../include/transport/http/HttpResponse.h"
#include "../../../../include/utils/CachedQueue.h"
#include "../../../../include/utils/ConcurrentCachedBlockQueue.h"
#include "utils/CurlCache.h"
#include "../../../../include/utils/LinkedBufferQueue.h"
#include "../../../../include/utils/ConcurrentQueue.h"

#include <curl/curl.h>
#include <atomic>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <unistd.h>
#include "AsyncEngine.h"

namespace VolcengineTos {

// 修改成其他上下文
// 不要前向声明
extern std::atomic<uint64_t> g_req_ctx_create_cnt;
extern std::atomic<uint64_t> g_req_ctx_destroy_cnt;

class AsyncHttpClient {
 public:
    // 构造函数，默认使用CPU核心数的线程
    explicit AsyncHttpClient(TransportConfig config, size_t thread_count = 0, bool crc_enable_ = false);
    AsyncHttpClient(TransportConfig config, std::shared_ptr<AsyncEngine> engine,
                    const AsyncClientSharingOptions& sharing = {});
    void beginClose();

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

    // Callback-only submission allocates no promise/shared future state.
    void sendCallback(const std::shared_ptr<HttpRequest>& request,
        const OnDataReceiveWithEvent& on_data_receive, const OnDataSendWithEvent& on_data_send,
        std::function<void(std::shared_ptr<HttpResponse>)> on_request_finished,
        OnRequestStart on_request_start = {}, OnHttpStatusSet on_http_status_set = {},
        OnContentLengthSet on_content_length_set = {}) const;

 private:
    friend class AsyncEngineCore;
    friend struct AsyncTransportTestAccess;
    std::future<std::shared_ptr<HttpResponse>> submit(
        const std::shared_ptr<HttpRequest>& request, const OnDataReceiveWithEvent& on_data_receive,
        const OnDataSendWithEvent& on_data_send,
        std::function<void(std::shared_ptr<HttpResponse>)> on_request_finished,
        OnRequestStart on_request_start, OnHttpStatusSet on_http_status_set,
        OnContentLengthSet on_content_length_set, bool need_future) const;
    struct CurlMultiHandle;

    // 内部结构体定义
    struct RequestContext {
        CurlMultiHandle* parent = nullptr;

        std::string url;
        std::shared_ptr<HttpRequest> request;
        std::optional<std::promise<std::shared_ptr<HttpResponse>>> promise;
        bool pending_slot = false;
        bool detail_log = false;
        size_t response_header_limit = 0;
        size_t response_header_bytes = 0;
        std::function<void(RequestContext&)> on_result_;
        std::function<void()> on_retired_;
        std::shared_ptr<void> engine_request;

        CURLM* multi_handle = nullptr;
        CURL* curl_handle = nullptr;
        bool registered_with_multi = false;

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
        // Worker-owned QoS timer, separate from the user's PauseFor/Resume.
        int64_t qos_resume_at_ms = 0;

        // 下游更强情况下：靠用户自行暂停，然后通知恢复。看是否以后能将客户的事件注册进来，实现事件共池
        std::shared_ptr<AsyncEvent> event_;

        // 上游更强情况下缓存：链表缓冲区
        LinkedBufferQueue cached_buffer_queue_;

        // 响应相关
        std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>();
        bool completed = false;
        // Worker-owned, sticky: a later CURLMSG_DONE/OK cannot undo a sink/source error.
        CURLcode callback_error = CURLE_OK;
        void recordCallbackError(CURLcode code) noexcept {
            if (callback_error == CURLE_OK) callback_error = code;
        }
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

        void releasePendingSlot() noexcept {
            if (pending_slot) {
                pending_slot = false;
                parent->pending_count.fetch_sub(1, std::memory_order_relaxed);
            }
        }
        void notifyFinished() noexcept {
            // Claim before external code, including reentry. Not a drain signal.
            std::function<void(std::shared_ptr<HttpResponse>)> callback;
            callback.swap(on_request_finished_);
            auto observer = std::move(on_result_);
            if (observer) { try { observer(*this); } catch (...) {} }
            if (callback) { try { callback(response); } catch (...) {} }
        }
        void failPromise(const char* message) noexcept {
            if (!promise) return;
            try { promise->set_exception(std::make_exception_ptr(std::runtime_error(message))); }
            catch (...) {}
        }
        void fulfillPromise() noexcept {
            if (!promise) return;
            try { promise->set_value(response); } catch (...) {}
        }
        void detachCurl() noexcept {
            if (registered_with_multi) {
                curl_multi_remove_handle(multi_handle, curl_handle);
                registered_with_multi = false;
            }
        }
        void failSetup(CURLcode code, const char* message) noexcept {
            // No transfer has run. Detach before external completion/reentry.
            detachCurl();
            completed = true;
            response->setStatus(http::otherErr);
            response->setCurlErrCode(code);
            try { response->setStatusMsg(message); } catch (...) {}
            failPromise(message);
            notifyFinished();
        }
        ~RequestContext() {
            releasePendingSlot();
            detachCurl();
            if (headers) {
                curl_slist_free_all(headers);
            }
            if (curl_handle) {
                parent->curl_cache.Release(curl_handle);
                curl_handle = nullptr;
            }
            request = nullptr;
            ++g_req_ctx_destroy_cnt;
            // Retiring means all user captures and transport buffers are gone,
            // not merely that the result callback has started.
            on_request_finished_ = {};
            on_result_ = {};
            on_request_start_ = {};
            on_http_status_set_ = {};
            on_content_length_set_ = {};
            on_data_receive_with_event_ = {};
            on_data_send_with_event_ = {};
            cached_buffer_queue_.clear();
            response.reset();
            event_.reset();
            if (on_retired_) { try { on_retired_(); } catch (...) {} }
        }

        bool operator==(const RequestContext& other) const {
            return (this->request == other.request) && (this->curl_handle == other.curl_handle);
        }
    };

    struct CurlMultiHandle {
        CURLM* multi_handle = nullptr;  // allocated after all throwing members
        std::thread thread;
        int max_connection_num = 0;

        // Socket/timer wait, including paused transfers. Submission, resume
        // and shutdown wake exactly the owning worker via this counted fd.
        int wake_fd = -1;
        // Shared engine: redirect wakeups to its worker, not a per-multi thread.
        void (*engine_notify)(void*) = nullptr;
        void* engine_notify_data = nullptr;

        // 向 wake_fd 写入事件，唤醒 workerThread
        void Notify();

        // 清空 wake_fd 上的事件计数
        void DrainWakeFd();

        std::atomic<bool> running{false};
        std::atomic<bool> running_finish{false};
        // Generation and queue_mutex_ close the predicate-check/park race for
        // the no-active-transfer CV wait (also the eventfd-failure fallback).
        std::atomic<uint64_t> wake_seq{0};

        // todo：大中小IO拆分
        ConcurrentQueue<RequestContext> request_queue_;
        CachedQueue<RequestContext> paused_queue_;
        std::set<RequestContext*> active_set_;
        std::atomic<uint64_t> active_count{0};
        std::atomic<size_t> pending_count{0};
        int curl_multi_wait_timeout_ms;

        CurlCache curl_cache;
        std::vector<curl_waitfd> extra_fds;

        // 线程本地时间缓存（毫秒级），在curl_multi_wait后更新
        std::atomic<time_t> current_ms{0};
        std::atomic<time_t> last_print_ms{0};

        std::mutex queue_mutex_;
        // No active transfers: park until a queued deadline or notification.
        // Paused transfers stay in curl so its own deadlines keep advancing.
        std::condition_variable queue_cv_;

        CurlMultiHandle(const TransportConfig& transport_config, const int max_connection_num)
            : max_connection_num(max_connection_num),
              curl_multi_wait_timeout_ms(transport_config.getCurlMultiWaitTimeoutMs()),
              curl_cache(transport_config, max_connection_num),
              current_ms(0),
              last_print_ms(0) {
            // A queue/cache/config constructor can throw. Allocate the raw
            // curl resource only after those members have finished.
            multi_handle = curl_multi_init();
        }

        ~CurlMultiHandle() {
            int wait_count = 50;
            while (!running_finish && wait_count > 0) {
                wait_count--;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Paused entries alias active_set_: they do not own contexts.
            while (paused_queue_.pop() != nullptr) {}
            while (RequestContext* ctx = request_queue_.pop()) {
                ctx->response->setStatus(http::otherErr);
                ctx->response->setStatusMsg("AsyncHttpClient destroyed before request processing");
                ctx->notifyFinished();
                ctx->failPromise("AsyncHttpClient destroyed before request processing");
                delete ctx;
            }
            for (RequestContext* ctx : active_set_) {
                ctx->response->setStatus(http::otherErr);
                ctx->response->setStatusMsg("AsyncHttpClient destroyed at request processing");
                ctx->notifyFinished();
                ctx->failPromise("AsyncHttpClient destroyed at request processing");
                ctx->detachCurl();
                delete ctx;
            }
            active_set_.clear();

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

    static RequestContext* getRequest(CurlMultiHandle& multi_handle);
    static CURLcode addRequestToEventor(CURLM* multi_handle, RequestContext* ctx);
    static void processCompleteActiveFds(CURLM* multi_handle, std::thread::id& thread_id,
                                         std::vector<curl_waitfd>& extra_fds,
                                         const std::set<RequestContext*>& active_ctxs);
    static void finishRequest(CurlMultiHandle& multi_handle, RequestContext* ctx, CURLcode result);
    static void processCompletedRequests(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                  CurlCache& curl_cache, std::vector<curl_waitfd>& extra_fds);
    static void updateCurrentTime(CurlMultiHandle& multi_handle);
    void tryPrintLog(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                     const CurlCache& curl_cache) const;
    void processNewRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                           CurlCache& curl_cache) const;
    static void processActiveFds(const CurlMultiHandle& multi_handle, std::vector<curl_waitfd>& extra_fds,
                                 const std::set<RequestContext*>& active_ctxs);
    static int64_t getNextIdleWakeupMs(const CurlMultiHandle& multi_handle);
    static int64_t pausedDeadlineMs(const RequestContext& ctx);
    void processActiveRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                              std::vector<curl_waitfd>& extra_fds) const;
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
    static size_t headerCallbackImpl(char* buffer, size_t total_size, RequestContext* ctx);
    static int debugCallback(CURL* curl, curl_infotype type, char* data, size_t size, void* userdata);

    // 成员变量
    // 全局引用计数：跟踪活跃的 AsyncHttpClient 实例数
    static std::atomic_uint curl_global_ref_count;
    // Serializes admission with close; never held across network IO or callbacks.
    mutable std::mutex submission_mutex_;
    std::vector<std::shared_ptr<CurlMultiHandle>> multi_handles_;
    size_t thread_count_;
    std::atomic<bool> running_{false};
    std::atomic<bool> closed_{false};

    TransportConfig transport_config_;
    bool enable_crc_ = false;
    std::shared_ptr<AsyncEngine> engine_;
    std::shared_ptr<struct AsyncClientSession> session_;

    // todo：DNS支持手动淘汰
    static size_t writeCallbackImpl(RequestContext* ctx, char* ptr, size_t data_len);
    static size_t readCallbackImpl(RequestContext* ctx, char* ptr, size_t data_len);
};

}  // namespace VolcengineTos
