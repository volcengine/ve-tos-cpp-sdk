#include "LoopbackHttpFixture.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "logger/logger.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <new>

// Single-shot faults are thread-local. WorkerSetupFailures arms them explicitly
// on the worker after curl registration, before active-set ownership transfer.
namespace fault {
thread_local long remaining = -1;
thread_local bool hit = false;
thread_local size_t allocations = 0;
bool Fail() {
    if (remaining < 0) return false;
    if (remaining-- != 0) return false;
    remaining = -1;
    hit = true;
    return true;
}
}  // namespace fault
void* operator new(size_t n) {
    ++fault::allocations;
    if (fault::Fail()) throw std::bad_alloc();
    if (auto* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }

using namespace VolcengineTos;
using Server = tos_test::LoopbackHttpFixture;
#define CHECK(x)                                                                                       \
    do {                                                                                               \
        if (!(x)) throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " #x); \
    } while (false)

namespace VolcengineTos {
struct AsyncTransportTestAccess {
    static void CurlBoundaries() {
        auto request = std::make_shared<HttpRequest>();
        request->setContentLength(8);
        AsyncHttpClient::RequestContext ctx(request);
        ctx.event_ = std::make_shared<AsyncEvent>();
        ctx.on_data_send_with_event_ = [](char*, size_t, AsyncEvent*) -> size_t { throw 42; };
        char bytes[8]{};
        CHECK(AsyncHttpClient::readCallback(bytes, 1, sizeof(bytes), &ctx) == CURL_READFUNC_ABORT);
        CHECK(AsyncHttpClient::readCallback(bytes, size_t(-1), 2, &ctx) == CURL_READFUNC_ABORT);
        CHECK(AsyncHttpClient::headerCallback(bytes, size_t(-1), 2, &ctx) == 0);
        CHECK(AsyncHttpClient::writeCallback(bytes, size_t(-1), 2, &ctx) == 0);
        std::string header = "X-Contract: " + std::string(256, 'h') + "\r\n";
        fault::hit = false;
        fault::remaining = 0;
        CHECK(AsyncHttpClient::headerCallback(&header[0], 1, header.size(), &ctx) == 0);
        CHECK(fault::hit);
        CHECK(AsyncHttpClient::headerCallback(&header[0], 1, header.size(), &ctx) == header.size());
        unsigned calls = 0;
        ctx.on_request_finished_ = [&](auto) {
            ++calls;
            throw 17;
        };
        ctx.notifyFinished();
        ctx.notifyFinished();
        CHECK(calls == 1);
    }
};
}  // namespace VolcengineTos

TransportConfig Config(int capacity = 8) {
    TransportConfig config;
    config.setEventThreadCount(1);
    config.setMaxConnections(1);
    config.setMaxRequestQueue(capacity);
    config.setRequestTimeout(1500);
    config.setConnectTimeout(1000);
    return config;
}
std::shared_ptr<HttpRequest> Request(const std::string& url, bool delayed = false) {
    auto request = std::make_shared<HttpRequest>();
    request->setUrl(Url(url));
    if (delayed)
        request->setNotSendUtilMs(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count() +
            60000);
    return request;
}
void AdmissionAndClose() {
    const auto created = g_req_ctx_create_cnt.load(), destroyed = g_req_ctx_destroy_cnt.load();
    AsyncHttpClient client(Config());
    auto request = Request("http://127.0.0.1:1/not-sent", true);
    std::atomic<unsigned> calls{0};
    std::vector<std::thread> threads;
    for (unsigned p = 0; p != 8; ++p)
        threads.emplace_back([&] {
            for (unsigned i = 0; i != 8; ++i) client.sendCallback(request, {}, {}, [&](auto) { ++calls; });
        });
    for (auto& thread : threads) thread.join();
    CHECK(calls == 56);
    CHECK(g_req_ctx_create_cnt.load() - created == 8);
    client.closeClient();
    CHECK(calls == 64);
    CHECK(g_req_ctx_destroy_cnt.load() - destroyed == 8);
    // Immediate rejection invokes callback once even when it throws; future ABI remains usable.
    auto result = client.send(request, {}, {},
                              [&](auto) {
                                  ++calls;
                                  throw 1;
                              },
                              {}, {}, {});
    CHECK(result.valid() && calls == 65);
    bool failed = false;
    try {
        result.get();
    } catch (const std::exception&) {
        failed = true;
    }
    CHECK(failed);
}
void SubmissionAllocationFailures() {
    auto request = Request("http://127.0.0.1:1/not-sent", true);
    unsigned injected = 0;
    for (long index = 0; index != 48; ++index) {
        AsyncHttpClient client(Config(1));
        const auto created = g_req_ctx_create_cnt.load(), destroyed = g_req_ctx_destroy_cnt.load();
        unsigned calls = 0;
        std::function<void(std::shared_ptr<HttpResponse>)> done = [&](auto) { ++calls; };
        fault::hit = false;
        fault::remaining = index;
        bool threw = false;
        try {
            client.sendCallback(request, {}, {}, done);
        } catch (const std::bad_alloc&) {
            threw = true;
        }
        fault::remaining = -1;
        if (fault::hit) ++injected;
        if (threw) {
            CHECK(calls == 0);
            // All failed-construction admission leases must have been returned.
            client.sendCallback(request, {}, {}, done);
            CHECK(calls == 0);
        }
        client.closeClient();
        CHECK(calls == 1);
        CHECK(g_req_ctx_create_cnt.load() - created == g_req_ctx_destroy_cnt.load() - destroyed);
    }
    std::cout << "submission allocation faults exercised=" << injected << '\n';
    CHECK(injected >= 3);
}
void CallbackOnlyAvoidsFutureAllocations() {
    auto request = Request("http://127.0.0.1:1/not-sent", true);
    auto measure = [&](bool legacy) {
        AsyncHttpClient client(Config(1));
        const auto before = fault::allocations;
        if (legacy)
            (void)client.send(request, {}, {}, {}, {}, {}, {});
        else
            client.sendCallback(request, {}, {}, {});
        const auto count = fault::allocations - before;
        client.closeClient();
        return count;
    };
    const auto callbacks = measure(false), futures = measure(true);
    CHECK(callbacks < futures);
    std::cout << "submission allocations callback=" << callbacks << " legacy_future=" << futures << '\n';
}
void CurlCacheAllocationFailures() {
    // No network request is made. A long proxy option guarantees a C++ string
    // allocation in both growPool and Release (which runs from a destructor).
    auto config = Config();
    config.setProxyHost(std::string(128, 'x') + ".invalid");
    config.setProxyPort(1);
    CurlCache cache(config, 1);
    fault::hit = false;
    fault::remaining = 0;
    bool failed = false;
    try { (void)cache.Acquire(); } catch (const std::bad_alloc&) { failed = true; }
    fault::remaining = -1;
    CHECK(failed && fault::hit);
    CURL* handle = cache.Acquire();
    CHECK(handle != nullptr);
    fault::hit = false;
    fault::remaining = 0;
    cache.Release(handle);  // Must clean up rather than throw/terminate.
    fault::remaining = -1;
    CHECK(fault::hit);
    handle = cache.Acquire();
    CHECK(handle != nullptr);  // Neither failure consumed a pool slot.
    cache.Release(handle);
}
void CloseRacesSubmission() {
    const auto created = g_req_ctx_create_cnt.load(), destroyed = g_req_ctx_destroy_cnt.load();
    AsyncHttpClient client(Config(8));
    auto request = Request("http://127.0.0.1:1/not-sent", true);
    std::atomic<unsigned> calls{0};
    std::atomic<bool> start{false};
    client.sendCallback(request, {}, {}, [&](auto) { ++calls; });
    std::vector<std::thread> threads;
    for (unsigned p = 0; p != 8; ++p)
        threads.emplace_back([&] {
            while (!start) std::this_thread::yield();
            for (unsigned i = 0; i != 128; ++i) client.sendCallback(request, {}, {}, [&](auto) { ++calls; });
        });
    start = true;
    client.closeClient();
    for (auto& thread : threads) thread.join();
    CHECK(calls == 1025);
    CHECK(g_req_ctx_create_cnt.load() - created == g_req_ctx_destroy_cnt.load() - destroyed);
}
void HeaderExceptionsAndLegacyFuture() {
    Server server({{"GET", "/status-throws", {}}, {"GET", "/length-throws", {}}, {"GET", "/healthy", {}}});
    AsyncHttpClient client(Config());
    for (int kind = 0; kind != 3; ++kind) {
        auto request =
            Request(server.Endpoint() + (kind == 0 ? "/status-throws" : kind == 1 ? "/length-throws" : "/healthy"));
        std::mutex mutex;
        std::condition_variable cv;
        unsigned calls = 0;
        int curl_error = -1;
        auto future = client.send(request, {}, {},
                                  [&](auto response) {
                                      std::lock_guard<std::mutex> lock(mutex);
                                      ++calls;
                                      curl_error = response->getCurlErrCode();
                                      cv.notify_all();
                                      throw 7;  // Completion exceptions must not kill the worker.
                                  },
                                  {},
                                  [kind](int) {
                                      if (kind == 0) throw 1;
                                  },
                                  [kind](int64_t) {
                                      if (kind == 1) throw 2;
                                  });
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls != 0; }));
        CHECK(calls == 1 && curl_error == (kind == 2 ? CURLE_OK : CURLE_WRITE_ERROR));
        CHECK(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    }
    client.closeClient();
    server.AssertDone();
}

void WorkerSetupFailures() {
    std::vector<Server::Step> steps;
    for (int kind = 0; kind != 4; ++kind)
        steps.push_back({"GET", "/healthy-" + std::to_string(kind), {}});
    Server server(std::move(steps));
    const auto created = g_req_ctx_create_cnt.load(), destroyed = g_req_ctx_destroy_cnt.load();
    AsyncHttpClient client(Config());
    std::atomic<unsigned> total_calls{0};
    for (int kind = 0; kind != 4; ++kind) {
        for (const bool healthy : {false, true}) {
            std::mutex mutex;
            std::condition_variable cv;
            unsigned calls = 0;
            int curl_error = -1;
            bool allocation_hit = false;
            auto future = client.send(
                Request(server.Endpoint() + (healthy ? "/healthy-" : "/must-not-send-") + std::to_string(kind)),
                {}, {},
                [&](auto response) {
                    const bool hit = fault::hit;
                    fault::remaining = -1;
                    fault::hit = false;
                    std::lock_guard<std::mutex> lock(mutex);
                    ++calls;
                    ++total_calls;
                    allocation_hit = hit;
                    curl_error = response->getCurlErrCode();
                    cv.notify_all();
                    throw 8;  // Even setup completion exceptions must be contained.
                },
                [kind, healthy] {
                    if (healthy) return;
                    if (kind == 0) throw std::runtime_error("start callback failed");
                    if (kind == 1) throw 42;
                    if (kind == 2) throw std::bad_alloc();
                    fault::hit = false;
                    fault::remaining = 0;  // Next allocation is active_set_.insert.
                }, {}, {});
            std::unique_lock<std::mutex> lock(mutex);
            CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls != 0; }));
            CHECK(calls == 1);
            const int expected = healthy ? CURLE_OK : kind == 3 ? CURLE_OUT_OF_MEMORY : CURLE_ABORTED_BY_CALLBACK;
            CHECK(curl_error == expected);
            CHECK(allocation_hit == (!healthy && kind == 3));
            CHECK(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
            bool failed = false;
            try { future.get(); } catch (const std::exception&) { failed = true; }
            CHECK(failed == !healthy);
        }
    }
    client.closeClient();
    server.AssertDone();
    CHECK(total_calls == 8 && server.Requests().size() == 4);
    CHECK(g_req_ctx_create_cnt.load() - created == 8);
    CHECK(g_req_ctx_destroy_cnt.load() - destroyed == 8);
}
void PausedCloseOnlyOwnsOnce() {
    Server::Response response;
    response.body = "pause this body";
    Server server({{"GET", "/pause", response}});
    AsyncHttpClient client(Config());
    std::mutex mutex;
    std::condition_variable cv;
    bool paused = false;
    std::atomic<unsigned> calls{0};
    const auto created = g_req_ctx_create_cnt.load(), destroyed = g_req_ctx_destroy_cnt.load();
    client.sendCallback(Request(server.Endpoint() + "/pause"),
                        [&](char*, size_t, AsyncEvent* event) {
                            event->Pause();
                            std::lock_guard<std::mutex> lock(mutex);
                            paused = true;
                            cv.notify_all();
                            return size_t(0);
                        },
                        {}, [&](auto) { ++calls; });
    {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return paused; }));
    }
    client.closeClient();
    server.AssertDone();
    CHECK(calls == 1);
    CHECK(g_req_ctx_create_cnt.load() - created == 1);
    CHECK(g_req_ctx_destroy_cnt.load() - destroyed == 1);
}
int main() {
    try {
        Logger::getInstance().setAsyncLogLevel(ERROR);
        AsyncTransportTestAccess::CurlBoundaries();
        AdmissionAndClose();
        SubmissionAllocationFailures();
        CallbackOnlyAvoidsFutureAllocations();
        CurlCacheAllocationFailures();
        CloseRacesSubmission();
        HeaderExceptionsAndLegacyFuture();
        WorkerSetupFailures();
        PausedCloseOnlyOwnsOnce();
        std::cout << "PASS: capacity, allocation rollback, C boundaries, once, legacy future, paused close\n";
        return 0;
    } catch (const std::exception& error) {
        fault::remaining = -1;
        std::cerr << error.what() << '\n';
        return 1;
    }
}
