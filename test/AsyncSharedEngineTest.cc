#include "AsyncLoadHttpFixture.h"
#include "AsyncTlsHttpFixture.h"
#include "LoopbackHttpFixture.h"
#include "AsyncEngine.h"
#include "TosAsyncClient.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "transport/http/async/AsyncHttpClientRegistry.h"
#include "logger/logger.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <sys/epoll.h>
#include <sys/eventfd.h>

using namespace VolcengineTos;
using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;
using Server = tos_test::AsyncLoadHttpFixture;
std::atomic<int> setup_fault{0};
std::atomic<int> global_curl_gate{0};
std::atomic<bool> global_curl_entered{false}, global_curl_release{false};
std::atomic<bool> global_curl_timed_out{false};
std::atomic<int> global_curl_wait_ms{5000};
extern "C" CURLcode __real_curl_global_init(long);
extern "C" void __real_curl_global_cleanup();
void GlobalCurlGate(int phase) noexcept {
    if (global_curl_gate == phase) {
        global_curl_entered = true;
        const auto deadline = Clock::now() + Ms(global_curl_wait_ms.load());
        while (!global_curl_release) {
            if (Clock::now() >= deadline) {
                // Never throw through a libcurl C hook or engine destructor.
                // Let teardown proceed and fail on the test control thread.
                global_curl_timed_out = true;
                return;
            }
            std::this_thread::sleep_for(Ms(1));
        }
    }
}
extern "C" CURLcode __wrap_curl_global_init(long flags) {
    GlobalCurlGate(1);
    return __real_curl_global_init(flags);
}
extern "C" void __wrap_curl_global_cleanup() {
    GlobalCurlGate(2);
    __real_curl_global_cleanup();
}
extern "C" CURL* __real_curl_easy_init();
extern "C" CURLM* __real_curl_multi_init();
extern "C" CURLMcode __real_curl_multi_add_handle(CURLM*, CURL*);
extern "C" int __real_eventfd(unsigned, int);
extern "C" int __real_epoll_ctl(int, int, int, epoll_event*);
extern "C" CURL* __wrap_curl_easy_init() {
    return setup_fault == 1 ? nullptr : __real_curl_easy_init();
}
extern "C" CURLM* __wrap_curl_multi_init() {
    return setup_fault == 2 ? nullptr : __real_curl_multi_init();
}
extern "C" CURLMcode __wrap_curl_multi_add_handle(CURLM* multi, CURL* easy) {
    return setup_fault == 3 ? CURLM_OUT_OF_MEMORY : __real_curl_multi_add_handle(multi, easy);
}
extern "C" int __wrap_eventfd(unsigned initial, int flags) {
    if (setup_fault == 4) {
        errno = EMFILE;
        return -1;
    }
    return __real_eventfd(initial, flags);
}
extern "C" int __wrap_epoll_ctl(int epoll, int op, int fd, epoll_event* event) {
    if (setup_fault == 5 && op == EPOLL_CTL_ADD && event && event->data.u64) {
        errno = ENOSPC;
        return -1;
    }
    return __real_epoll_ctl(epoll, op, fd, event);
}
#define CHECK(x)                                                                    \
    do {                                                                            \
        if (!(x))                                                                   \
            throw std::runtime_error("line " + std::to_string(__LINE__) + ": " #x); \
    } while (false)
template <typename F>
void Until(F f, int ms = 5000) {
    const auto deadline = Clock::now() + Ms(ms);
    while (!f()) {
        CHECK(Clock::now() < deadline);
        std::this_thread::sleep_for(Ms(1));
    }
}
size_t DirectoryCount(const char* path) {
    auto* dir = opendir(path);
    CHECK(dir);
    size_t count = 0;
    while (auto* e = readdir(dir))
        if (e->d_name[0] != '.')
            ++count;
    closedir(dir);
    return count;
}
long Rss() {
    std::ifstream in("/proc/self/status");
    std::string line;
    while (std::getline(in, line))
        if (line.compare(0, 6, "VmRSS:") == 0)
            return std::stol(line.substr(6));
    return 0;
}
TransportConfig MakeTransportConfig(int timeout = 4000) {
    TransportConfig c;
    c.setRequestTimeout(timeout);
    c.setConnectTimeout(1500);
    c.setDetailLog(false);
    return c;
}
struct Result {
    std::atomic<unsigned> calls{0};
    int code{-1};
    size_t bytes{0};
    bool bad{false};
    void Done(std::shared_ptr<HttpResponse> r) {
        code = r->getCurlErrCode();
        ++calls;
    }
    void Wait(int expected = 0, int timeout = 5000) {
        Until([&] { return calls.load() != 0; }, timeout);
        if (code != expected)
            throw std::runtime_error("curl result " + std::to_string(code) + " expected " + std::to_string(expected));
        CHECK(calls == 1);
        CHECK(!bad);
    }
};
std::shared_ptr<HttpRequest> Request(Server& s, size_t bytes = 4096, int delay = 0, bool put = false) {
    auto r = std::make_shared<HttpRequest>(put ? "PUT" : "GET");
    r->setUrl(Url(s.Endpoint() + "/bytes/" + std::to_string(bytes) + "/" + std::to_string(delay)));
    if (put)
        r->setContentLength(bytes);
    return r;
}
std::shared_ptr<Result> Send(AsyncHttpClient& c, const std::shared_ptr<HttpRequest>& req, int pause_ms = 0) {
    auto r = std::make_shared<Result>();
    bool put = req->method() == "PUT";
    c.sendCallback(
            req,
            put ? OnDataReceiveWithEvent{} : OnDataReceiveWithEvent([r, pause_ms](char* p, size_t n, AsyncEvent* e) {
                bool pause = r->bytes == 0 && pause_ms;
                size_t used = pause ? std::min<size_t>(1, n) : n;
                if (std::find_if(p, p + used, [](char c) { return c != 'x'; }) != p + used)
                    r->bad = true;
                r->bytes += used;
                if (pause)
                    e->PauseFor(pause_ms);
                return used;
            }),
            put ? OnDataSendWithEvent([r, size = req->getContentLength()](char* p, size_t n, AsyncEvent*) {
                size_t used = std::min(n, static_cast<size_t>(size) - r->bytes);
                std::memset(p, 'x', used);
                r->bytes += used;
                return used;
            })
                : OnDataSendWithEvent{},
            [r](auto response) { r->Done(response); });
    return r;
}
void Drained(const std::shared_ptr<AsyncEngine>& e) {
    Until([&] { return e->stats().outstanding == 0; });
    auto s = e->stats();
    CHECK(s.active == 0 && s.reserved_buffer_bytes == 0);
    CHECK(s.accepted == s.results && s.results == s.released);
}
void Reuse() {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 4;
    std::mutex trace_mutex;
    std::map<uint64_t, std::vector<AsyncEngineTrace::Phase>> phases;
    o.trace = [&](const AsyncEngineTrace& t) {
        std::lock_guard<std::mutex> lock(trace_mutex);
        phases[t.request_id].push_back(t.phase);
    };
    auto e = AsyncEngine::Create(o);
    std::vector<std::unique_ptr<AsyncHttpClient>> clients;
    for (int i = 0; i != 100; ++i)
        clients.emplace_back(new AsyncHttpClient(MakeTransportConfig(1000 + i), e));
    for (int n = 0; n != 200; ++n)
        Send(*clients[n % 100], Request(server))->Wait();
    Drained(e);
    auto stats = e->stats();
    CHECK(stats.clients == 100 && stats.profiles == 1 && stats.multi_shards == 1);
    CHECK(stats.new_connections == 1 && stats.reused_connections == 199);
    CHECK(server.ConnectionsAccepted() == 1);
    {
        std::lock_guard<std::mutex> lock(trace_mutex);
        CHECK(phases.size() == 200);
        for (const auto& p : phases)
            CHECK(p.second == std::vector<AsyncEngineTrace::Phase>(
                                      {AsyncEngineTrace::Phase::Queued, AsyncEngineTrace::Phase::Started,
                                       AsyncEngineTrace::Phase::Result, AsyncEngineTrace::Phase::Released}));
    }
    clients[0]->closeClient();
    Send(*clients[1], Request(server))->Wait();
    Send(*clients[0], Request(server))->Wait(CURLE_ABORTED_BY_CALLBACK);
    Drained(e);
    CHECK(e->stats().new_connections == 1);
    e->close();
    server.Check();
}
void ProfilesAndBudget() {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 4;
    o.max_connections = 2;
    o.max_profiles = 4;
    std::mutex trace_mutex;
    std::vector<AsyncEngineTrace> traces;
    o.trace = [&](const AsyncEngineTrace& t) {
        std::lock_guard<std::mutex> lock(trace_mutex);
        traces.push_back(t);
    };
    auto e = AsyncEngine::Create(o);
    std::vector<std::unique_ptr<AsyncHttpClient>> clients;
    for (int i = 0; i != 4; ++i) {
        AsyncClientSharingOptions sharing;
        sharing.isolation_domain = std::to_string(i);
        clients.emplace_back(new AsyncHttpClient(MakeTransportConfig(), e, sharing));
    }
    std::vector<std::shared_ptr<Result>> results;
    for (int i = 0; i != 16; ++i)
        results.push_back(Send(*clients[i % 4], Request(server, 4096, 5)));
    try {
        for (const auto& r : results) {
            r->Wait();
            CHECK(e->stats().connection_credits <= 2);
        }
    } catch (...) {
        auto s = e->stats();
        std::cerr << "credit pressure: active=" << s.active << " outstanding=" << s.outstanding
                  << " credits=" << s.connection_credits << " loops=" << s.loop_iterations << '\n';
        std::lock_guard<std::mutex> lock(trace_mutex);
        for (auto& t : traces)
            std::cerr << "request=" << t.request_id << " worker=" << t.worker << " profile=" << t.profile_id
                      << " phase=" << int(t.phase) << " us=" << t.elapsed_us << " code=" << t.curl_code << '\n';
        throw;
    }
    Drained(e);
    CHECK(e->stats().profiles == 4 && e->stats().profile_evictions > 0);
    bool rejected = false;
    try {
        AsyncClientSharingOptions s;
        s.isolation_domain = "fifth";
        AsyncHttpClient extra(MakeTransportConfig(), e, s);
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    CHECK(rejected);
    e->close();
    CHECK(e->stats().connection_credits == 0);
    server.Check();
}
void DeadlineAndClose() {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 2;
    o.max_connections = 1;
    auto e = AsyncEngine::Create(o);
    AsyncHttpClient slow(MakeTransportConfig(4000), e), short_lived(MakeTransportConfig(50), e);
    auto first = Send(slow, Request(server, 4096, 300));
    Until([&] { return server.Requests() == 1; });
    auto expired = Send(short_lived, Request(server));
    expired->Wait(CURLE_OPERATION_TIMEDOUT);
    CHECK(server.Requests() == 1);
    auto queued = Send(short_lived, Request(server));
    short_lived.closeClient();
    queued->Wait(CURLE_ABORTED_BY_CALLBACK);
    first->Wait();
    Send(slow, Request(server))->Wait();
    Drained(e);
    CHECK(e->stats().timeouts == 1 && e->stats().cancelled == 1);
    e->close();
    server.Check();
}
void ReentryAndRace() {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 4;
    o.max_connections = 32;
    auto e = AsyncEngine::Create(o);
    auto a = std::make_shared<AsyncHttpClient>(MakeTransportConfig(), e);
    auto b = std::make_shared<AsyncHttpClient>(MakeTransportConfig(), e);
    std::atomic<unsigned> callbacks{0};
    a->sendCallback(
            Request(server), [](char*, size_t n, AsyncEvent*) { return n; }, {},
            [&](auto) {
                a->closeClient();
                b->sendCallback(
                        Request(server), [](char*, size_t n, AsyncEvent*) { return n; }, {},
                        [&](auto r) {
                            CHECK(r->getCurlErrCode() == 0);
                            ++callbacks;
                        });
                ++callbacks;
            });
    Until([&] { return callbacks == 2; });
    Drained(e);
    std::vector<std::thread> submitters;
    for (int t = 0; t != 8; ++t)
        submitters.emplace_back([&] {
            for (int i = 0; i != 64; ++i)
                b->sendCallback(Request(server), {}, {}, [&](auto) { ++callbacks; });
        });
    std::thread closer([&] { b->closeClient(); });
    for (auto& t : submitters)
        t.join();
    closer.join();
    Until([&] { return callbacks == 514; });
    Drained(e);
    e->close();
    // Last engine + client owner can disappear on an engine callback safely.
    auto last_engine = AsyncEngine::Create();
    auto last_client = std::make_shared<AsyncHttpClient>(MakeTransportConfig(), last_engine);
    std::weak_ptr<AsyncEngine> weak = last_engine;
    std::atomic<bool> last_done{false};
    last_client->sendCallback(Request(server, 4096, 50), {}, {}, [held = last_client, &last_done](auto) mutable {
        held.reset();
        last_done = true;
    });
    last_client.reset();
    last_engine.reset();
    Until([&] { return last_done && weak.expired(); });
    // Callback return is not SO quiescence. A later engine close does not
    // pretend to join that other engine; the process lifecycle joiner owns it.
}
void AdmissionAndHeaders() {
    Server server;
    AsyncEngineOptions o;
    o.max_requests = 2;
    o.max_requests_per_client = 1;
    auto e = AsyncEngine::Create(o);
    AsyncHttpClient a(MakeTransportConfig(), e), b(MakeTransportConfig(), e);
    auto first = Send(a, Request(server, 4096, 100));
    Send(a, Request(server))->Wait(CURLE_AGAIN);
    Send(b, Request(server))->Wait();
    first->Wait();
    Drained(e);
    auto oversized = Request(server);
    oversized->setHeader("X-Oversized", std::string(70000, 'a'));
    Send(a, oversized)->Wait(CURLE_AGAIN);
    auto delayed = Request(server);
    delayed->setNotSendUtilMs(
            std::chrono::duration_cast<Ms>(std::chrono::system_clock::now().time_since_epoch()).count() + 60000);
    auto r = Send(a, delayed);
    a.beginClose();
    a.closeClient();
    r->Wait(CURLE_ABORTED_BY_CALLBACK);
    Drained(e);
    e->close();
    server.Check();
}
void ReclaimAndCrossClose() {
    Server server;
    AsyncEngineOptions o;
    o.max_connections = 2;
    auto e = AsyncEngine::Create(o);
    AsyncClientSharingOptions separate;
    separate.isolation_domain = "second";
    AsyncHttpClient a(MakeTransportConfig(), e), b(MakeTransportConfig(), e, separate);
    Send(a, Request(server))->Wait();
    Send(b, Request(server))->Wait();
    Drained(e);
    CHECK(e->stats().connection_credits == 2);
    const auto count = server.Requests();
    auto first = Send(a, Request(server, 4096, 300));
    auto second = Send(a, Request(server, 4096, 300));
    // Idle B must surrender its credit without waiting for the 60s idle TTL.
    Until([&] { return server.Requests() == count + 2; }, 200);
    first->Wait();
    second->Wait();
    Drained(e);
    e->close();

    auto x = AsyncEngine::Create(), y = AsyncEngine::Create();
    AsyncHttpClient cx(MakeTransportConfig(), x), cy(MakeTransportConfig(), y);
    std::atomic<unsigned> entered{0}, returned{0};
    auto close_other = [&](AsyncHttpClient& other, const std::shared_ptr<AsyncEngine>& engine) {
        ++entered;
        Until([&] { return entered == 2; });  // deliberate cross-callback deadlock test
        other.closeClient();
        CHECK(!engine->close());  // never join another engine from a reactor
        ++returned;
    };
    cx.sendCallback(Request(server), {}, {}, [&](auto) { close_other(cy, y); });
    cy.sendCallback(Request(server), {}, {}, [&](auto) { close_other(cx, x); });
    Until([&] { return returned == 2; });
    Drained(x);
    Drained(y);
    CHECK(x->close() && y->close());
    server.Check();
}
void CrossWorkerCapacity() {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 2;
    o.max_connections = 2;
    o.affinity_spill_threshold = 1;
    auto e = AsyncEngine::Create(o);
    AsyncHttpClient client(MakeTransportConfig(), e);
    // Origin affinity sends the first request to A, spills the second to B,
    // then places the third back on A (both workers have one outstanding IO).
    auto slow = Send(client, Request(server, 4096, 900));
    Until([&] { return server.Requests() == 1; });
    auto fast = Send(client, Request(server, 4096, 150));
    Until([&] { return server.Requests() == 2; });
    auto queued = Send(client, Request(server));
    // B finishing must wake A and yield capacity, while A's first IO is still
    // active. Neither a request deadline nor the idle profile TTL is a wakeup.
    queued->Wait(CURLE_OK, 600);
    CHECK(slow->calls == 0);
    fast->Wait();
    slow->Wait();
    Drained(e);
    e->close();
    server.Check();
}
struct GlobalLifetimeThreads {
    std::thread owner, competitor;
    std::exception_ptr owner_error, competitor_error;
    ~GlobalLifetimeThreads() {
        ReleaseAndJoin();
    }
    void ReleaseAndJoin() {
        global_curl_release = true;
        if (owner.joinable())
            owner.join();
        if (competitor.joinable())
            competitor.join();
        global_curl_gate = 0;
        global_curl_wait_ms = 5000;
    }
};
struct InjectedLifetimeFailure {};
void GlobalLifetimeAttempt(int phase, int failure_point = 0) {
    auto previous = phase == 2 ? AsyncEngine::Create() : nullptr;
    global_curl_entered = false;
    global_curl_release = false;
    global_curl_timed_out = false;
    global_curl_gate = phase;
    std::shared_ptr<AsyncEngine> first, second;
    std::atomic<bool> entering{false}, returned{false};
    // Declared after all captured objects: unwind joins before they die.
    GlobalLifetimeThreads threads;
    threads.owner = std::thread([&] {
        try {
            if (phase == 1)
                first = AsyncEngine::Create();
            else
                previous.reset();
            if (failure_point == 3)
                throw InjectedLifetimeFailure{};
        } catch (...) {
            threads.owner_error = std::current_exception();
        }
    });
    Until([&] { return global_curl_entered.load(); });
    if (failure_point == 1)
        throw InjectedLifetimeFailure{};
    threads.competitor = std::thread([&] {
        try {
            entering = true;
            second = AsyncEngine::Create();
            returned = true;
        } catch (...) {
            threads.competitor_error = std::current_exception();
        }
    });
    Until([&] { return entering.load(); });
    if (failure_point == 2)
        throw InjectedLifetimeFailure{};
    std::this_thread::sleep_for(Ms(100));
    const bool escaped = returned.load();
    threads.ReleaseAndJoin();
    if (threads.owner_error)
        std::rethrow_exception(threads.owner_error);
    if (threads.competitor_error)
        std::rethrow_exception(threads.competitor_error);
    // Atomic reference counts alone do not serialize global initialization
    // or the zero-to-one transition against the previous owner's cleanup.
    CHECK(!escaped && !global_curl_timed_out);
    if (first)
        first->close();
    second->close();
}
void ConcurrentGlobalLifetime() {
    for (int phase : {1, 2})
        GlobalLifetimeAttempt(phase);
}
void GlobalLifetimeFailurePaths() {
    for (int phase : {1, 2}) {
        for (int failure_point : {1, 2, 3}) {
            bool caught = false;
            try {
                GlobalLifetimeAttempt(phase, failure_point);
            } catch (const InjectedLifetimeFailure&) {
                caught = true;
            }
            CHECK(caught && global_curl_gate == 0 && global_curl_release && !global_curl_timed_out);
            auto recovered = AsyncEngine::Create();
            CHECK(recovered->close());
        }
    }
    // Explicitly exercise the bounded gate with the release signal omitted.
    for (int phase : {1, 2}) {
        GlobalLifetimeThreads reset;
        global_curl_gate = phase;
        global_curl_release = false;
        global_curl_entered = false;
        global_curl_timed_out = false;
        global_curl_wait_ms = 20;
        const auto started = Clock::now();
        GlobalCurlGate(phase);
        CHECK(global_curl_entered && global_curl_timed_out && Clock::now() - started < Ms(1500));
    }
    ConcurrentGlobalLifetime();  // Healthy initialization/cleanup still works.
}
void SetupFailures() {
    Server server;
    for (int fault : {1, 2, 3, 5}) {
        auto e = AsyncEngine::Create();
        AsyncHttpClient c(MakeTransportConfig(), e);
        setup_fault = fault;
        auto r = Send(c, Request(server));
        Until([&] { return r->calls.load() != 0; });
        CHECK(r->calls == 1 && r->code != 0);
        setup_fault = 0;
        Drained(e);
        Send(c, Request(server))->Wait();
        Drained(e);
        e->close();
    }
    setup_fault = 4;
    bool threw = false;
    try {
        (void)AsyncEngine::Create();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    setup_fault = 0;
    CHECK(threw);
    auto e = AsyncEngine::Create();
    {
        AsyncHttpClient c(MakeTransportConfig(), e);
        Send(c, Request(server))->Wait();
    }
    Drained(e);
    e->close();
    server.Check();

    tos_test::LoopbackHttpFixture::Response response;
    response.headers["X-Bound"] = std::string(2000, 'x');
    response.allow_peer_disconnect = true;
    tos_test::LoopbackHttpFixture large_headers({{"GET", "/headers", response}});
    AsyncEngineOptions o;
    o.max_response_header_bytes = 1024;
    auto limited = AsyncEngine::Create(o);
    AsyncHttpClient c(MakeTransportConfig(), limited);
    auto req = std::make_shared<HttpRequest>("GET");
    req->setUrl(Url(large_headers.Endpoint() + "/headers"));
    Send(c, req)->Wait(CURLE_WRITE_ERROR);
    Drained(limited);
    limited->close();
}
void PublicApiAndLegacyShared() {
    Server server;
    auto e = AsyncEngine::Create();
    ClientConfig c;
    c.endPoint = server.Endpoint();
    c.isCustomDomain = true;
    c.requestTimeout = 2000;
    c.maxRetryCount = 0;
    c.detail_log_ = false;
    c.enableCRC = false;
    TosAsyncClient a("test", StaticCredentials("offline-a", "offline-signing-key-a"), c, e);
    TosAsyncClient b("test", StaticCredentials("offline-b", "offline-signing-key-b"), c, e);
    auto get = [&](TosAsyncClient& client) {
        GetObjectAsyncInput in("offline-bucket", "bytes/4096/0");
        std::atomic<unsigned> calls{0};
        bool ok = false;
        client.getObjectAsync(
                in, [](char*, size_t n, AsyncEvent*) { return n; },
                [&](auto& out) {
                    ok = out.isSuccess();
                    ++calls;
                });
        Until([&] { return calls.load(); });
        CHECK(calls == 1 && ok);
    };
    get(a);
    get(b);
    a.close();
    get(b);
    b.close();
    Drained(e);
    CHECK(server.ConnectionsAccepted() == 1);
    const auto threads = DirectoryCount("/proc/self/task"), fds = DirectoryCount("/proc/self/fd");
    const auto rss = Rss();
    std::vector<std::unique_ptr<TosAsyncClient>> sdk_clients;
    for (int i = 0; i != 1000; ++i)
        sdk_clients.emplace_back(new TosAsyncClient("test", StaticCredentials("offline", "offline-signing"), c, e));
    CHECK(DirectoryCount("/proc/self/task") == threads && DirectoryCount("/proc/self/fd") == fds);
    CHECK(e->stats().clients == 1000 && e->stats().profiles == 1);
    std::cout << "public_sdk_clients=1000 rss_delta_kib=" << Rss() - rss << " new_threads=0 new_fds=0\n";
    get(*sdk_clients.front());
    get(*sdk_clients.back());
    sdk_clients.front()->close();
    get(*sdk_clients.back());
    sdk_clients.clear();
    Drained(e);
    CHECK(e->stats().clients == 0 && server.ConnectionsAccepted() == 1);
    e->close();
    auto config = MakeTransportConfig();
    config.setAsyncTransportMode(AsyncTransportMode::Shared);
    auto x = AcquireAsyncHttpClient(config, false), y = AcquireAsyncHttpClient(config, false);
    CHECK(x != y);
    Send(*x, Request(server))->Wait();
    x->closeClient();
    Send(*y, Request(server))->Wait();
    bool mismatch = false;
    try {
        config.setEventThreadCount(2);
        (void)AcquireAsyncHttpClient(config, false);
    } catch (const std::invalid_argument&) {
        mismatch = true;
    }
    CHECK(mismatch);
    y->closeClient();
    server.Check();
}
void DensityAndLoad(bool large) {
    Server server;
    AsyncEngineOptions o;
    o.worker_count = 4;
    o.max_connections = 32;
    o.max_host_connections = 32;
    auto e = AsyncEngine::Create(o);
    const auto threads = DirectoryCount("/proc/self/task"), fds = DirectoryCount("/proc/self/fd");
    const auto rss_before_clients = Rss();
    std::vector<std::unique_ptr<AsyncHttpClient>> clients;
    for (int i = 0; i != 1000; ++i)
        clients.emplace_back(new AsyncHttpClient(MakeTransportConfig(60000), e));
    CHECK(DirectoryCount("/proc/self/task") == threads);
    CHECK(DirectoryCount("/proc/self/fd") == fds);
    CHECK(e->stats().profiles == 1 && e->stats().multi_shards == 0);
    const auto rss_after_clients = Rss();
    const auto idle_before = e->stats();
    std::this_thread::sleep_for(Ms(300));
    CHECK(e->stats().loop_iterations - idle_before.loop_iterations < 10);
    Send(*clients.front(), Request(server))->Wait();
    Drained(e);
    const auto warm_idle = e->stats();
    std::this_thread::sleep_for(Ms(400));
    const auto warm_after = e->stats();
    CHECK(warm_after.loop_iterations - warm_idle.loop_iterations < 16);
    CHECK(warm_after.worker_cpu_ns - warm_idle.worker_cpu_ns < 20000000);
    const auto before = e->stats();
    const auto started = Clock::now();
    const size_t bytes = large ? 100 * 1024 * 1024 : 4096;
    const size_t requests = large ? 16 : 1000;
    for (size_t i = 0; i != requests;) {
        std::vector<std::shared_ptr<Result>> batch;
        for (int j = 0; j != (large ? 8 : 32) && i < requests; ++j, ++i)
            batch.push_back(Send(*clients[i % clients.size()], Request(server, bytes, 0, i % 2), i % 3 == 0 ? 2 : 0));
        for (const auto& r : batch) {
            r->Wait(0, 70000);
            CHECK(r->bytes == bytes);
        }
    }
    Drained(e);
    const double wall = std::chrono::duration<double>(Clock::now() - started).count();
    auto after = e->stats();
    CHECK(after.connection_credits <= 32 && after.multi_shards <= 4);
    std::cout << "clients=1000 requests=" << requests << " bytes=" << bytes << " wall_s=" << wall
              << " worker_cpu_ms=" << (after.worker_cpu_ns - before.worker_cpu_ns) / 1000000.
              << " worker_cores=" << (after.worker_cpu_ns - before.worker_cpu_ns) / 1e9 / wall
              << " process_rss_kib=" << Rss() << " client_rss_delta_kib=" << rss_after_clients - rss_before_clients
              << " warm_idle_cpu_ms=" << (warm_after.worker_cpu_ns - warm_idle.worker_cpu_ns) / 1000000.
              << " warm_idle_iterations=" << warm_after.loop_iterations - warm_idle.loop_iterations
              << " connections=" << server.ConnectionsAccepted()
              << " iterations=" << after.loop_iterations - before.loop_iterations << '\n';
    clients.clear();
    CHECK(e->stats().clients == 0);
    e->close();
    server.Check();
}
void Tls(int version) {
    tos_test::AsyncTlsHttpFixture server(version), untrusted(version);
    AsyncEngineOptions o;
    o.worker_count = 2;
    o.affinity_spill_threshold = 1;
    std::mutex mutex;
    std::set<size_t> used_workers;
    o.trace = [&](const AsyncEngineTrace& t) {
        if (t.phase == AsyncEngineTrace::Phase::Started) {
            std::lock_guard<std::mutex> lock(mutex);
            used_workers.insert(t.worker);
        }
    };
    auto e = AsyncEngine::Create(o);
    auto config = MakeTransportConfig();
    config.setEnableVerifySsl(true);
    config.setCaFile(server.CaFile());
    AsyncHttpClient a(config, e), b(config, e);
    auto request = [&](const char* path) {
        auto r = std::make_shared<HttpRequest>("GET");
        r->setUrl(Url(server.Endpoint() + path));
        return r;
    };
    Send(a, request("/keep"))->Wait();
    Send(b, request("/keep"))->Wait();
    CHECK(server.Connections() == 1 && server.Handshakes() == 1);
    a.closeClient();
    Send(b, request("/keep"))->Wait();
    Send(b, request("/close"))->Wait();
    Send(b, request("/close"))->Wait();
    CHECK(server.Resumed() >= 1);
    // Hold the affinity worker occupied; the second request spills to another
    // worker and must resume a session cached by the first worker's CURLM.
    const auto previous = server.Requests();
    auto slow = Send(b, request("/slow"));
    Until([&] { return server.Requests() > previous; });
    const auto resumed_before = server.Resumed();
    Send(b, request("/keep"))->Wait();
    slow->Wait();
    {
        std::lock_guard<std::mutex> lock(mutex);
        CHECK(used_workers.size() == 2);
    }
    CHECK(server.Resumed() > resumed_before);
    auto large = Send(b, request("/large"), 2);
    large->Wait();
    CHECK(large->bytes == 100 * 1024 * 1024);

    // Neither an unverified connection nor its sessions can make a verified
    // client with the wrong CA succeed against the same origin.
    auto insecure = MakeTransportConfig();
    insecure.setEnableVerifySsl(false);
    AsyncHttpClient unchecked(insecure, e);
    Send(unchecked, request("/keep"))->Wait();
    auto wrong = config;
    wrong.setCaFile(untrusted.CaFile());
    AsyncHttpClient rejected(wrong, e);
    Send(rejected, request("/keep"))->Wait(CURLE_PEER_FAILED_VERIFICATION);
    AsyncClientSharingOptions generation;
    generation.tls_generation = "rotated";
    AsyncHttpClient rotated(config, e, generation);
    Send(rotated, request("/keep"))->Wait();
    CHECK(e->stats().profiles == 4);
    Drained(e);
    e->close();
    server.Check();
    untrusted.Check();
    std::cout << "tls_version=" << version << " connections=" << server.Connections()
              << " handshakes=" << server.Handshakes() << " resumed=" << server.Resumed() << '\n';
}
int main(int argc, char** argv) {
    try {
        std::signal(SIGPIPE, SIG_IGN);  // test TLS server writes after intentional cancellation
        Logger::getInstance().setAsyncLogLevel(ERROR);
        CHECK(argc == 2);
        std::string test = argv[1];
        if (test == "reuse")
            Reuse();
        else if (test == "profiles")
            ProfilesAndBudget();
        else if (test == "deadline")
            DeadlineAndClose();
        else if (test == "close")
            ReentryAndRace();
        else if (test == "admission")
            AdmissionAndHeaders();
        else if (test == "reclaim")
            ReclaimAndCrossClose();
        else if (test == "capacity")
            CrossWorkerCapacity();
        else if (test == "global_lifetime")
            ConcurrentGlobalLifetime();
        else if (test == "global_lifetime_failures")
            GlobalLifetimeFailurePaths();
        else if (test == "setup")
            SetupFailures();
        else if (test == "public")
            PublicApiAndLegacyShared();
        else if (test == "density")
            DensityAndLoad(false);
        else if (test == "large")
            DensityAndLoad(true);
        else if (test == "tls12")
            Tls(TLS1_2_VERSION);
#ifdef TLS1_3_VERSION
        else if (test == "tls13")
            Tls(TLS1_3_VERSION);
#endif
        else
            throw std::runtime_error("unknown scenario");
        std::cout << "PASS shared engine " << test << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
