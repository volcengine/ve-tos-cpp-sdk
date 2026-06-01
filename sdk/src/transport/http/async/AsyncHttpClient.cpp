#include "AsyncHttpClient.h"


#include "TosClient.h"
#include "logger/logger.h"
#include "transport/CurlStatsCollector.h"
#include "utils/Name2Log.h"
#include "utils/crc64.h"

#include <curl/curl.h>
#include <sys/mman.h>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <thread>
#include <utility>

#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace VolcengineTos;
namespace VolcengineTos {
std::atomic<uint64_t> g_req_ctx_create_cnt(0);
std::atomic<uint64_t> g_req_ctx_destroy_cnt(0);
std::atomic_uint AsyncHttpClient::curl_global_ref_count(0);

void AsyncHttpClient::WakeupMultiHandleFromEvent(void* user_data) {
    auto* handle = reinterpret_cast<CurlMultiHandle*>(user_data);
    if (handle != nullptr) {
        handle->Notify();
    }
}

void AsyncHttpClient::CurlMultiHandle::Notify() {
    // 对 queue_cv_ 路径来说，真正的唤醒条件不是“刚才有人 notify 过”，而是
    // “自从 waiter 决定睡眠以来，是否发生过新的事件”。因此先递增 wake_seq，
    // 再发出 notify_one()；这样即便 notify 恰好落在 waiter 进入 wait() 的边缘窗口，
    // waiter 也会因为看到 wake_seq 已变化而不再继续睡下去。
    wake_seq.fetch_add(1, std::memory_order_relaxed);
    if (wake_fd < 0) {
        // 这里不需要额外持有 queue_mutex_：wait 的 predicate 观察的是 running/wake_seq
        // 这两个原子状态，而不是某个必须在同一把 mutex 下发布的普通布尔值。
        queue_cv_.notify_one();
        return;
    }
    const uint64_t one = 1;
    const ssize_t n = ::write(wake_fd, &one, sizeof(one));
    (void)n;
    // wake_fd 唤醒的是 curl_multi_wait，queue_cv_ 唤醒的是 idle / paused-only wait；
    // 两条阻塞路径都要覆盖。
    queue_cv_.notify_one();
}

void AsyncHttpClient::CurlMultiHandle::DrainWakeFd() {
    if (wake_fd < 0) {
        return;
    }
    uint64_t value = 0;
    while (true) {
        const ssize_t n = ::read(wake_fd, &value, sizeof(value));
        if (n == static_cast<ssize_t>(sizeof(value))) {
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        }
        break;
    }
}

AsyncHttpClient::AsyncHttpClient(TransportConfig config, const size_t thread_count, const bool crc_enable_)
    : running_(true), transport_config_(std::move(config)), enable_crc_(crc_enable_) {
    Logger::getInstance().info("AsyncHttpClient init start, requested_thread_count=", thread_count,
                               ", config_event_thread_count=", config.getEventThreadCount(),
                               ", max_connections=", config.getMaxConnections(), ", enable_crc=", crc_enable_);
    initCurl();

    // 如果线程数为0，自动设置为CPU核心数/4，最少保留一个worker。
    if (thread_count == 0) {
        const size_t hardware_threads = std::max<size_t>(1, std::thread::hardware_concurrency());
        thread_count_ = std::max<size_t>(1, hardware_threads / 4);
    } else {
        thread_count_ = thread_count;
    }

    if (config.getEventThreadCount() > 0) {
        thread_count_ = static_cast<size_t>(config.getEventThreadCount());
    }
    thread_count_ = std::max<size_t>(1, thread_count_);
    const int configured_max_connections = config.getMaxConnections();
    const size_t max_connections =
        configured_max_connections > 0 ? static_cast<size_t>(configured_max_connections) : 1;
    const size_t connections_per_worker = std::max<size_t>(1, max_connections / thread_count_);

    Logger::getInstance().info("HttpClient will use ", thread_count_, " worker threads",
                               ", connections_per_worker=", connections_per_worker);

    // 创建多个curl_multi实例，每个实例一个线程
    for (size_t i = 0; i < thread_count_; ++i) {
        auto multi_handle = std::make_shared<CurlMultiHandle>(
            config, static_cast<int>(connections_per_worker));
        if (!multi_handle->multi_handle) {
            Logger::getInstance().error("Failed to create curl_multi handle for thread ", i);
            continue;
        }

        // 每个 worker 维护独立 eventfd，只定向唤醒对应 multi handle，避免像全局 condvar
        // 那样把所有线程都惊醒。这个设计初衷是降低 async cutover 后的无效唤醒成本。
        multi_handle->wake_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (multi_handle->wake_fd < 0) {
            Logger::getInstance().error("Failed to create eventfd for thread ", i, ", errno=", errno);
        }
        Logger::getInstance().debug("Created curl_multi handle ", ptrToString(multi_handle->multi_handle),
                                    " for thread ", i);

        multi_handle->running = true;
        multi_handle->thread = std::thread(&AsyncHttpClient::workerThread, this, std::ref(*multi_handle));
        Logger::getInstance().info("AsyncHttpClient worker create success, worker_index=", i,
                                   ", multi_handle=", ptrToString(multi_handle->multi_handle), ", wake_fd=",
                                   multi_handle->wake_fd);

        multi_handles_.push_back(multi_handle);
    }

    Logger::getInstance().info("AsyncHttpClient init done, configured_workers=", thread_count_,
                               ", created_workers=", multi_handles_.size());
}

void AsyncHttpClient::closeClient() {
    bool expected = false;
    if (!closed_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        Logger::getInstance().info("CloseClient already completed, skip");
        return;
    }

    running_ = false;
    Logger::getInstance().info("AsyncHttpClient shutdown start, worker_count=", multi_handles_.size());
    for (const auto& handle : multi_handles_) {
        handle->running = false;  // 标记当前线程需退出
        handle->Notify();
    }
    Logger::getInstance().info("AsyncHttpClient shutdown notified all workers");

    // 等待所有工作线程结束
    for (size_t i = 0; i < multi_handles_.size(); ++i) {
        const auto& multi_handle = multi_handles_[i];
        if (multi_handle->thread.joinable()) {
            Logger::getInstance().info("AsyncHttpClient waiting worker join, worker_index=", i,
                                       ", multi_handle=", ptrToString(multi_handle->multi_handle));
            multi_handle->thread.join();
            Logger::getInstance().info("AsyncHttpClient worker joined, worker_index=", i,
                                       ", multi_handle=", ptrToString(multi_handle->multi_handle));
        }
    }

    // Destroy curl_multi handles + drain queued/active requests BEFORE global curl cleanup.
    // Otherwise queued RequestContext/HttpRequest payloads (e.g. upload PartStream buffers) may leak.
    std::vector<std::shared_ptr<CurlMultiHandle>> handles;
    handles.swap(multi_handles_);
    handles.clear();

    cleanupCurl();
    Logger::getInstance().info("AsyncHttpClient shutdown done");
}

AsyncHttpClient::~AsyncHttpClient() {
    Logger::getInstance().info("AsyncHttpClient destructor start");
    closeClient();
}

void AsyncHttpClient::initCurl() {
    const uint32_t old_count = curl_global_ref_count.fetch_add(1, std::memory_order_acq_rel);
    if (old_count == 0) {
        // 第一个实例：初始化 libcurl 全局资源
        const CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
        if (res != CURLE_OK) {
            // 初始化失败：回减引用计数，避免泄露
            curl_global_ref_count.fetch_sub(1, std::memory_order_acq_rel);
            std::string err_msg = "curl_global_init failed: " + std::string(curl_easy_strerror(res));
            Logger::getInstance().error(err_msg);
            throw std::runtime_error(err_msg);
        }
        Logger::getInstance().info("libcurl initialized successfully (global ref count: 1)");
    } else {
        // 非第一个实例：复用已初始化的全局资源
        uint32_t current_count = curl_global_ref_count.load(std::memory_order_relaxed);
        Logger::getInstance().info("libcurl already initialized, global ref count: ", current_count);
    }
}

void AsyncHttpClient::cleanupCurl() {
    uint32_t old_count = curl_global_ref_count.load(std::memory_order_acquire);
    while (true) {
        if (old_count == 0) {
            Logger::getInstance().warn("cleanupCurl called when global ref count is already 0, skip");
            return;
        }
        if (curl_global_ref_count.compare_exchange_weak(old_count, old_count - 1, std::memory_order_acq_rel,
                                                        std::memory_order_acquire)) {
            break;
        }
    }

    if (old_count == 1) {
        // 最后一个实例：释放 libcurl 全局资源（需确保所有句柄已释放）
        curl_global_cleanup();
        Logger::getInstance().info("libcurl cleaned up (global ref count: 0)");
    } else {
        // 还有其他活跃实例：不释放全局资源
        Logger::getInstance().info("libcurl global ref count decreased to: ", old_count - 1);
    }
}

std::future<std::shared_ptr<HttpResponse>> AsyncHttpClient::send(
    const std::shared_ptr<HttpRequest>& request, const OnDataReceiveWithEvent& on_data_receive,
    const OnDataSendWithEvent& on_data_send,
    std::function<void(std::shared_ptr<HttpResponse>)> on_request_finished, OnRequestStart on_request_start,
    OnHttpStatusSet on_http_status_set, OnContentLengthSet on_content_length_set) const {
    // 1. 创建请求上下文
    auto* ctx = new RequestContext(request);
    ctx->url = request->url().toString();
    ctx->enable_crc_ = request->isCheckCrc64();
    ctx->on_data_receive_with_event_ = on_data_receive;
    ctx->on_data_send_with_event_ = on_data_send;
    ctx->on_request_finished_ = std::move(on_request_finished);
    if (on_request_start) {
        ctx->on_request_start_ = std::move(on_request_start);
    }
    if (on_content_length_set) {
        ctx->on_content_length_set_ = std::move(on_content_length_set);
    }
    if (on_http_status_set) {
        ctx->on_http_status_set_ = std::move(on_http_status_set);
    }
    ctx->not_send_util_ = request->notSendUtilMs();
    ctx->is_chunked = request->isChunked();
    ctx->rate_limiter = request->getRateLimiter();
    ctx->event_ = std::make_shared<AsyncEvent>();

    auto future = ctx->promise.get_future();

    // 2. 选择「请求数最少」的线程队列
    std::shared_ptr<CurlMultiHandle> target_handle = nullptr;
    size_t min_queue_size = std::numeric_limits<size_t>::max();

    // 遍历所有线程队列，无锁获取大小，可能不精确
    for (auto& handle : multi_handles_) {
        size_t current_size = handle->request_queue_.unsafeSize() + handle->active_count;

        // 更新最小队列（找到更小的则替换）
        if (current_size < min_queue_size) {
            min_queue_size = current_size;
            target_handle = handle;
        }

        // 优化：找到空队列直接退出遍历（减少后续锁开销）
        if (min_queue_size == 0) break;
    }

    // 异常处理
    if (!running_ || !target_handle) {
        Logger::getInstance().error("send() failed: no valid worker thread, worker number: ",
                                    thread_count_);
        ctx->response->setStatus(http::otherErr);
        ctx->response->setStatusMsg("failed: no valid worker thread.");
        if (ctx->on_request_finished_) {
            ctx->on_request_finished_(ctx->response);
        }
        try {
            ctx->promise.set_exception(
                std::make_exception_ptr(std::runtime_error("No worker thread available")));
        } catch (...) {
        }
        delete ctx;
        return future;
    }

    size_t max_queue_size;
    if (transport_config_.getMaxRequestQueue() > 0) {
        max_queue_size = static_cast<size_t>(transport_config_.getMaxRequestQueue());
    } else {
        max_queue_size = static_cast<size_t>(target_handle->curl_cache.getMaxPoolSize() * 2);
    }

    if (target_handle->request_queue_.unsafeSize() > max_queue_size) {
        // todo：可以考虑做个任务窃取，应对投递的任务长尾问题，就算请求均分了，某些请求在其他worker因为客户投递顺序问题更容易拿到某类任务，执行时间长，未充分发挥多核能力。
        //  如果要实现，在getRequest的时候不要一把拿了那么多请求，要做点反压，还需设计。
        Logger::getInstance().error(
            "send() failed: queue overflow, now request wait: ", target_handle->request_queue_.unsafeSize(),
            ", max: ", max_queue_size, ", worker number:", thread_count_);
        ctx->response->setStatus(http::otherErr);
        ctx->response->setStatusMsg("failed: request queue overflow.");
        if (ctx->on_request_finished_) {
            ctx->on_request_finished_(ctx->response);
        }
        try {
            ctx->promise.set_exception(
                std::make_exception_ptr(std::runtime_error("Request queue overflow")));
        } catch (...) {
        }
        delete ctx;
        return future;
    }
    ctx->parent = target_handle.get();
    ctx->event_->setNotifier(&AsyncHttpClient::WakeupMultiHandleFromEvent, ctx->parent);

    // 3. 将请求投递到目标队列
    size_t old_count = target_handle->request_queue_.unsafeSize();
    // 队列入队 + 原子变量更新
    target_handle->request_queue_.push(ctx);
    Logger::getInstance().debug("send request ", ptrToString(ctx),
                                " to handle (multi: ", ptrToString(target_handle->multi_handle),
                                "), queue_size: ", target_handle->request_queue_.unsafeSize(), " -> ",
                                old_count, ", active: ", target_handle->active_count, " url: ", ctx->url);

    // 4. 唤醒目标线程处理新请求（让活跃态 curl_multi_wait 与空闲态 idle wait 都能立即返回）。
    target_handle->Notify();
    return future;
}

AsyncHttpClient::RequestContext* AsyncHttpClient::getRequest(CurlMultiHandle& multi_handle,
                                                             int64_t* next_request_not_before_ms) {
    std::thread::id thread_id = std::this_thread::get_id();
    RequestContext* target_ctx = nullptr;
    int64_t next_not_before_ms = -1;

    // 循环获取：直到找到符合发送时间的ctx，或队列为空。
    // 当队列中的请求都尚未到 not_send_util_ 时，顺手记录最早可发送时间，供 idle wait 使用。
    size_t queue_len = multi_handle.request_queue_.unsafeSize();

    int i = 0;
    // 防止push进去循环判断
    while (i < queue_len) {
        i++;

        // 1. 当前没人投递请求，且没有请求在运行
        RequestContext* ctx = multi_handle.request_queue_.pop();

        // 2. 校验ctx有效性（避免空指针）
        if (!ctx) {
            Logger::getInstance().error("getRequest() thread ", thread_id,
                                        " popped null RequestContext, skip");
            continue;
        }

        // 3. not_send_util_是否已到达（当前时间 >= 允许发送时间）
        int64_t current_time = multi_handle.current_ms.load();  // 需确保该函数返回毫秒级时间戳
        if (current_time >= ctx->not_send_util_) {
            // 已到发送时间，返回该ctx
            Logger::getInstance().debug(
                "getRequest() thread ", thread_id, " retrieved valid RequestContext ", ptrToString(ctx),
                ", not_send_util_:", ctx->not_send_util_, ", current_time:", current_time,
                ", remaining queue size: ", multi_handle.request_queue_.unsafeSize());
            target_ctx = ctx;
            break;
        } else {
            // 未到发送时间，放回队列尾部，继续取下一个
            multi_handle.request_queue_.push(ctx);  // 放回队列（需确保queue的push线程安全）
            if (ctx->not_send_util_ > 0 &&
                (next_not_before_ms < 0 || ctx->not_send_util_ < next_not_before_ms)) {
                next_not_before_ms = ctx->not_send_util_;
            }

            Logger::getInstance().debug("getRequest() thread ", thread_id, " RequestContext ",
                                        ptrToString(ctx),
                                        " not ready (not_send_util_:", ctx->not_send_util_,
                                        ", current_time:", current_time, "), pushed back to queue");
        }
    }

    if (next_request_not_before_ms != nullptr) {
        *next_request_not_before_ms = next_not_before_ms;
    }

    return target_ctx;
}

// todo: https协议测试
CURLcode AsyncHttpClient::addRequestToEventor(CURLM* multi_handle, RequestContext* ctx) const {
    ctx->multi_handle = multi_handle;

    CURLcode res = CURLE_FAILED_INIT;
    const std::shared_ptr<HttpRequest> request = ctx->request;
    Logger::getInstance().debug("addRequestToMulti() for URL: ", ctx->url,
                                ", multi_handle: ", ptrToString(multi_handle));

    auto fail_request = [ctx](const std::string& err_msg, const CURLcode code) {
        ctx->response->setStatus(http::otherErr);
        ctx->response->setStatusMsg(err_msg);
        ctx->response->setCurlErrCode(code);
        if (ctx->on_request_finished_) {
            ctx->on_request_finished_(ctx->response);
        }
        try {
            ctx->promise.set_exception(std::make_exception_ptr(std::runtime_error(err_msg)));
        } catch (...) {
        }
    };

    // 处理ctx->curl_handle泄露和ctx泄露
    CURL* curl = ctx->curl_handle;
    if (!curl) {
        Logger::getInstance().error("Failed to create curl handle for URL: ", ctx->url);
        fail_request("Failed to create curl handle", CURLE_FAILED_INIT);
        return CURLE_FAILED_INIT;
    }

    // 设置URL
    res = curl_easy_setopt(curl, CURLOPT_URL, ctx->url.c_str());
    if (res != CURLE_OK) {
        const std::string err_msg = std::string("Failed to set URL: ") + curl_easy_strerror(res);
        Logger::getInstance().error("Failed to set URL for ", ctx->url, ": ", curl_easy_strerror(res));
        fail_request(err_msg, res);
        return res;
    }

    // set opt for different http methods
    if (request->method() == http::MethodGet) {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        ctx->is_read = true;
    } else if (request->method() == http::MethodHead) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
        ctx->is_read = true;
    } else if (request->method() == http::MethodPut) {
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        if (ctx->is_chunked) {
            curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
        } else {
            // make sure httpRequest content length has been set
            const curl_off_t bodySize = ctx->request->getContentLength();
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodySize);
        }
        ctx->is_read = false;
    } else if (request->method() == http::MethodPost) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (ctx->is_chunked) {
            curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
        } else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request->getContentLength());
        }
        ctx->is_read = false;
    } else if (request->method() == http::MethodDelete) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        ctx->is_read = false;
    }

    if (ctx->is_read) {
        // todo：有些case应该不用算crc，要真有数据才需要算，404的body理论上不用管了
        ctx->recv_crc64_value = ctx->request->getPreHashCrc64Ecma();
    } else {
        ctx->send_crc64_value = ctx->request->getPreHashCrc64Ecma();
    }

    // 设置请求头
    curl_slist* list = nullptr;
    auto& headers = request->Headers();
    for (const auto& p : headers) {
        if (p.second.empty()) continue;
        std::string str(p.first);
        str.append(":").append(p.second);
        list = curl_slist_append(list, str.c_str());
    }
    // Disable Expect: 100-continue
    list = curl_slist_append(list, "Expect:");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    ctx->headers = list;
    // add user-agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, VolcengineTos::DefaultUserAgent().c_str());

    if (transport_config_.isDetailLog()) {
        Logger::getInstance().info("HTTP Request: ", request->method(), " ", ctx->url);
        curl_slist* temp = ctx->headers;  // 临时指针，避免修改原链表
        while (temp != nullptr) {
            Logger::getInstance().info("    ", temp->data);
            temp = temp->next;
        }
    }

    // 设置读写回调
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &AsyncHttpClient::headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, ctx);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &AsyncHttpClient::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ctx);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, &AsyncHttpClient::readCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, ctx);

    // 调试模式
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, &HttpClient::debugCallback);
    // Logger::getInstance().info("Enabled verbose mode for URL: ", url);

    // 将请求添加到multi handle
    CURLMcode mres = curl_multi_add_handle(multi_handle, curl);
    if (mres != CURLM_OK) {
        const std::string err_msg =
            std::string("Failed to add handle to multi: ") + curl_multi_strerror(mres);
        Logger::getInstance().error("Failed to add handle to multi for URL: ", ctx->url, ": ",
                                    curl_multi_strerror(mres));
        fail_request(err_msg, CURLE_FAILED_INIT);
        return CURLE_FAILED_INIT;
    }

    // 将上下文指针存储在curl句柄中
    curl_easy_setopt(curl, CURLOPT_PRIVATE, ctx);

    if (ctx->on_request_start_) {
        ctx->on_request_start_();
    }

    Logger::getInstance().debug("Associated RequestContext with curl handle for URL: ", ctx->url);
    return CURLE_OK;
}

void AsyncHttpClient::processCompleteActiveFds(CURLM* multi_handle, std::thread::id& thread_id,
                                               std::vector<curl_waitfd>& extra_fds,
                                               const std::set<RequestContext*>& active_ctxs) {
    // for (size_t i = 0; i < extra_fds.size(); ++i) {
    //     if (extra_fds[i].revents & (POLLERR | POLLHUP)) {
    //         RequestContext* err_ctx = active_ctxs[i];
    //         Logger::getInstance().error("Worker thread ", thread_id, " file error (fd: ",
    //         extra_fds[i].fd,
    //                                     ") for URL: ", err_ctx->request->url().toString());
    //         if (extra_fds[i].fd != err_ctx->file_fd) {
    //             Logger::getInstance().error("Worker thread ", thread_id, " file error (fd: ",
    //             extra_fds[i].fd,
    //                                         ") for URL, but not match: ",
    //                                         err_ctx->request->url().toString());
    //             continue;
    //         }
    //         // 找到对应的活跃请求（extra_fds与active_requests顺序一致）
    //         // 终止异常请求？？？？
    //         err_ctx->completed = true;
    //         curl_multi_remove_handle(multi_handle, err_ctx->curl_handle);
    //     }
    // }
}

static CURLcode fillHttpResponseStats(const int64_t current_time, CURL* curl, HttpResponse& response,
                                      HttpRequest& request) {
    if (!curl) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    CURLcode res = CURLE_OK;
    CurlStats& stats = response.getCurlStats();
    char* str_buf = nullptr;  // 临时存储libcurl返回的字符串

    // ========================== 1. 时间类统计（毫秒级，核心字段）==========================
    // 域名解析耗时（CURLINFO_NAMELOOKUP_TIME：秒 → 毫秒）
    res = curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &stats.getNamelookupTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_NAMELOOKUP_TIME: ", curl_easy_strerror(res));
    }

    res = curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &stats.getTlsConnectTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_APPCONNECT_TIME: ", curl_easy_strerror(res));
    }

    // 连接耗时（CURLINFO_CONNECT_TIME：秒 → 毫秒）
    res = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &stats.getConnectTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_CONNECT_TIME: ", curl_easy_strerror(res));
    }

    // 请求总耗时
    res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &stats.getTotalTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_TOTAL_TIME: ", curl_easy_strerror(res));
    }

    res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &stats.getRedirectTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_REDIRECT_TIME: ", curl_easy_strerror(res));
    }

    // 首字节响应耗时（CURLINFO_STARTTRANSFER_TIME：秒 → 毫秒）
    res = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &stats.getStartTransferTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_STARTTRANSFER_TIME: ", curl_easy_strerror(res));
    }

    res = curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &stats.getPreTransTime());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_PRETRANSFER_TIME: ", curl_easy_strerror(res));
    }

    // ========================== 2. 字节统计类（核心字段）==========================
    // 下载响应体总字节数
    res = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &stats.getSizeDownload());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_SIZE_DOWNLOAD: ", curl_easy_strerror(res));
    }

    // 上传请求体总字节数
    res = curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &stats.getSizeUpload());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_SIZE_UPLOAD: ", curl_easy_strerror(res));
    }

    // 响应头部总字节数
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &stats.getHeaderSize());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_HEADER_SIZE: ", curl_easy_strerror(res));
    }

    // 请求总字节数（头部+体）
    res = curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &stats.getRequestSize());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_REQUEST_SIZE: ", curl_easy_strerror(res));
    }

    // 服务器声明的响应体长度（Content-Length）
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &stats.getContentLengthDownload());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_CONTENT_LENGTH_DOWNLOAD: ",
                                   curl_easy_strerror(res));
    }

    // 上传请求体长度
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &stats.getContentLengthUpload());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_CONTENT_LENGTH_UPLOAD: ",
                                   curl_easy_strerror(res));
    }

    // ========================== 3. 连接信息类（非核心字段）==========================
    // 远程服务器主IP
    res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &str_buf);
    if (res == CURLE_OK && str_buf != nullptr) {
        stats.setPrimaryIp(str_buf);
    } else {
        stats.setPrimaryIp("");
        Logger::getInstance().warn("Failed to get CURLINFO_PRIMARY_IP: ", curl_easy_strerror(res));
    }

    // 远程服务器端口
    res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &stats.getPrimaryPort());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_PRIMARY_PORT: ", curl_easy_strerror(res));
    }

    // 本地出口IP
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &str_buf);
    if (res == CURLE_OK && str_buf != nullptr) {
        stats.setLocalIp(str_buf);
    } else {
        Logger::getInstance().warn("Failed to get CURLINFO_LOCAL_IP: ", curl_easy_strerror(res));
    }

    // 本地临时端口
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &stats.getLocalPort());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_LOCAL_PORT: ", curl_easy_strerror(res));
    }

    // 本请求连接创建总数
    res = curl_easy_getinfo(curl, CURLINFO_NUM_CONNECTS, &stats.getNumConnects());
    if (res != CURLE_OK) {
        Logger::getInstance().warn("Failed to get CURLINFO_NUM_CONNECTS: ", curl_easy_strerror(res));
    }

    // SSL证书验证结果（0=成功）
    res = curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &stats.getSslVerifyResult());
    if (res != CURLE_OK) {
        stats.getSslVerifyResult() = -1;
        Logger::getInstance().warn("Failed to get CURLINFO_SSL_VERIFYRESULT: ", curl_easy_strerror(res));
    }

    // ========================== 5. 其他统计字段 ==========================
    // 请求重试退避，服务端需携带Retry-After头
    //    res = curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &stats.getRetryAfter());
    //    if (res != CURLE_OK) {
    //        Logger::getInstance().debug("Failed to get CURLINFO_RETRY_COUNT: ", curl_easy_strerror(res));
    //    }

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &stats.getStatusCode());
    if (res != CURLE_OK) {
        Logger::getInstance().debug("Failed to get CURLINFO_RESPONSE_CODE: ", curl_easy_strerror(res));
    }

    response.getCurlStats().setAction(request.userName());
    response.getCurlStats().setE2eStartTime(request.realStartTimeMs());
    response.getCurlStats().setE2eTime(static_cast<double>(current_time - request.realStartTimeMs()));

    // 单请求的日志
    // Logger::getInstance().error(response.getCurlStats().toString());
    CurlStatsCollector::getInstance().addCurlStats(response.getCurlStats());
    return CURLE_OK;
}

void AsyncHttpClient::processRequestDone(CurlMultiHandle& multi_handle, CurlCache& curl_cache,
                                         const CURLMsg* msg, int& messages_left) const {
    CURL* curl = msg->easy_handle;
    void* ptr;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &ptr);
    const auto ctx = static_cast<RequestContext*>(ptr);

    const CURLcode res = msg->data.result;
    if (res == CURLE_COULDNT_CONNECT) {
        ctx->response->setStatus(http::Refused);
        std::stringstream ss;
        ss << "curlCode: " << res << ", " << curl_easy_strerror(res);
        ctx->response->setStatusMsg(ss.str());
        ctx->response->setCurlErrCode(res);
    } else if (res != CURLE_OK) {
        ctx->response->setStatus(http::otherErr);
        std::stringstream ss;
        ss << "curlCode: " << res << ", " << curl_easy_strerror(res);
        ctx->response->setStatusMsg(ss.str());
        ctx->response->setCurlErrCode(res);
    } else {
        ctx->response->setStatus(http::Success);
        if (ctx->is_read) {
            ctx->response->setHashCrc64Result(ctx->recv_crc64_value);
        } else {
            ctx->response->setHashCrc64Result(ctx->send_crc64_value);
        }
    }

    ctx->completed = true;

    if (transport_config_.isDetailLog()) {
        Logger::getInstance().info("Http Response: ", ctx->url);
        if (ctx->response->Headers().empty()) {
        } else {
            const std::map<std::string, std::string>& headers = ctx->response->Headers();
            for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end();
                 ++it) {
                const std::string& key = it->first;
                const std::string& value = it->second;
                if (value.empty()) {
                    Logger::getInstance().info("    ", key, ":");
                } else {
                    Logger::getInstance().info("    ", key, ":", value);
                }
            }
        }
    }

    Logger::getInstance().info("Request fully completed (body and callback) - URL: ", ctx->url,
                               ", Request Number to process: ", messages_left, " cached_buffer_queue_empty",
                               ctx->cached_buffer_queue_.empty(), ", receive=", ctx->receive,
                               " send=", ctx->send);
    ctx->on_request_finished_(ctx->response);

    const auto it = multi_handle.active_set_.find(ctx);
    if (it != multi_handle.active_set_.end()) {
        multi_handle.active_set_.erase(it);
        multi_handle.active_count--;
    } else {
        Logger::getInstance().error(
            "Request fully completed (body and callback) but can't find in active_set - URL: ", ctx->url,
            ", Request Number to process: ", messages_left, " cached_buffer_queue_empty",
            ctx->cached_buffer_queue_.empty(), ", receive=", ctx->receive, " send=", ctx->send);
    }

    // 因为 future 已在 header 完成时唤醒，此处只需处理错误和资源清理
    if (msg->data.result != CURLE_OK) {
        //        Logger::getInstance().error("Request failed: ", curl_easy_strerror(msg->data.result));
        // 若 header 未完成就出错，需唤醒 future 传递异常
        if (!ctx->headers_completed_) {
            ctx->promise.set_exception(
                std::make_exception_ptr(std::runtime_error(curl_easy_strerror(msg->data.result))));
        }
    }

    // 填充响应性能数据
    if (transport_config_.isDetailLog()) {
        fillHttpResponseStats(multi_handle.current_ms.load(), ctx->curl_handle, *ctx->response,
                              *ctx->request);
    }

    // curl_cache.Release(ctx->curl_handle, msg->data.result != CURLE_OK);
    // ctx->curl_handle = nullptr;
    curl_multi_remove_handle(multi_handle.multi_handle, curl);
    removeFromPausedQueue(multi_handle, ctx);

    delete ctx;
}

void AsyncHttpClient::processCompletedRequests(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                               CurlCache& curl_cache,
                                               std::vector<curl_waitfd>& extra_fds) const {
    Logger::getInstance().debug("processCompletedRequests() called for multi_handle: ",
                                ptrToString(multi_handle.multi_handle));

    processCompleteActiveFds(multi_handle.multi_handle, thread_id, extra_fds, multi_handle.active_set_);

    int messages_left = 0;
    CURLMsg* msg;

    while ((msg = curl_multi_info_read(multi_handle.multi_handle, &messages_left)) != nullptr) {
        if (msg->msg == CURLMSG_DONE) {
            processRequestDone(multi_handle, curl_cache, msg, messages_left);
        } else {
            // 理论上无第二种
            Logger::getInstance().error("Worker thread ", thread_id,
                                        " curl_multi_info_read failed: ", msg->msg);
        }
    }
}

void AsyncHttpClient::updateCurrentTime(CurlMultiHandle& multi_handle) {
    const time_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count();
    multi_handle.current_ms.store(now_ms);
}

void AsyncHttpClient::tryPrintLog(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                  const CurlCache& curl_cache) const {
    const int PRINT_INTERVAL_MS = transport_config_.getDetailLogInterval() * 1000;
    if (multi_handle.current_ms - multi_handle.last_print_ms < PRINT_INTERVAL_MS) {
        return;
    }

    // 打印RequestContext全局计数（创建/释放/存活）
    uint64_t create_cnt = g_req_ctx_create_cnt.load();
    uint64_t destroy_cnt = g_req_ctx_destroy_cnt.load();
    uint64_t alive_cnt = create_cnt - destroy_cnt;
    Logger::getInstance().warn("[Thread ", thread_id, "] RequestContext Stats: ", "Created=", create_cnt,
                               ", Destroyed=", destroy_cnt, ", Alive=", alive_cnt);

    try {
        // 1. 获取各队列的元素快照
        // const auto req_ctxs = multi_handle.request_queue_.traverse();
        const auto paused_ctxs = multi_handle.paused_queue_.traverse();

        // 2. 打印request_queue_状态（大小+每个请求URL）
        size_t req_size = multi_handle.request_queue_
                              .unsafeSize();  // 使用traverse返回的向量大小，更准确反映实际遍历到的元素
        Logger::getInstance().warn("[Thread ", thread_id, "] request_queue_: Size=", req_size);
        // for (size_t i = 0; i < req_size; ++i) {
        //     RequestContext* ctx = req_ctxs[i];  // 从向量中直接获取元素
        //     if (ctx) {
        //         Logger::getInstance().warn("  ├─ request_queue_[", i, "]: URL=", ctx->url,
        //                                    ", Completed=", ctx->completed, ", Send=", ctx->send,
        //                                    ", Receive=", ctx->receive);
        //     }
        // }

        // 3. 打印paused_queue_状态（大小+暂停信息）
        size_t paused_size = paused_ctxs.size();
        Logger::getInstance().warn("[Thread ", thread_id, "] paused_queue_: Size=", paused_size);
        for (size_t i = 0; i < paused_size; ++i) {
            RequestContext* ctx = paused_ctxs[i];
            if (ctx) {
                Logger::getInstance().warn("  ├─ paused_queue_[", i, "]: URL=", ctx->url,
                                           ", EventStatus=", (int)ctx->event_->getState(),
                                           ", WaitUntil=", ctx->event_->getResumeAt(), ", Send=", ctx->send,
                                           ", Receive=", ctx->receive);
            }
        }

        // 4. 打印active_queue_状态（大小+活跃信息）
        Logger::getInstance().warn("[Thread ", thread_id,
                                   "] active_set_: Size=", multi_handle.active_count);
        int i = 0;
        for (auto it = multi_handle.active_set_.begin(); it != multi_handle.active_set_.end(); ++it, i++) {
            // 访问元素：it 指向 const RequestContext，不可修改
            const RequestContext* ctx = *it;
            Logger::getInstance().warn("  └─ active_queue_[", i, "]: URL=", ctx->url,
                                       ", Completed=", ctx->completed, ", Send=", ctx->send,
                                       ", Receive=", ctx->receive);
        }

        Logger::getInstance().warn(curl_cache.printStatus());

        Logger::getInstance().warn(CurlStatsCollector::getInstance().toString());

    } catch (const std::exception& e) {
        Logger::getInstance().warn("tryPrintLog unknown log error:", e.what());
    } catch (...) {
        Logger::getInstance().warn("tryPrintLog unknown log error.");
    }

    multi_handle.last_print_ms.store(multi_handle.current_ms.load());
}

void AsyncHttpClient::processNewRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                        CurlCache& curl_cache) const {
    if (multi_handle.active_count >= multi_handle.max_connection_num) {
        return;
    }

    int i = 0;
    do {
        i++;
        int64_t next_request_not_before_ms = -1;
        auto ctx = getRequest(multi_handle, &next_request_not_before_ms);
        if (!ctx) {
            multi_handle.next_request_not_before_ms.store(next_request_not_before_ms,
                                                          std::memory_order_relaxed);
            break;
        }
        multi_handle.next_request_not_before_ms.store(-1, std::memory_order_relaxed);
        ctx->curl_handle = curl_cache.Acquire();
        if (ctx->curl_handle) {
            Logger::getInstance().debug("Worker thread ", thread_id,
                                        " processing request to URL: ", ctx->url);
            const CURLcode res = addRequestToEventor(multi_handle.multi_handle, ctx);
            if (res != CURLE_OK) {
                delete ctx;
                break;
            }
            multi_handle.active_set_.insert(ctx);
            multi_handle.active_count++;
        } else {
            Logger::getInstance().error("Worker thread ", thread_id, " can't processing request to URL.");
            ctx->response->setStatus(http::otherErr);
            ctx->response->setStatusMsg("can't processing request to URL.");
            ctx->on_request_finished_(ctx->response);
            ctx->promise.set_exception(
                std::make_exception_ptr(std::runtime_error("can't processing request to URL")));
            delete ctx;
            break;
        }
        if (multi_handle.active_count >= multi_handle.max_connection_num) {
            return;
        }
    } while (i < 5);  // 每次尽量放五个请求
}

void AsyncHttpClient::processActiveFds(const CurlMultiHandle& multi_handle,
                                       std::vector<curl_waitfd>& extra_fds,
                                       const std::set<RequestContext*>& active_ctxs) {
    extra_fds.clear();

    // 1) eventfd：用于跨线程唤醒 curl_multi_wait（新请求入队/外部恢复等）
    if (multi_handle.wake_fd >= 0) {
        curl_waitfd wake{};
        wake.fd = multi_handle.wake_fd;
        wake.events = CURL_WAIT_POLLIN;
        wake.revents = 0;
        extra_fds.push_back(wake);
    }

    // 2) TODO：文件/管道等外部 fd
    // for (const auto ctx : active_ctxs) {
    //     if (!ctx->completed && ctx->file_fd != -1) {
    //         curl_waitfd fd_info{};
    //         fd_info.fd = ctx->file_fd;
    //         fd_info.events = CURL_WAIT_POLLERR | CURL_WAIT_POLLHUP;
    //         fd_info.revents = 0;
    //         extra_fds.push_back(fd_info);
    //     }
    // }
    (void)active_ctxs;
}

bool AsyncHttpClient::hasRunnableActive(const CurlMultiHandle& multi_handle) {
    const int64_t now_ms = multi_handle.current_ms.load(std::memory_order_relaxed);
    for (const auto* ctx : multi_handle.active_set_) {
        if (ctx == nullptr || ctx->completed) {
            continue;
        }
        if (ctx->event_ != nullptr) {
            if (ctx->event_->isStopping()) {
                continue;
            }
            if (ctx->event_->isWaiting()) {
                const int64_t resume_at_ms = ctx->event_->getResumeAt();
                if (resume_at_ms > 0 && now_ms < resume_at_ms) {
                    continue;
                }
            }
        }
        if (!ctx->cached_buffer_queue_.empty()) {
            continue;
        }
        return true;
    }
    return false;
}

int64_t AsyncHttpClient::getNextIdleWakeupMs(const CurlMultiHandle& multi_handle) {
    int64_t next_wakeup_ms = multi_handle.next_request_not_before_ms.load(std::memory_order_relaxed);
    const auto paused_ctxs = multi_handle.paused_queue_.traverse();
    for (const auto* ctx : paused_ctxs) {
        if (ctx == nullptr || ctx->event_ == nullptr) {
            continue;
        }
        if (multi_handle.active_set_.find(const_cast<RequestContext*>(ctx)) ==
            multi_handle.active_set_.end()) {
            continue;
        }
        const int64_t event_resume_at_ms = ctx->event_->isWaiting() ? ctx->event_->getResumeAt() : 0;
        if (event_resume_at_ms > 0 && (next_wakeup_ms < 0 || event_resume_at_ms < next_wakeup_ms)) {
            next_wakeup_ms = event_resume_at_ms;
        }
    }
    return next_wakeup_ms;
}

void AsyncHttpClient::processActiveRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id,
                                           std::vector<curl_waitfd>& extra_fds) {
    int still_running = 0;
    CURLMcode res;
    do {
        res = curl_multi_perform(multi_handle.multi_handle, &still_running);
    } while (res == CURLM_CALL_MULTI_PERFORM);

    if (res != CURLM_OK) {
        Logger::getInstance().error("Worker thread ", thread_id,
                                    " curl_multi_perform failed: ", curl_multi_strerror(res));
        return;
    }

    // res == CURLM_OK
    processActiveFds(multi_handle, extra_fds, multi_handle.active_set_);

    // 当前轮 perform 已经把所有 runnable 传输都推进过一遍；若 still_running == 0，
    // 说明这轮没有继续阻塞在 curl_multi_wait 的必要，直接回到 worker 主循环处理 completed /
    // paused / idle 分支即可。
    if (still_running == 0) {
        multi_handle.DrainWakeFd();
        return;
    }

    int numfds = 0;
    res = curl_multi_wait(multi_handle.multi_handle, extra_fds.empty() ? nullptr : extra_fds.data(),
                          extra_fds.size(), multi_handle.curl_multi_wait_timeout_ms, &numfds);
    Logger::getInstance().debug("Worker thread ", thread_id,
                                " calling curl_multi_wait with numfds: ", numfds,
                                ", still_running=", still_running);

    if (res != CURLM_OK) {
        Logger::getInstance().error("Worker thread ", thread_id,
                                    " calling curl_multi_wait with failed, numfds: ", numfds,
                                    " error: ", curl_multi_strerror(res));
    }

    // 若被 eventfd 唤醒，清空计数，避免下一次空转。
    // 这里保留显式 Drain 是因为 eventfd 是计数语义；如果不及时读空，后续 wait 可能会被历史
    // 唤醒残留连续打断，反过来放大 busy loop。
    multi_handle.DrainWakeFd();
}

void AsyncHttpClient::enqueuePausedRequest(CurlMultiHandle& multi_handle, RequestContext* ctx) {
    if (ctx == nullptr) {
        return;
    }
    bool expected = false;
    if (!ctx->in_paused_queue.compare_exchange_strong(expected, true, std::memory_order_acq_rel,
                                                      std::memory_order_acquire)) {
        Logger::getInstance().info("skip duplicate paused enqueue, ctx=", ptrToString(ctx), ", url=", ctx->url,
                                   ", event=", ctx->event_ ? ctx->event_->toString() : std::string("null"));
        return;
    }
    Logger::getInstance().info("enqueue paused request, ctx=", ptrToString(ctx), ", url=", ctx->url,
                               ", event=", ctx->event_ ? ctx->event_->toString() : std::string("null"));
    multi_handle.paused_queue_.push(ctx);
}

void AsyncHttpClient::waitForWorkOrResume(CurlMultiHandle& multi_handle) {
    const int64_t next_wakeup_ms = getNextIdleWakeupMs(multi_handle);
    auto has_immediate_work = [&multi_handle]() {
        if (!multi_handle.request_queue_.unsafeEmpty()) {
            return true;
        }

        const auto now_ms = multi_handle.current_ms.load();
        const auto paused_ctxs = multi_handle.paused_queue_.traverse();
        for (const auto* ctx : paused_ctxs) {
            if (ctx == nullptr || ctx->event_ == nullptr) {
                continue;
            }
            if (multi_handle.active_set_.find(const_cast<RequestContext*>(ctx)) ==
                multi_handle.active_set_.end()) {
                continue;
            }
            if (ctx->event_->isFailed()) {
                return true;
            }
            if (ctx->event_->isStopping()) {
                continue;
            }
            if (!ctx->cached_buffer_queue_.empty()) {
                return true;
            }
            bool user_pause_active = ctx->event_->isStopping();
            if (ctx->event_->isWaiting()) {
                const int64_t resume_at_ms = ctx->event_->getResumeAt();
                if (resume_at_ms > 0 && now_ms < resume_at_ms) {
                    user_pause_active = true;
                }
            }
            if (!user_pause_active) {
                return true;
            }
        }
        return false;
    };

    // 在真正进入 wait 前记录当前世代。后面的 predicate 比较“现在的 wake_seq 是否还等于
    // 我准备睡时看到的 wake_seq”。如果其间恰好有 Notify() 发生，即便那次 notify_one()
    // 没被 wait 这次边沿接住，predicate 也会因为 wake_seq 已变化而立即返回，避免丢通知。
    const uint64_t wake_seq = multi_handle.wake_seq.load(std::memory_order_relaxed);

    std::unique_lock<std::mutex> lock(multi_handle.queue_mutex_);
    if (!multi_handle.running.load(std::memory_order_acquire)) {
        return;
    }
    if (has_immediate_work()) {
        // TODO(review): 仅靠 wake_seq 只能记住“睡下之后有没有新的 notify 边沿”，记不住“当前队列里
        // 已经存在可立即处理的 paused/request work”。当 AsyncEvent::markResumed() 恰好发生在
        // worker 决定 idle 但尚未真正 wait 的窗口里时，wake_seq 可能已经被新的快照吞掉；如果
        // 这里不再主动复查 paused/request 状态，worker 会抱着一个已经 resume 的请求永久睡死。
        return;
    }

    // queue_cv_ 只是停车点；真正的唤醒条件是：
    //   1) worker 被要求退出；或
    //   2) 自己准备睡眠之后发生过新的外部事件（由 wake_seq 变化表示）。
    // 若没有 wake_seq，这里只能写成“被 notify 就醒”，那就会暴露如下竞态：
    //   T1(waiter): 判断当前无 runnable active，准备 wait
    //   T2(producer): 入队请求并 notify_one
    //   T1(waiter): 此时尚未真正阻塞，因此错过这次 notify 边沿
    //   T1(waiter): 随后睡下去，已有工作却没人再叫醒它
    // 使用 generation counter 后，T2 会先 ++wake_seq；于是 T1 即使错过 notify 边沿，
    // 也会在 wait 的 predicate 检查里观察到 wake_seq != snapshot，从而不再睡眠。
    auto should_wake = [&multi_handle, wake_seq, &has_immediate_work]() {
        return !multi_handle.running.load(std::memory_order_acquire) ||
               multi_handle.wake_seq.load(std::memory_order_relaxed) != wake_seq || has_immediate_work();
    };

    if (next_wakeup_ms > 0) {
        const auto deadline =
            std::chrono::system_clock::time_point(std::chrono::milliseconds(next_wakeup_ms));
        multi_handle.queue_cv_.wait_until(lock, deadline, should_wake);
    } else if (multi_handle.active_count.load(std::memory_order_relaxed) > 0 ||
               !multi_handle.paused_queue_.empty()) {
        // TODO(review): 仅靠外部 notify 驱动 paused-only worker 存在单点风险：只要上游恢复边沿没有
        // 成功送达（例如第三方异步源未注册 notifier、或某条恢复路径未来回归遗漏 Notify），
        // worker 就会抱着 active/paused 请求永久睡死。这里在“仍有 active/paused 请求但当前无
        // runnable work”的场景下退化成短周期条件等待，把恢复检测改成 level-triggered 兜底，
        // 让 `processPausedRequest()` 至少按 `curl_multi_wait_timeout_ms` 周期重新扫描一次。
        multi_handle.queue_cv_.wait_for(lock, std::chrono::milliseconds(multi_handle.curl_multi_wait_timeout_ms),
                                        should_wake);
    } else {
        multi_handle.queue_cv_.wait(lock, should_wake);
    }

    lock.unlock();
    multi_handle.DrainWakeFd();
}

void AsyncHttpClient::removeFromPausedQueue(CurlMultiHandle& multi_handle, RequestContext* target_ctx) {
    if (target_ctx == nullptr) {
        return;
    }

    std::vector<RequestContext*> to_keep;
    while (true) {
        auto* paused_ctx = multi_handle.paused_queue_.pop();
        if (!paused_ctx) {
            break;
        }
        paused_ctx->in_paused_queue.store(false, std::memory_order_release);
        if (paused_ctx != target_ctx) {
            to_keep.push_back(paused_ctx);
        }
    }

    for (auto* ctx : to_keep) {
        enqueuePausedRequest(multi_handle, ctx);
    }
}

void AsyncHttpClient::processPausedRequest(CurlMultiHandle& multi_handle, std::thread::id& thread_id) {
    Logger::getInstance().debug("Worker thread ", thread_id, " processPausedRequest.");

    std::vector<RequestContext*> to_resume;
    std::vector<RequestContext*> to_pause;
    // 无锁批量pop：循环取出所有可恢复的请求（避免频繁CAS）
    while (true) {
        // 修改为传入闭包，直接pop省去pushback
        auto* paused_ctx = multi_handle.paused_queue_.pop();
        if (!paused_ctx) {
            break;  // 队空，退出循环
        }
        paused_ctx->in_paused_queue.store(false, std::memory_order_release);
        Logger::getInstance().info("dequeue paused request, ctx=", ptrToString(paused_ctx), ", url=", paused_ctx->url,
                                   ", event=", paused_ctx->event_ ? paused_ctx->event_->toString()
                                                                  : std::string("null"));

        if (multi_handle.active_set_.find(paused_ctx) == multi_handle.active_set_.end()) {
            // 可能是已完成/已清理的上下文遗留在暂停队列里，直接丢弃
            Logger::getInstance().warn("drop paused request not in active_set, ctx=", ptrToString(paused_ctx),
                                       ", url=", paused_ctx->url);
            continue;
        }

        if (paused_ctx->event_->isFailed()) {
            // TODO(review): 数据源/落盘 sink 已经把请求标成终态失败时，不能继续留在 paused queue 里等待；
            // 必须先恢复 easy handle，让下一轮回调把失败传回 libcurl/SDK，避免 future 永久不完成。
            to_resume.push_back(paused_ctx);
            continue;
        }

        bool wait_ok = false;
        bool buff_send_ok = true;

        const time_t now_ms = multi_handle.current_ms.load();
        if (paused_ctx->event_->isStopping()) {
            to_pause.push_back(paused_ctx);
            continue;
        }

        const auto resume_at = paused_ctx->event_->getResumeAt();
        if (resume_at <= 0 || now_ms >= resume_at) {
            wait_ok = true;
        }

        // 触发cache暂停继续回调用户callback（带事件）
        if (wait_ok && !paused_ctx->cached_buffer_queue_.empty()) {
            AsyncEvent* ev = paused_ctx->event_.get();
            ev->setReason("from cache");
            const std::size_t drained_from_cache = paused_ctx->cached_buffer_queue_.drainTo(
                [paused_ctx, ev](char* data, std::size_t len) -> std::size_t {
                    Logger::getInstance().info("consume buffered request, buff len=", len);
                    if (paused_ctx->is_read) {
                        const DownloadConsumeResult result = consumeDownloadData(paused_ctx, data, len, ev);
                        return result.invalid_result || result.terminal_failure ? static_cast<size_t>(0)
                                                                               : result.consumed;
                    }

                    size_t used = 0;
                    if (paused_ctx->on_data_send_with_event_) {
                        used = paused_ctx->on_data_send_with_event_(data, len, ev);
                    } else {
                        used = len;
                    }
                    if (used > len) {
                        return static_cast<size_t>(0);
                    }
                    paused_ctx->send += used;
                    if (paused_ctx->enable_crc_ && used > 0) {
                        paused_ctx->send_crc64_value = CRC64::CalcCRC(paused_ctx->send_crc64_value, data, used);
                    }
                    return used;
                });

            if (paused_ctx->is_read && drained_from_cache > 0) {
                // libcurl 在解除 pause 后会把原始 chunk 再次回放到 write callback。
                // paused queue 里已经从 backlog 重放给下游的这部分字节，也必须累计进 needSkip，
                // 否则 live replay 会把尾部数据再次交付给业务 sink，造成重复消费。
                paused_ctx->needSkip += drained_from_cache;
            }

            // 调用callback后event状态可能会变化，变化的场景返回值也会变成pause
            if (!paused_ctx->cached_buffer_queue_.empty()) {
                buff_send_ok = false;
            }
        }

        if (paused_ctx->event_->isFailed()) {
            to_resume.push_back(paused_ctx);
            continue;
        }

        if (!wait_ok || !buff_send_ok) {
            // 时间没等到，或buff没发完，未满足恢复条件，重新push回队列
            to_pause.push_back(paused_ctx);
        } else {
            to_resume.push_back(paused_ctx);
        }
    }

    // 恢复请求
    for (auto* ctx : to_resume) {
        Logger::getInstance().info("resume request, event: ", ctx->event_->toString(),
                                   " for URL: ", ctx->url);
        if (!ctx->event_->isFailed() && ctx->event_->isPaused()) {
            ctx->event_->Resume();
        }
        // handle 仍在 multi 中，仅需取消 pause
        curl_easy_pause(ctx->curl_handle, CURLPAUSE_CONT);
    }

    // 继续暂停
    for (auto* ctx : to_pause) {
        enqueuePausedRequest(multi_handle, ctx);
    }
}

AsyncHttpClient::PauseDecision AsyncHttpClient::buildPauseDecision(RequestContext* ctx, size_t bytes_used,
                                                                   bool force_pause) {
    PauseDecision decision;
    decision.should_pause = force_pause;

    CurlMultiHandle* multi_handle = ctx->parent;

    if (ctx->rate_limiter != nullptr && bytes_used > 0) {
        auto acquire_res = ctx->rate_limiter->Acquire(static_cast<int64_t>(bytes_used));
        if (acquire_res.first) {
            Logger::getInstance().debug("transfer callback consumed ", bytes_used,
                                        " bytes for URL: ", ctx->url);
        } else {
            decision.should_pause = true;
            const int64_t qos_resume_at = multi_handle->current_ms + acquire_res.second;
            if (qos_resume_at > decision.resume_at_ms) {
                decision.resume_at_ms = qos_resume_at;
                ctx->event_->setReason(std::string("qos: ") + std::to_string(acquire_res.second));
            }
        }
    }

    if (ctx->event_->isStopping()) {
        decision.should_pause = true;
    }

    if (ctx->event_->isWaiting()) {
        decision.should_pause = true;
        const auto resume_at = ctx->event_->getResumeAt();
        if (decision.resume_at_ms == 0 || resume_at > decision.resume_at_ms) {
            decision.resume_at_ms = resume_at;
        }
    }

    if (!ctx->cached_buffer_queue_.empty()) {
        decision.should_pause = true;
    }

    return decision;
}

void AsyncHttpClient::applyPauseState(RequestContext* ctx, int64_t resume_at_ms) {
    if (resume_at_ms > 0) {
        ctx->event_->setResumeAt(resume_at_ms);
    }
}

AsyncHttpClient::DownloadConsumeResult AsyncHttpClient::consumeDownloadData(RequestContext* ctx, char* ptr,
                                                                            size_t data_len,
                                                                            AsyncEvent* event) {
    DownloadConsumeResult result;
    if (ctx == nullptr || ptr == nullptr || data_len == 0) {
        return result;
    }

    if (ctx->on_data_receive_with_event_) {
        result.consumed = ctx->on_data_receive_with_event_(ptr, data_len, event);
    } else {
        result.consumed = data_len;
    }

    if (result.consumed > data_len) {
        Logger::getInstance().error("writeCallback: over used ", result.consumed, "/", data_len,
                                    " bytes for URL: ", ctx->url);
        result.invalid_result = true;
        return result;
    }

    if (event != nullptr && event->isFailed()) {
        result.terminal_failure = true;
        return result;
    }

    ctx->receive += result.consumed;
    if (ctx->enable_crc_ && result.consumed > 0) {
        ctx->recv_crc64_value = CRC64::CalcCRC(ctx->recv_crc64_value, ptr, result.consumed);
    }
    return result;
}

size_t AsyncHttpClient::finalizeUploadCallback(RequestContext* ctx, size_t bytes_read) {
    if (ctx->event_->isFailed()) {
        Logger::getInstance().warn("abort request from terminal async event, url=", ctx->url,
                                   ", event=", ctx->event_->toString());
        return static_cast<size_t>(CURL_READFUNC_ABORT);
    }

    const PauseDecision decision = buildPauseDecision(ctx, bytes_read, false);
    if (!decision.should_pause) {
        return bytes_read;
    }

    Logger::getInstance().info("pause request, event: ", ctx->event_->toString(), " for URL: ", ctx->url);
    applyPauseState(ctx, decision.resume_at_ms);
    enqueuePausedRequest(*ctx->parent, ctx);

    // TODO(review): upload/read callback 中，0-byte 是否 pause 完全由调用方显式 Pause()/PauseFor()
    // 决定；不能再依赖 content-length 推断“0 是暂时没数据还是 EOF”，否则 chunked/unknown-size
    // 上传会把临时无数据误判成 EOF。只有显式 pause 时才返回 CURL_READFUNC_PAUSE。
    curl_easy_pause(ctx->curl_handle, CURLPAUSE_SEND);
    if (bytes_read == 0) {
        return CURL_READFUNC_PAUSE;
    }
    return bytes_read;
}

size_t AsyncHttpClient::finalizeDownloadCallback(RequestContext* ctx, char* ptr, size_t data_len,
                                                 size_t written, size_t skipped) {
    const bool partial_consume = written < data_len;
    if (partial_consume) {
        ctx->cached_buffer_queue_.append(ptr + written, data_len - written);
    }

    const PauseDecision decision = buildPauseDecision(ctx, written, partial_consume);
    if (!decision.should_pause) {
        return skipped + written;
    }

    Logger::getInstance().info("pause request, event: ", ctx->event_->toString(), " for URL: ", ctx->url);
    applyPauseState(ctx, decision.resume_at_ms);
    enqueuePausedRequest(*ctx->parent, ctx);
    ctx->needSkip += skipped + written;
    curl_easy_pause(ctx->curl_handle, CURLPAUSE_RECV);
    return CURL_WRITEFUNC_PAUSE;
}

size_t AsyncHttpClient::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<RequestContext*>(userdata);
    if (!ctx) {
        Logger::getInstance().error("writeCallback: null context");
        return -1;
    }

    size_t data_len = size * nmemb;
    Logger::getInstance().debug("writeCallback: received ", data_len);

    return writeCallbackImpl(ctx, ptr, data_len);
}

size_t AsyncHttpClient::writeCallbackImpl(RequestContext* ctx, char* ptr, size_t data_len) {
    if (data_len == 0) {
        Logger::getInstance().error("empty buf response");
        return 0;
    }

    if (ctx->completed) {
        Logger::getInstance().debug("writeCallback: request already completed for URL: ", ctx->url);
        return 0;
    }

    size_t written = 0;
    size_t skipped = 0;

    // async pause/resume 会让 libcurl 把同一批数据再次送进 callback。
    // 这里显式记录 skipped，是为了保证“暂停前已跳过的数据”在恢复后不会被重复记账，
    // 否则看起来像 on_data_receive 少消费了字节，实际是 AsyncHttpClient 自己把 skip 长度丢了。
    if (ctx->needSkip > 0) {
        skipped = std::min(data_len, ctx->needSkip);
        ptr += skipped;
        ctx->needSkip -= skipped;
        data_len -= skipped;
        if (data_len == 0) {
            return skipped;
        }
    }

    if (ctx->event_->isFailed()) {
        Logger::getInstance().warn("writeCallback abort due to terminal async event for URL: ", ctx->url,
                                   ", event=", ctx->event_->toString());
        return 0;
    }

    const DownloadConsumeResult consume_result = consumeDownloadData(ctx, ptr, data_len, ctx->event_.get());
    if (consume_result.invalid_result) {
        return -1;
    }
    if (consume_result.terminal_failure) {
        Logger::getInstance().warn("writeCallback received terminal async failure after sink callback for URL: ",
                                   ctx->url, ", event=", ctx->event_->toString());
        return 0;
    }
    written = consume_result.consumed;

    if (written == 0 && !ctx->event_->isStopping() && !ctx->event_->isWaiting()) {
        Logger::getInstance().error("writeCallback: zero-byte consume without explicit pause for URL: ",
                                    ctx->url);
        ctx->event_->Fail("write callback consumed zero bytes without pause");
        return 0;
    }

    return finalizeDownloadCallback(ctx, ptr, data_len, written, skipped);
}

size_t AsyncHttpClient::readCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<RequestContext*>(userdata);
    if (!ctx) {
        Logger::getInstance().error("readCallback: null context");
        return -1;
    }

    size_t data_len = size * nmemb;
    Logger::getInstance().debug("readCallback: requesting up to ", data_len, " bytes for URL: ", ctx->url);

    return readCallbackImpl(ctx, ptr, data_len);
}

size_t AsyncHttpClient::readCallbackImpl(AsyncHttpClient::RequestContext* ctx, char* ptr, size_t data_len) {
    if (data_len == 0) {
        return 0;
    }

    if (ctx->event_->isFailed()) {
        Logger::getInstance().warn("readCallback abort due to terminal async event for URL: ", ctx->url,
                                   ", event=", ctx->event_->toString());
        return CURL_READFUNC_ABORT;
    }

    if (ctx->request->getContentLength() > 0 && ctx->send + data_len > ctx->request->getContentLength()) {
        // 保证不越content-length
        data_len = ctx->request->getContentLength() - ctx->send;
    }

    size_t bytes_read = 0;
    if (ctx->on_data_send_with_event_) {
        bytes_read = ctx->on_data_send_with_event_(ptr, data_len, ctx->event_.get());
    }

    if (bytes_read > data_len) {
        Logger::getInstance().error("writeCallback: read over used ", bytes_read, "/", data_len,
                                    " bytes for URL: ", ctx->url);
        return -1;  // 实际消费超过限制
    }

    if (ctx->event_->isFailed()) {
        Logger::getInstance().warn("readCallback received terminal async failure after source callback for URL: ",
                                   ctx->url, ", event=", ctx->event_->toString());
        return CURL_READFUNC_ABORT;
    }

    ctx->send += bytes_read;

    if (ctx->enable_crc_ && bytes_read > 0) {
        ctx->send_crc64_value = CRC64::CalcCRC(ctx->send_crc64_value, ptr, bytes_read);
    }

    if (bytes_read == 0 && !ctx->event_->isPaused() && !ctx->event_->isFailed() &&
        ctx->request->getContentLength() > 0 &&
        ctx->send < static_cast<size_t>(ctx->request->getContentLength())) {
        Logger::getInstance().error("readCallback: zero-byte produce without explicit pause before content-length is "
                                    "fully sent, url=",
                                    ctx->url, ", sent=", ctx->send,
                                    ", content_length=", ctx->request->getContentLength());
        ctx->event_->Fail("read callback produced zero bytes before content-length was fully sent");
        return CURL_READFUNC_ABORT;
    }

    return finalizeUploadCallback(ctx, bytes_read);
}

size_t AsyncHttpClient::headerCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* ctx = static_cast<RequestContext*>(userdata);
    if (!ctx) {
        Logger::getInstance().error("headerCallback: null context");
        return size * nitems;
    }

    size_t total_size = size * nitems;
    std::string header(buffer, total_size);
    Logger::getInstance().debug("headerCallback: received header (", total_size, " bytes): ", header);

    // 空行（"\r\n"）标志着响应头结束（HTTP协议规定）
    const bool is_empty_line = (header == "\r\n" || header == "\n");

    if (!is_empty_line) {
        // 处理普通响应头（如 Content-Type、Content-Length 等）
        const size_t colon_pos = header.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = header.substr(0, colon_pos);
            std::string value = header.substr(colon_pos + 1);

            // 去除前后空格
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            ctx->response->setHeader(name, value);
            Logger::getInstance().debug("headerCallback: parsed header - ", name, ": ", value);
        } else {
            Logger::getInstance().debug("headerCallback: non-standard header line: ", header);
        }
    } else {
        if (!ctx->headers_completed_) {
            // 获取并设置状态码（此时状态码已可用）
            long status_code;
            const CURLcode res = curl_easy_getinfo(ctx->curl_handle, CURLINFO_RESPONSE_CODE, &status_code);
            if (res == CURLE_OK) {
                const int http_status = static_cast<int>(status_code);
                ctx->response->setStatusCode(http_status);
                if (ctx->on_http_status_set_) {
                    ctx->on_http_status_set_(http_status);
                }
                Logger::getInstance().debug("headerCallback: all headers received for URL: ", ctx->url,
                                            ", status code: ", status_code);
            } else {
                Logger::getInstance().error("headerCallback: failed to get status code: ",
                                            curl_easy_strerror(res));
            }

            // 唤醒future（此时response已包含完整的头信息，body流将继续接收数据）
            try {
                ctx->promise.set_value(ctx->response);
                Logger::getInstance().debug("headerCallback: future notified - URL: ", ctx->url);
            } catch (const std::exception& e) {
                Logger::getInstance().error("headerCallback: future notify failed: ", e.what());
            }

            if (ctx->on_content_length_set_) {
                const std::string value = MapUtils::findValueByKeyIgnoreCase(ctx->response->Headers(),
                                                                             http::HEADER_CONTENT_LENGTH);
                if (!value.empty()) {
                    try {
                        const int64_t contentLength = std::stoll(value);
                        ctx->on_content_length_set_(contentLength);
                    } catch (const std::exception& e) {
                        Logger::getInstance().warn("headerCallback: invalid Content-Length: ", value,
                                                   ", err: ", e.what());
                    } catch (...) {
                        Logger::getInstance().warn("headerCallback: invalid Content-Length: ", value,
                                                   ", err: unknown");
                    }
                }
            }

            ctx->headers_completed_ = true;
        }
    }

    return total_size;
}

int AsyncHttpClient::debugCallback(CURL* curl, curl_infotype type, char* data, size_t size,
                                   void* userdata) {
    (void)curl;
    (void)userdata;

    std::string message(data, size);
    // 去除结尾的换行符，避免日志重复换行
    if (!message.empty() && message.back() == '\n') {
        message.pop_back();
    }

    switch (type) {
        case CURLINFO_TEXT:
            Logger::getInstance().debug("curl debug: ", message);
            break;
        case CURLINFO_HEADER_IN:
            Logger::getInstance().debug("curl received header: ", message);
            break;
        case CURLINFO_HEADER_OUT:
            Logger::getInstance().debug("curl sent header: ", message);
            break;
        case CURLINFO_DATA_IN:
            Logger::getInstance().debug("curl received data: (", size, " bytes)");
            break;
        case CURLINFO_DATA_OUT:
            Logger::getInstance().debug("curl sent data: (", size, " bytes)");
            break;
        default:
            Logger::getInstance().debug("curl debug (type ", type, "): ", message);
            break;
    }

    return 0;
}

void AsyncHttpClient::workerThread(CurlMultiHandle& multi_handle) const {
    set_current_thread_name("tos-async-wkr");
    std::thread::id thread_id = std::this_thread::get_id();

    Logger::getInstance().info("AsyncHttpClient worker start, thread_id=", thread_id,
                               ", thread_name=tos-async-wkr, multi_handle=",
                               ptrToString(multi_handle.multi_handle), ", wake_fd=", multi_handle.wake_fd,
                               ", max_connection_num=", multi_handle.max_connection_num);

    while (multi_handle.running) {
        updateCurrentTime(multi_handle);
        if (transport_config_.isDetailLog()) {
            tryPrintLog(multi_handle, thread_id, multi_handle.curl_cache);
        }

        // 1、优先收尾，尽早释放 active_set_/active_count 槽位。
        processCompletedRequests(multi_handle, thread_id, multi_handle.curl_cache, multi_handle.extra_fds);
        updateCurrentTime(multi_handle);

        // 2、恢复到点的 paused 请求。
        processPausedRequest(multi_handle, thread_id);

        // 3、补充投递新的请求到 multi handle。
        processNewRequest(multi_handle, thread_id, multi_handle.curl_cache);

        // 4、根据当前是否存在 runnable active 请求，分别走 active I/O poll 或 idle/pause wait。
        if (hasRunnableActive(multi_handle)) {
            processActiveRequest(multi_handle, thread_id, multi_handle.extra_fds);
        } else {
            waitForWorkOrResume(multi_handle);
        }
    }

    multi_handle.running_finish = true;
    Logger::getInstance().info("AsyncHttpClient worker exit, thread_id=", thread_id,
                               ", multi_handle=", ptrToString(multi_handle.multi_handle), ", wake_fd=",
                               multi_handle.wake_fd);
}
}  // namespace VolcengineTos
