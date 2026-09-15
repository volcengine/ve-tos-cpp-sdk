#include "AsyncLoadHttpFixture.h"
#include "LoopbackHttpFixture.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "logger/logger.h"
#include "utils/crc64.h"
#include <pthread.h>
#include <dirent.h>
#include <sys/resource.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <functional>
#include <iostream>

using namespace VolcengineTos;
using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;
using Server = tos_test::AsyncLoadHttpFixture;
#define CHECK(x)                                                                                       \
    do {                                                                                               \
        if (!(x)) throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " #x); \
    } while (false)
std::atomic<int> unpause_error{0};
std::atomic<int> inject_unpause_error{0};
// Used only on the single owning worker, and cleared before any completion.
AsyncEvent* fail_before_unpause = nullptr;
extern "C" CURLcode __real_curl_easy_pause(CURL*, int);
extern "C" CURLcode __wrap_curl_easy_pause(CURL* handle, int action) {
    if (action == CURLPAUSE_CONT && fail_before_unpause) {
        auto* event = fail_before_unpause;
        fail_before_unpause = nullptr;
        event->Fail();
    }
    auto code = __real_curl_easy_pause(handle, action);
    // Only the dedicated error case changes a return value; all other cases
    // observe native curl unchanged. Inject after real replay to exercise
    // retirement with curl already able to generate a successful DONE.
    if (action == CURLPAUSE_CONT && code == CURLE_OK) {
        const auto injected = inject_unpause_error.exchange(0);
        if (injected) code = static_cast<CURLcode>(injected);
    }
    if (action == CURLPAUSE_CONT && code != CURLE_OK) unpause_error = code;
    return code;
}
namespace VolcengineTos {
struct AsyncTransportTestAccess {
    static double WorkerCpu(AsyncHttpClient& client) {
        if (client.engine_) return client.engine_->stats().worker_cpu_ns / 1e9;
        double result = 0;
        for (auto id : WorkerClocks(client)) {
            timespec time{}; CHECK(clock_gettime(id, &time) == 0);
            result += time.tv_sec + time.tv_nsec * 1e-9;
        }
        return result;
    }
    static std::shared_ptr<AsyncEngine> SharedEngine(AsyncHttpClient& client) {
        return client.engine_;
    }
    static std::vector<clockid_t> WorkerClocks(AsyncHttpClient& client) {
        std::vector<clockid_t> clocks;
        for (auto& worker : client.multi_handles_) {
            clockid_t id;
            CHECK(pthread_getcpuclockid(worker->thread.native_handle(), &id) == 0);
            clocks.push_back(id);
        }
        return clocks;
    }
};
}  // namespace VolcengineTos
namespace {
bool shared_mode = false;
std::unique_ptr<AsyncHttpClient> MakeClient(const TransportConfig& config) {
    if (!shared_mode) return std::make_unique<AsyncHttpClient>(config);
    AsyncEngineOptions options;
    options.worker_count = config.getEventThreadCount();
    options.max_connections = config.getMaxConnections();
    options.max_requests = options.max_requests_per_client = 4096;
    return std::make_unique<AsyncHttpClient>(config, AsyncEngine::Create(options));
}
int64_t EpochMs() {
    return std::chrono::duration_cast<Ms>(std::chrono::system_clock::now().time_since_epoch()).count();
}
double Cpu(clockid_t id) {
    timespec t{};
    CHECK(clock_gettime(id, &t) == 0);
    return t.tv_sec + t.tv_nsec * 1e-9;
}
TransportConfig Config(unsigned workers, unsigned slots, int cap = 1000) {
    TransportConfig config;
    config.setEventThreadCount(workers);
    config.setMaxConnections(slots);
    config.setMaxRequestQueue(4096);
    config.setRequestTimeout(10000);
    config.setConnectTimeout(2000);
    config.setDetailLog(false);
    if (cap) config.setCurlMultiWaitTimeoutMs(cap);
    return config;
}
struct Result {
    std::mutex mutex;
    std::condition_variable cv;
    unsigned calls = 0, chunks = 0;
    size_t bytes = 0;
    bool bad = false, success = false;
    int error = -1;
    AsyncEvent* event = nullptr;
    Clock::time_point started = Clock::now(), ended;
    void Done(const std::shared_ptr<HttpResponse>& response) {
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        error = response->getCurlErrCode();
        success = response->status() == http::Success;
        event = nullptr;
        ended = Clock::now();
        cv.notify_all();
    }
    void Wait(bool expect_success = true, Ms budget = Ms(12000)) {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, budget, [&] { return calls; }));
        CHECK(calls == 1);
        if (expect_success && (!success || error != 0 || bad)) {
            throw std::runtime_error("unexpected completion: success=" + std::to_string(success) +
                                     " curl_error=" + std::to_string(error) + " bad_data=" + std::to_string(bad) +
                                     " bytes=" + std::to_string(bytes));
        }
    }
    void Pause(AsyncEvent* value, int delay) {
        std::lock_guard<std::mutex> lock(mutex);
        if (delay < 0)
            value->Pause();
        else
            value->PauseFor(delay);
        event = value;
        cv.notify_all();
    }
    void WaitPaused() {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, Ms(4000), [&] { return event || calls; }));
        CHECK(event && !calls);
    }
    void Signal(bool fail = false) {
        // Completion clears the raw pointer under this same mutex before the
        // SDK destroys its event. Never invoke a stale event after completion.
        std::lock_guard<std::mutex> lock(mutex);
        if (event) {
            if (fail)
                event->Fail();
            else
                event->Resume();
        }
    }
};
std::shared_ptr<HttpRequest> Request(Server& server, bool put, size_t bytes, unsigned network_delay = 0,
                                     unsigned queue_delay = 0) {
    auto req = std::make_shared<HttpRequest>(put ? "PUT" : "GET");
    req->setUrl(Url(server.Endpoint() + "/bytes/" + std::to_string(bytes) + "/" + std::to_string(network_delay)));
    if (put) {
        req->setContentLength(bytes);
        req->setHeader("Expect", "");
    }
    if (queue_delay) req->setNotSendUtilMs(EpochMs() + queue_delay);
    return req;
}
std::shared_ptr<Result> Submit(AsyncHttpClient& client, Server& server, bool put, size_t size, bool verify = true,
                               int pause = 0, unsigned network_delay = 0, unsigned queue_delay = 0) {
    auto result = std::make_shared<Result>();
    auto req = Request(server, put, size, network_delay, queue_delay);
    auto receive = [result, verify, pause](char* data, size_t size, AsyncEvent* event) {
        ++result->chunks;
        const bool first_pause = result->chunks == 1 && pause;
        const auto used = first_pause ? std::min(size_t(1), size) : size;
        if (verify && std::find_if(data, data + used, [](char byte) { return byte != 'x'; }) != data + used)
            result->bad = true;
        result->bytes += used;
        if (first_pause) result->Pause(event, pause);
        return used;
    };
    auto produce = [result, size, pause](char* data, size_t available, AsyncEvent* event) {
        ++result->chunks;
        if (result->chunks == 1 && pause) {
            result->Pause(event, pause);
            return size_t(0);
        }
        const auto used = std::min(available, size - result->bytes);
        std::memset(data, 'x', used);
        result->bytes += used;
        return used;
    };
    client.sendCallback(req, put ? OnDataReceiveWithEvent{} : OnDataReceiveWithEvent(receive),
                        put ? OnDataSendWithEvent(produce) : OnDataSendWithEvent{},
                        [result](auto response) { result->Done(response); });
    return result;
}
void Load(unsigned workers, unsigned window, unsigned count, size_t size, bool put, bool mixed, bool benchmark) {
    CHECK(workers >= 1 && workers <= 16 && window >= workers && window <= 128);
    CHECK(count >= 1 && count <= 100000 && size >= 1 && size <= 100 * 1024 * 1024);
    Server server(!benchmark);
    auto config = Config(workers, std::max(workers, window), benchmark ? 0 : 1000);
    // Full TSan instruments both the SDK and the byte-validating HTTP server.
    // 100 MiB uploads can exceed 10 s there. This is a payload/lifetime test,
    // not a deadline test; dedicated 100/250 ms timeout gates stay unchanged.
    const bool large_payload = size >= 100 * 1024 * 1024;
    if (large_payload) config.setRequestTimeout(60000);
    auto client_owner = MakeClient(config);
    auto& client = *client_owner;
    // Warm connections and curl caches, outside measurement.
    for (unsigned i = 0; i < window; ++i) Submit(client, server, put, 4096)->Wait();
    auto workers_cpu = [&] {
        return AsyncTransportTestAccess::WorkerCpu(client);
    };
    const auto begin = Clock::now();
    const double wc = workers_cpu(), pc = Cpu(CLOCK_PROCESS_CPUTIME_ID);
    std::vector<double> latency;
    latency.reserve(count);
    for (unsigned i = 0; i < count;) {
        std::vector<std::shared_ptr<Result>> batch;
        for (unsigned j = 0; j < window && i < count; ++j, ++i) {
            const bool upload = mixed ? i % 2 : put;
            batch.push_back(Submit(client, server, upload, size, !benchmark, mixed && i % 7 == 0 ? 2 : 0,
                                   mixed && i % 11 == 0 ? 3 : 0, mixed && i % 5 == 0 ? 2 : 0));
        }
        for (auto& result : batch) {
            result->Wait(true, large_payload ? Ms(70000) : Ms(12000));
            CHECK(result->bytes == size);
            latency.push_back(std::chrono::duration<double, std::milli>(result->ended - result->started).count());
        }
    }
    const auto wall = std::chrono::duration<double>(Clock::now() - begin).count();
    const double worker_cpu = workers_cpu() - wc, process_cpu = Cpu(CLOCK_PROCESS_CPUTIME_ID) - pc;
    std::ifstream status("/proc/self/status");
    std::string line;
    long rss = 0, hwm = 0;
    while (std::getline(status, line)) {
        if (line.compare(0, 6, "VmRSS:") == 0) rss = std::stol(line.substr(6));
        if (line.compare(0, 6, "VmHWM:") == 0) hwm = std::stol(line.substr(6));
    }
    std::sort(latency.begin(), latency.end());
    std::cout << std::fixed << std::setprecision(4) << "load workers=" << workers << " window=" << window
              << " count=" << count << " bytes=" << size << " put=" << put << " mixed=" << mixed << " wall_s=" << wall
              << " worker_cpu_ms_per_op=" << worker_cpu * 1000 / count << " worker_cores=" << worker_cpu / wall
              << " process_cpu_ms_per_op=" << process_cpu * 1000 / count << " p50_ms=" << latency[count / 2]
              << " p99_ms=" << latency[(count - 1) * 99 / 100] << " rss_kib=" << rss << " hwm_kib=" << hwm << '\n';
    client.closeClient();
    server.Check();
    CHECK(server.Requests() == count + window);
}
struct OnceLimiter : RateLimiter {
    std::atomic<unsigned> calls{0};
    std::pair<bool, time_t> Acquire(int64_t) override {
        return {calls++ != 0, 80};
    }
};
void UploadAndQos() {
    Server server;
    auto client_owner = MakeClient(Config(1, 4, 5000));
    auto& client = *client_owner;
    for (bool put : {false, true}) {
        auto paused = Submit(client, server, put, 256 * 1024, true, -1);
        paused->WaitPaused();
        auto quick = Submit(client, server, !put, 4096);
        quick->Wait();
        CHECK(quick->ended - quick->started < Ms(500));
        paused->Signal();
        paused->Wait();
        CHECK(paused->bytes == 256 * 1024);

        auto result = std::make_shared<Result>();
        auto limiter = std::make_shared<OnceLimiter>();
        auto req = Request(server, put, 4096);
        req->setRateLimiter(limiter);
        client.sendCallback(
                req, put ? OnDataReceiveWithEvent{} : OnDataReceiveWithEvent([result](char*, size_t n, AsyncEvent*) {
                    result->bytes += n;
                    return n;
                }),
                put ? OnDataSendWithEvent([result](char* data, size_t n, AsyncEvent*) {
                    n = std::min(n, 4096 - result->bytes);
                    std::memset(data, 'x', n);
                    result->bytes += n;
                    return n;
                })
                    : OnDataSendWithEvent{},
                [result](auto response) { result->Done(response); });
        result->Wait();
        CHECK(result->bytes == 4096 && limiter->calls > 0);
        CHECK(result->ended - result->started >= Ms(60) && result->ended - result->started < Ms(500));
    }
    client.closeClient();
    server.Check();
}
void CloseRace() {
    Server server;
    for (unsigned round = 0; round < 64; ++round) {
        auto client_owner = MakeClient(Config(4, 16, 5000));
        auto& client = *client_owner;
        auto paused = Submit(client, server, round % 2, 65536, true, -1);
        paused->WaitPaused();
        std::vector<std::shared_ptr<Result>> submitted;
        std::atomic<bool> start{false};
        std::thread producer([&] {
            while (!start.load()) std::this_thread::yield();
            for (unsigned i = 0; i < 128; ++i)
                submitted.push_back(Submit(client, server, i % 2, 4096, true, 0, 0, i % 2 ? 50 : 0));
        });
        std::thread resumer([&] {
            while (!start.load()) std::this_thread::yield();
            for (unsigned i = 0; i < 64; ++i) paused->Signal(i % 3 == 0);
        });
        start = true;
        const auto begin = Clock::now();
        client.closeClient();
        producer.join();
        resumer.join();
        CHECK(Clock::now() - begin < Ms(1000));
        paused->Wait(false);
        for (auto& result : submitted) result->Wait(false);
    }
    server.Check();
}
void ReentrantCompletion() {
    Server server;
    auto client_owner = MakeClient(Config(1, 1, 5000));
    auto& client = *client_owner;
    std::atomic<unsigned> count{0};
    std::atomic<bool> bad{false};
    std::mutex mutex;
    std::condition_variable cv;
    std::function<void()> submit;
    submit = [&] {
        client.sendCallback(Request(server, false, 4096), {}, {}, [&](auto response) {
            if (response->getCurlErrCode()) bad = true;
            if (++count < 1000)
                submit();
            else {
                std::lock_guard<std::mutex> lock(mutex);
                cv.notify_all();
            }
        });
    };
    submit();
    std::unique_lock<std::mutex> lock(mutex);
    const bool done = cv.wait_for(lock, Ms(10000), [&] { return count == 1000; });
    lock.unlock();
    client.closeClient();
    CHECK(done && !bad && count == 1000);
    server.Check();
}
void CachedFailure(const std::string& fault) {
    Server server;
    // A broken callback can produce millions of identical ERROR lines. Keep
    // exact failure/callback/CPU counters, not unbounded diagnostic output.
    Logger::getInstance().setMaxQueueSize(0);
    auto config = Config(1, 1);
    config.setRequestTimeout(3000);
    auto client_owner = MakeClient(config);
    auto& client = *client_owner;
    const auto cpu_start = AsyncTransportTestAccess::WorkerCpu(client);
    auto result = std::make_shared<Result>();
    if (fault == "unpause") inject_unpause_error = CURLE_RECV_ERROR;
    client.sendCallback(Request(server, false, 4096),
                        [result, fault](char*, size_t size, AsyncEvent* event) {
                            if (++result->chunks == 1) {
                                event->PauseFor(20);
                                result->bytes = 1;
                                return size_t(1);
                            }
                            if (fault == "throw") throw std::runtime_error("cached callback fault");
                            if (fault == "throw_int") throw 7;
                            if (fault == "fail" || fault == "fail_full") {
                                event->Fail();
                                return fault == "fail" ? size_t(0) : size;
                            }
                            if (fault == "overconsume") return size + 1;
                            if (fault == "unpause" || fault == "skip_fail") {
                                if (fault == "skip_fail") fail_before_unpause = event;
                                result->bytes += size;
                                return size;
                            }
                            return size_t(0);
                        },
                        {}, [result](auto response) { result->Done(response); });
    std::unique_lock<std::mutex> lock(result->mutex);
    const bool completed = result->cv.wait_for(lock, Ms(700), [&] { return result->calls; });
    lock.unlock();
    const auto cpu_ms = (AsyncTransportTestAccess::WorkerCpu(client) - cpu_start) * 1000;
    // Same single worker and single slot must remain usable after retirement.
    auto healthy = Submit(client, server, false, 65536);
    healthy->Wait();
    client.closeClient();
    server.Check();
    std::cout << "cached fault=" << fault << " completed=" << completed << " calls=" << result->calls
              << " success=" << result->success << " error=" << result->error << " bytes=" << result->bytes
              << " chunks=" << result->chunks << " unpause_error=" << unpause_error << " worker_cpu_ms=" << cpu_ms
              << '\n';
    CHECK(completed && !result->success && result->chunks == 2 && result->calls == 1);
    CHECK(result->ended - result->started < Ms(500) && cpu_ms < 100);
    CHECK(healthy->bytes == 65536 && healthy->calls == 1);
    if (fault == "unpause") {
        CHECK(result->error == CURLE_RECV_ERROR && result->bytes == 4096 && unpause_error == CURLE_RECV_ERROR);
    } else if (fault == "skip_fail") {
        CHECK(result->error == CURLE_WRITE_ERROR && result->bytes == 4096 && unpause_error == CURLE_WRITE_ERROR);
    } else {
        CHECK(result->error == CURLE_WRITE_ERROR && result->bytes == 1);
    }
    CHECK(inject_unpause_error == 0 && fail_before_unpause == nullptr);
}
void CachedRearm(unsigned early_resume = 0) {
    Server server;
    auto client_owner = MakeClient(Config(1, 1, 5000));
    auto& client = *client_owner;
    auto result = std::make_shared<Result>();
    Clock::time_point rearmed;
    client.sendCallback(Request(server, false, 4096), [&](char*, size_t size, AsyncEvent* event) {
        ++result->chunks;
        if (result->chunks == 1) {
            event->PauseFor(20);
            result->bytes = 1;
            return size_t(1);
        }
        if (result->chunks == 2) {
            rearmed = Clock::now();
            if (early_resume) {
                event->Pause();
                // Downstream may finish before this callback returns.
                if (early_resume == 2) {
                    std::thread notifier([event] { event->Resume(); });
                    notifier.join();
                } else event->Resume();
            } else event->PauseFor(60);
            return size_t(0);  // Valid zero progress with freshly armed backpressure.
        }
        if (!early_resume && Clock::now() - rearmed < Ms(50)) result->bad = true;
        result->bytes += size;
        return size;
    }, {}, [result](auto response) { result->Done(response); });
    result->Wait();
    client.closeClient();
    CHECK(result->bytes == 4096 && result->chunks == 3 && result->calls == 1);
    server.Check();
}
void ErrorReentrant() {
    Server server;
    auto config = Config(1, 1, 5000);
    config.setMaxRequestQueue(1);
    auto client_owner = MakeClient(config);
    auto& client = *client_owner;
    std::atomic<unsigned> failures{0}, successes{0};
    std::atomic<bool> bad{false};
    std::mutex mutex;
    std::condition_variable cv;
    std::function<void()> submit;
    submit = [&] {
        auto chunks = std::make_shared<unsigned>(0);
        client.sendCallback(Request(server, false, 4096), [chunks](char*, size_t, AsyncEvent* event) {
            if (++*chunks == 1) {
                event->PauseFor(1);
                return size_t(1);
            }
            event->Fail();
            return size_t(0);
        }, {}, [&, chunks](auto response) {
            if (response->getCurlErrCode() != CURLE_WRITE_ERROR || response->status() == http::Success ||
                *chunks != 2) bad = true;
            ++failures;
            // Reenter before the failed context's destructor, with admission
            // limited to one: retirement must release its slot first.
            client.sendCallback(Request(server, false, 4096), {}, {}, [&](auto healthy) {
                if (healthy->getCurlErrCode() != CURLE_OK || healthy->status() != http::Success) bad = true;
                if (++successes < 64) submit();
                else {
                    std::lock_guard<std::mutex> lock(mutex);
                    cv.notify_all();
                }
            });
        });
    };
    submit();
    std::unique_lock<std::mutex> lock(mutex);
    const bool done = cv.wait_for(lock, Ms(10000), [&] { return successes == 64; });
    lock.unlock();
    client.closeClient();
    CHECK(done && !bad && failures == 64 && successes == 64);
    server.Check();
}
void TimeoutMixed() {
    Server server;
    // Reserve capacity for all 16 paused plus 16 healthy requests. Shared
    // limits are engine-wide and its deadline includes admission queue time;
    // queued timeout behavior is covered separately by AsyncShared_deadline.
    auto config = Config(4, 32, 5000);
    config.setRequestTimeout(100);
    auto client_owner = MakeClient(config);
    auto& client = *client_owner;
    std::unique_ptr<AsyncHttpClient> ready_owner;
    if (shared_mode) {
        // Test short deadlines beside healthy work on the SAME engine. A cold
        // 32-connection start under full TSan can itself exceed 100 ms; that is
        // a valid deadline expiration, not a failure to schedule healthy IO.
        // Keep held requests at 100 ms and the <800 ms progress assertions.
        // The healthy session has its own attempt budget, as real clients do.
        auto healthy_config = config;
        healthy_config.setRequestTimeout(2000);
        ready_owner = std::make_unique<AsyncHttpClient>(healthy_config,
                AsyncTransportTestAccess::SharedEngine(client));
    }
    auto& ready_client = ready_owner ? *ready_owner : client;
    std::vector<std::shared_ptr<Result>> held, ready;
    for (unsigned i = 0; i < 16; ++i) {
        held.push_back(Submit(client, server, i % 2, 65536, true, -1));
        ready.push_back(Submit(ready_client, server, i % 2, 4096));
    }
    int64_t held_max_ms = 0, ready_max_ms = 0;
    for (auto& r : held) {
        r->Wait(false);
        CHECK(!r->success && r->error == CURLE_OPERATION_TIMEDOUT);
        CHECK(r->ended - r->started < Ms(800));
        held_max_ms = std::max(held_max_ms, std::chrono::duration_cast<Ms>(r->ended - r->started).count());
    }
    for (auto& r : ready) {
        r->Wait();
        CHECK(r->ended - r->started < Ms(800));
        ready_max_ms = std::max(ready_max_ms, std::chrono::duration_cast<Ms>(r->ended - r->started).count());
    }
    std::cout << "timeout_mixed shared=" << shared_mode << " paused_max_ms=" << held_max_ms
              << " healthy_max_ms=" << ready_max_ms << '\n';
    client.closeClient();
    server.Check();
}
void LiveEarlyResume(bool put) {
    Server server;
    auto config = Config(1, 1, 5000);
    config.setRequestTimeout(1000);
    auto client_owner = MakeClient(config);
    auto& client = *client_owner;
    auto result = std::make_shared<Result>();
    auto transfer = [result, put](char* data, size_t size, AsyncEvent* event) {
        if (++result->chunks == 1) {
            event->Pause();
            std::thread notifier([event] { event->Resume(); });
            notifier.join();
            return size_t(0);  // A completed downstream operation, not EOF/error.
        }
        if (put) {
            size = std::min(size, 4096 - result->bytes);
            std::memset(data, 'x', size);
        } else if (std::find_if(data, data + size, [](char x) { return x != 'x'; }) != data + size) {
            result->bad = true;
        }
        result->bytes += size;
        return size;
    };
    client.sendCallback(Request(server, put, 4096), put ? OnDataReceiveWithEvent{} : transfer,
                        put ? transfer : OnDataSendWithEvent{}, [result](auto response) { result->Done(response); });
    result->Wait();
    auto healthy = Submit(client, server, !put, 4096);
    healthy->Wait();
    client.closeClient();
    // TCP may split the GET body into several data callbacks. Only terminal
    // delivery is exactly once; after the initial pause all bytes must arrive.
    CHECK(result->bytes == 4096 && result->chunks >= 2 && result->calls == 1);
    server.Check();
}
void PatternedRead(const std::string& framing) {
    using Script = tos_test::LoopbackHttpFixture;
    std::string payload(65536, '\0');
    for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<char>((i * 131 + i / 257) % 251);
    Script::Response reply;
    reply.body = payload;
    if (framing == "chunked") reply.framing = Script::BodyFraming::Chunked;
    if (framing == "close") reply.framing = Script::BodyFraming::CloseDelimited;
    if (framing == "short") reply.declared_content_length = payload.size() + 8192;
    Script server({{"GET", "/pattern", reply}});
    auto client_owner = MakeClient(Config(1, 1, 5000));
    auto& client = *client_owner;
    auto result = std::make_shared<Result>();
    uint64_t crc = 0;
    auto req = std::make_shared<HttpRequest>();
    req->setCheckCrc64(true);
    req->setUrl(Url(server.Endpoint() + "/pattern"));
    client.sendCallback(req, [result, &payload](char* data, size_t size, AsyncEvent* event) {
        ++result->chunks;
        const size_t used = std::min(size, size_t(1021));
        if (result->bytes + used > payload.size() ||
            std::memcmp(data, payload.data() + result->bytes, used) != 0) result->bad = true;
        result->bytes += used;
        if (result->chunks % 5 == 1) event->PauseFor(1);
        return used;
    }, {}, [result, &crc](auto response) {
        crc = response->getHashCrc64Result();
        result->Done(response);
    });
    result->Wait(framing != "short");
    client.closeClient();
    server.AssertDone();
    CHECK(result->bytes == payload.size() && !result->bad && result->calls == 1 && result->chunks > 64);
    if (framing == "short") CHECK(!result->success && result->error == CURLE_PARTIAL_FILE);
    else CHECK(crc == CRC64::CalcCRC(0, payload.data(), payload.size()));
}
void FailureTimeoutRace() {
    Server server;
    for (unsigned workers : {1u, 4u}) {
        auto config = Config(workers, 64, 5000);
        config.setRequestTimeout(120);
        auto client_owner = MakeClient(config);
        auto& client = *client_owner;
        std::vector<std::shared_ptr<Result>> held, ready;
        for (unsigned i = 0; i < 32; ++i) {
            held.push_back(Submit(client, server, i % 2, 65536, true, -1));
            ready.push_back(Submit(client, server, i % 2, 4096));
        }
        for (auto& result : held) result->WaitPaused();
        std::vector<std::thread> notifiers;
        for (unsigned i = 0; i < held.size(); ++i) {
            notifiers.emplace_back([result = held[i], i] {
                std::this_thread::sleep_for(Ms(60 + (i * 17) % 110));
                // Signal checks completion under the same lock that clears
                // the borrowed event. Late signals must not dereference it.
                result->Signal(true);
                result->Signal();
            });
        }
        for (auto& notifier : notifiers) notifier.join();
        for (auto& result : held) {
            result->Wait(false);
            CHECK(!result->success && (result->error == CURLE_OPERATION_TIMEDOUT ||
                  result->error == CURLE_WRITE_ERROR || result->error == CURLE_ABORTED_BY_CALLBACK));
            CHECK(result->ended - result->started < Ms(800));
        }
        for (auto& result : ready) result->Wait();
        client.closeClient();
        for (auto& result : held) CHECK(result->calls == 1);
    }
    server.Check();
}
unsigned OpenDescriptors() {
    DIR* directory = opendir("/proc/self/fd");
    CHECK(directory);
    unsigned count = 0;
    while (readdir(directory)) ++count;
    closedir(directory);
    return count;
}
unsigned ThreadCount() {
    std::ifstream input("/proc/self/status");
    std::string line;
    while (std::getline(input, line))
        if (line.compare(0, 8, "Threads:") == 0) return std::stoul(line.substr(8));
    throw std::runtime_error("missing process thread count");
}
void CheckThreadsRetired(unsigned expected) {
    // pthread_join observes clear_child_tid, before Linux necessarily removes
    // the task from /proc's thread count. Bound that observation lag; a live
    // leaked worker must still fail. Do not delay fd/context leak checks.
    const auto deadline = Clock::now() + Ms(100);
    unsigned actual;
    while ((actual = ThreadCount()) != expected) {
        if (Clock::now() >= deadline)
            throw std::runtime_error("threads did not retire: " + std::to_string(actual) +
                                     "/" + std::to_string(expected));
        std::this_thread::sleep_for(Ms(1));
    }
}
void ResourceCycles() {
    // Logger is already running. Take this baseline before creating temporary
    // workers, not immediately after joining a warm-up client's threads.
    const auto threads = ThreadCount();
    auto cycle = [] {
        Server server;
        auto client_owner = MakeClient(Config(4, 16, 5000));
        auto& client = *client_owner;
        std::vector<std::shared_ptr<Result>> results;
        for (unsigned i = 0; i < 64; ++i)
            results.push_back(Submit(client, server, i % 2, 4096, true, i % 3 == 0 ? 1 : 0));
        for (auto& result : results) result->Wait();
        client.closeClient();
        server.Check();
    };
    cycle();  // Initialize lazy SDK/logger state before establishing the baseline.
    CheckThreadsRetired(threads);
    const auto descriptors = OpenDescriptors();
    for (unsigned i = 0; i < 32; ++i) {
        cycle();
        const auto actual_fds = OpenDescriptors();
        if (actual_fds != descriptors) {
            throw std::runtime_error("resource cycle=" + std::to_string(i) + " fds=" +
                std::to_string(actual_fds) + "/" + std::to_string(descriptors));
        }
        CHECK(g_req_ctx_create_cnt == g_req_ctx_destroy_cnt);
        CheckThreadsRetired(threads);
    }
}
}  // namespace
int main(int argc, char** argv) {
    try {
        const rlimit no_core{0, 0};
        CHECK(setrlimit(RLIMIT_CORE, &no_core) == 0);
        Logger::getInstance().setAsyncLogLevel(ERROR);
        CHECK(argc >= 2);
        std::string mode = argv[1];
        if (mode.compare(0, 7, "shared_") == 0) {
            shared_mode = true; mode = mode.substr(7);
        }
        if (mode == "load") {
            CHECK(argc == 7);
            Load(std::stoul(argv[2]), std::stoul(argv[3]), std::stoul(argv[4]), std::stoull(argv[5]),
                 std::stoul(argv[6]), false, true);
        } else if (mode == "mixed") {
            for (unsigned workers : {1u, 4u})
                for (unsigned window : {4u, 32u, 128u}) Load(workers, window, 1024, 65536, false, true, false);
        } else if (mode == "large") {
            for (bool put : {false, true}) Load(4, 4, 8, 100 * 1024 * 1024, put, false, false);
        } else if (mode == "mixed_wide") {
            Load(8, 128, 8192, 65536, false, true, false);
        } else if (mode == "mixed_info") {
            Logger::getInstance().setAsyncLogLevel(INFO);
            Logger::getInstance().setMaxQueueSize(0);  // Execute formatting, bound output.
            Load(4, 32, 1024, 65536, false, true, false);
        } else if (mode == "upload_qos")
            UploadAndQos();
        else if (mode == "close_race")
            CloseRace();
        else if (mode == "reentrant")
            ReentrantCompletion();
        else if (mode == "timeout_mixed")
            TimeoutMixed();
        else if (mode == "cache_rearm")
            CachedRearm();
        else if (mode == "cache_resume_rearm")
            CachedRearm(1);
        else if (mode == "cache_resume_thread")
            CachedRearm(2);
        else if (mode == "error_reentrant")
            ErrorReentrant();
        else if (mode == "live_resume_get" || mode == "live_resume_put")
            LiveEarlyResume(mode == "live_resume_put");
        else if (mode == "failure_timeout_race")
            FailureTimeoutRace();
        else if (mode == "resource_cycles")
            ResourceCycles();
        else if (mode.compare(0, 8, "pattern_") == 0)
            PatternedRead(mode.substr(8));
        else if (mode.compare(0, 6, "cache_") == 0)
            CachedFailure(mode.substr(6));
        else
            throw std::runtime_error("unknown stress mode");
        CHECK(g_req_ctx_create_cnt == g_req_ctx_destroy_cnt);
        std::cout << "PASS " << mode << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
