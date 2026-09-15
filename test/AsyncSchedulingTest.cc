#include "LoopbackHttpFixture.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "logger/logger.h"
#include <pthread.h>
#include <sys/eventfd.h>
#include <cerrno>
#include <time.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace VolcengineTos;
using Server = tos_test::LoopbackHttpFixture;
using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;
#define CHECK(x)                                                                                       \
    do {                                                                                               \
        if (!(x)) throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " #x); \
    } while (false)

// Linker-only fault injection: fail the next worker's eventfd creation, without
// exhausting process-wide descriptors or changing production SDK behavior.
std::atomic<bool> fail_eventfd{false};
extern "C" int __real_eventfd(unsigned int, int);
extern "C" int __wrap_eventfd(unsigned int init, int flags) {
    if (fail_eventfd.exchange(false)) {
        errno = EMFILE;
        return -1;
    }
    return __real_eventfd(init, flags);
}

namespace VolcengineTos {
struct AsyncTransportTestAccess {
    static clockid_t WorkerClock(AsyncHttpClient& client) {
        clockid_t result;
        CHECK(client.multi_handles_.size() == 1);
        CHECK(pthread_getcpuclockid(client.multi_handles_[0]->thread.native_handle(), &result) == 0);
        return result;
    }
#ifndef TOS_SCHEDULING_BASELINE
    static void WaitPlan() {
        AsyncHttpClient::initCurl();
        {
            TransportConfig config;
            AsyncHttpClient::CurlMultiHandle handle(config, 1);
            handle.running_finish = true;  // Synthetic scheduler state, no worker.
            auto* late = new AsyncHttpClient::RequestContext(std::make_shared<HttpRequest>());
            late->not_send_util_ = 900;
            handle.request_queue_.push(late);
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == 900);
            auto* early = new AsyncHttpClient::RequestContext(std::make_shared<HttpRequest>());
            early->not_send_util_ = 500;
            handle.request_queue_.push(early);
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == 500);
            handle.active_count = 1;
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == -1);  // No connection slot.
            auto* paused = new AsyncHttpClient::RequestContext(std::make_shared<HttpRequest>());
            paused->event_ = std::make_shared<AsyncEvent>();
            paused->event_->markWaiting();
            paused->event_->setResumeAt(700);
            handle.active_set_.insert(paused);
            handle.paused_queue_.push(paused);
            paused->in_paused_queue = true;
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == 700);
            paused->event_->Pause();
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == -1);
            paused->event_->Resume();
            CHECK(paused->event_->getResumeAt() == 0 && AsyncHttpClient::getNextIdleWakeupMs(handle) == 0);
            paused->event_->PauseFor(3000);
            const auto decision = AsyncHttpClient::buildPauseDecision(paused, 0, true);
            paused->event_->Resume();
            AsyncHttpClient::applyPauseState(paused, decision.resume_at_ms);
            CHECK(paused->event_->getResumeAt() == 0 && AsyncHttpClient::getNextIdleWakeupMs(handle) == 0);
            // An explicit Resume clears user backpressure, not SDK QoS debt.
            AsyncHttpClient::applyPauseState(paused, 700);
            paused->event_->Resume();
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == 700);
            paused->event_->Fail();
            CHECK(AsyncHttpClient::getNextIdleWakeupMs(handle) == 0);
        }
        AsyncHttpClient::cleanupCurl();
    }
#endif
};
}  // namespace VolcengineTos
namespace {
bool benchmark = false;
int64_t EpochMs() {
    return std::chrono::duration_cast<Ms>(std::chrono::system_clock::now().time_since_epoch()).count();
}
double Cpu(clockid_t clock) {
    timespec value{};
    CHECK(clock_gettime(clock, &value) == 0);
    return value.tv_sec + value.tv_nsec * 1e-9;
}
struct Sample {
    clockid_t worker;
    Clock::time_point start{Clock::now()};
    double worker_start, process_start;
    explicit Sample(AsyncHttpClient& client)
        : worker(AsyncTransportTestAccess::WorkerClock(client)),
          worker_start(Cpu(worker)),
          process_start(Cpu(CLOCK_PROCESS_CPUTIME_ID)) {
    }
    double Report(const char* name) {
        const double wall = std::chrono::duration<double>(Clock::now() - start).count();
        const double worker_cpu = Cpu(worker) - worker_start;
        const double process_cpu = Cpu(CLOCK_PROCESS_CPUTIME_ID) - process_start;
        // /proc VmHWM belongs to this address space; ru_maxrss can include a
        // larger pre-exec launcher image and distort tiny SDK measurements.
        std::ifstream status("/proc/self/status");
        std::string line;
        long rss = 0, hwm = 0;
        while (std::getline(status, line)) {
            if (line.compare(0, 6, "VmRSS:") == 0) rss = std::stol(line.substr(6));
            if (line.compare(0, 6, "VmHWM:") == 0) hwm = std::stol(line.substr(6));
        }
        std::cout << std::fixed << std::setprecision(4) << name << " wall_ms=" << wall * 1000
                  << " worker_cpu_ms=" << worker_cpu * 1000 << " worker_cores=" << worker_cpu / wall
                  << " process_cpu_ms=" << process_cpu * 1000 << " rss_kib=" << rss << " hwm_kib=" << hwm << '\n';
        return worker_cpu / wall;
    }
};
TransportConfig Config(unsigned connections = 1) {
    TransportConfig config;
    config.setEventThreadCount(1);
    config.setMaxConnections(connections);
    config.setMaxRequestQueue(128);
    config.setConnectTimeout(2000);
    config.setRequestTimeout(4000);
    return config;
}
std::shared_ptr<HttpRequest> Request(const std::string& url, int64_t not_before = 0) {
    auto result = std::make_shared<HttpRequest>();
    result->setUrl(Url(url));
    result->setNotSendUtilMs(not_before);
    return result;
}
struct Result {
    std::mutex mutex;
    std::condition_variable cv;
    unsigned calls{0};
    int error{-1};
    std::string bytes;
    AsyncEvent* event{nullptr};
    Clock::time_point ended;
    void Done(std::shared_ptr<HttpResponse> response) {
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        error = response->getCurlErrCode();
        ended = Clock::now();
        cv.notify_all();
    }
    void Wait(int expected_error = 0) {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, Ms(5000), [&] { return calls != 0; }));
        CHECK(calls == 1 && error == expected_error);
    }
    void WaitPaused() {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, Ms(5000), [&] { return event != nullptr; }));
    }
};
void Idle() {
    AsyncHttpClient client(Config());
    Sample sample(client);
    std::this_thread::sleep_for(Ms(500));
    const auto cores = sample.Report("idle");
    client.closeClient();
    if (!benchmark) CHECK(cores < 0.15);
}
void Delayed() {
    Server::Response reply;
    reply.body = "delayed bytes";
    Server server({{"GET", "/delayed", reply}});
    AsyncHttpClient client(Config());
    Result result;
    const auto target = EpochMs() + 900;
    client.sendCallback(Request(server.Endpoint() + "/delayed", target),
                        [&](char* bytes, size_t size, AsyncEvent*) {
                            result.bytes.append(bytes, size);
                            return size;
                        },
                        {}, [&](auto response) { result.Done(response); });
    Sample sample(client);
    std::this_thread::sleep_for(Ms(500));
    const auto cores = sample.Report("delayed_queue");
    CHECK(server.Requests().empty());
    result.Wait();
    CHECK(EpochMs() >= target && result.bytes == reply.body);
    client.closeClient();
    CHECK(result.calls == 1);
    server.AssertDone();
    if (!benchmark) CHECK(cores < 0.15);
}
void Paused(bool timed, bool queued) {
    Server::Response reply;
    reply.body = "paused bytes must appear exactly once";
    std::vector<Server::Step> script{{"GET", "/paused", reply}};
    if (queued) script.push_back({"GET", "/queued", {}});
    Server server(std::move(script));
    AsyncHttpClient client(Config());
    Result result, other;
    bool first = true;
    const auto begun = Clock::now();
    client.sendCallback(Request(server.Endpoint() + "/paused"),
                        [&](char* bytes, size_t size, AsyncEvent* event) {
                            if (first) {
                                first = false;
                                result.bytes.append(bytes, 1);
                                if (timed)
                                    event->PauseFor(900);
                                else
                                    event->Pause();
                                std::lock_guard<std::mutex> lock(result.mutex);
                                result.event = event;
                                result.cv.notify_all();
                                return size_t(1);  // Retain a genuine SDK backlog.
                            }
                            result.bytes.append(bytes, size);
                            return size;
                        },
                        {}, [&](auto response) { result.Done(response); });
    result.WaitPaused();
    if (queued)
        client.sendCallback(Request(server.Endpoint() + "/queued"), {}, {},
                            [&](auto response) { other.Done(response); });
    Sample sample(client);
    std::this_thread::sleep_for(Ms(500));
    const auto cores = sample.Report(timed ? "pause_for_backlog" : queued ? "paused_full_plus_queue" : "paused_only");
    {
        std::lock_guard<std::mutex> lock(result.mutex);
        CHECK(result.calls == 0);
    }
    const auto resumed = Clock::now();
    if (!timed) result.event->Resume();
    result.Wait();
    if (queued) other.Wait();
    CHECK(result.bytes == reply.body);
    if (timed)
        CHECK(result.ended - begun >= Ms(850));
    else
        CHECK(result.ended - resumed < Ms(500));
    client.closeClient();
    CHECK(result.calls == 1 && (!queued || other.calls == 1));
    server.AssertDone();
    if (!benchmark) CHECK(cores < 0.15);
}
void NetworkWait() {
    auto gate = std::make_shared<Server::Gate>();
    Server::Response reply;
    reply.gate = gate;
    reply.body = "network bytes";
    Server server({{"GET", "/held", reply}});
    AsyncHttpClient client(Config());
    Result result;
    client.sendCallback(Request(server.Endpoint() + "/held"),
                        [&](char* bytes, size_t size, AsyncEvent*) {
                            result.bytes.append(bytes, size);
                            return size;
                        },
                        {}, [&](auto response) { result.Done(response); });
    CHECK(gate->WaitUntilReached());
    Sample sample(client);
    std::this_thread::sleep_for(Ms(500));
    const auto cores = sample.Report("network_wait");
    gate->Release();
    result.Wait();
    CHECK(result.bytes == reply.body);
    client.closeClient();
    server.AssertDone();
    if (!benchmark) CHECK(cores < 0.15);
}
void SmallIo(bool sparse) {
    const unsigned count = sparse ? 16 : 64;
    Server::Response reply;
    reply.body.assign(4096, 'x');
    Server server(std::vector<Server::Step>(count, {"GET", "/small", reply}));
    AsyncHttpClient client(Config());
    Sample sample(client);
    std::vector<double> latency;
    for (unsigned i = 0; i < count; ++i) {
        if (sparse) std::this_thread::sleep_for(Ms(30));
        Result result;
        const auto start = Clock::now();
        client.sendCallback(Request(server.Endpoint() + "/small"),
                            [&](char* bytes, size_t size, AsyncEvent*) {
                                result.bytes.append(bytes, size);
                                return size;
                            },
                            {}, [&](auto response) { result.Done(response); });
        result.Wait();
        CHECK(result.bytes == reply.body);
        latency.push_back(std::chrono::duration<double, std::milli>(result.ended - start).count());
    }
    sample.Report(sparse ? "sparse_4k" : "serial_4k");
    std::sort(latency.begin(), latency.end());
    std::cout << "latency count=" << count << " p50_ms=" << latency[count / 2]
              << " p95_ms=" << latency[(count - 1) * 95 / 100] << '\n';
    client.closeClient();
    server.AssertDone();
}
void EarlierQueuedDeadline() {
    Server server({{"GET", "/early", {}}, {"GET", "/late", {}}});
    AsyncHttpClient client(Config());
    Result early, late;
    client.sendCallback(Request(server.Endpoint() + "/late", EpochMs() + 800), {}, {},
                        [&](auto response) { late.Done(response); });
    std::this_thread::sleep_for(Ms(20));
    auto start = Clock::now();
    client.sendCallback(Request(server.Endpoint() + "/early", EpochMs() + 50), {}, {},
                        [&](auto response) { early.Done(response); });
    early.Wait();
    CHECK(early.ended - start >= Ms(40) && early.ended - start < Ms(400));
    late.Wait();
    client.closeClient();
    server.AssertDone();
}
void ResumeOverridesTimerAndPausedTimeout(bool timeout) {
    Server::Response reply;
    reply.body = "must not replay";
    Server server({{"GET", "/paused", reply}});
    auto config = Config();
    if (timeout) config.setRequestTimeout(250);
    AsyncHttpClient client(config);
    Result result;
    bool first = true;
    client.sendCallback(Request(server.Endpoint() + "/paused"),
                        [&](char* bytes, size_t size, AsyncEvent* event) {
                            if (first) {
                                first = false;
                                result.bytes.append(bytes, 1);
                                if (timeout)
                                    event->Pause();
                                else
                                    event->PauseFor(3000);
                                std::lock_guard<std::mutex> lock(result.mutex);
                                result.event = event;
                                result.cv.notify_all();
                                return size_t(1);
                            }
                            result.bytes.append(bytes, size);
                            return size;
                        },
                        {}, [&](auto response) { result.Done(response); });
    result.WaitPaused();
    auto start = Clock::now();
    if (!timeout) result.event->Resume();
    result.Wait(timeout ? CURLE_OPERATION_TIMEDOUT : CURLE_OK);
    std::cout << (timeout ? "paused_timeout" : "early_resume")
              << " latency_ms=" << std::chrono::duration<double, std::milli>(result.ended - start).count() << '\n';
    CHECK(result.ended - start < Ms(600));
    CHECK(result.bytes == (timeout ? reply.body.substr(0, 1) : reply.body));
    client.closeClient();
    CHECK(result.calls == 1);
    server.AssertDone();
}
void CompletionDoesNotWaitForOtherSocket() {
    auto gate = std::make_shared<Server::Gate>();
    Server::Response reply;
    reply.gate = gate;
    Server server({{"GET", "/ready", {}}, {"GET", "/held", reply}});
    AsyncHttpClient client(Config(2));
    Result ready, held;
    const auto start = Clock::now();
    client.sendCallback(Request(server.Endpoint() + "/ready"), {}, {}, [&](auto response) { ready.Done(response); });
    client.sendCallback(Request(server.Endpoint() + "/held"), {}, {}, [&](auto response) { held.Done(response); });
    CHECK(gate->WaitUntilReached());
    ready.Wait();
    CHECK(ready.ended - start < Ms(500));
    gate->Release();
    held.Wait();
    client.closeClient();
    server.AssertDone();
}
void CachedCallbackCanPauseAgain() {
    Server::Response reply;
    reply.body = "a second timer must survive even after the backlog is empty";
    Server server({{"GET", "/twice", reply}});
    AsyncHttpClient client(Config());
    Result result;
    unsigned chunks = 0;
    const auto start = Clock::now();
    client.sendCallback(Request(server.Endpoint() + "/twice"),
                        [&](char* bytes, size_t size, AsyncEvent* event) {
                            ++chunks;
                            const auto used = chunks == 1 ? size_t(1) : size;
                            result.bytes.append(bytes, used);
                            event->PauseFor(chunks == 1 ? 50 : 200);
                            return used;
                        },
                        {}, [&](auto response) { result.Done(response); });
    result.Wait();
    CHECK(chunks == 2 && result.bytes == reply.body);
    CHECK(result.ended - start >= Ms(230) && result.ended - start < Ms(800));
    client.closeClient();
    CHECK(result.calls == 1);
    server.AssertDone();
}
void CloseWakesDelayedIdle() {
    for (unsigned i = 0; i < 32; ++i) {
        AsyncHttpClient client(Config());
        std::atomic<unsigned> calls{0};
        client.sendCallback(Request("http://127.0.0.1:1/never", EpochMs() + 60000), {}, {}, [&](auto) { ++calls; });
        const auto start = Clock::now();
        client.closeClient();
        CHECK(calls == 1 && Clock::now() - start < Ms(500));
    }
}
}  // namespace
int main(int argc, char** argv) {
    try {
        benchmark = argc == 2 && std::string(argv[1]) == "--benchmark";
        Logger::getInstance().setAsyncLogLevel(ERROR);
        Idle();
        Delayed();
        Paused(true, false);
        Paused(false, true);
        Paused(false, false);
        NetworkWait();
        SmallIo(true);
        SmallIo(false);
        if (!benchmark) {
#ifndef TOS_SCHEDULING_BASELINE
            AsyncTransportTestAccess::WaitPlan();
#endif
            EarlierQueuedDeadline();
            ResumeOverridesTimerAndPausedTimeout(false);
            ResumeOverridesTimerAndPausedTimeout(true);
            CompletionDoesNotWaitForOtherSocket();
            CachedCallbackCanPauseAgain();
            CloseWakesDelayedIdle();
            fail_eventfd = true;
            Paused(false, true);
            CHECK(!fail_eventfd);
            fail_eventfd = true;
            ResumeOverridesTimerAndPausedTimeout(true);
            CHECK(!fail_eventfd);
        }
        CHECK(g_req_ctx_create_cnt == g_req_ctx_destroy_cnt);
        std::cout << "PASS scheduling and exact-once bytes/lifetimes\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
