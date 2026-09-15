// Actual native GET callbacks on one SDK worker and a finite loopback server.
// Single-shot allocation faults permit error delivery/cleanup to allocate; this
// is not a claim of recovery from persistent process-wide memory exhaustion.
#include "LoopbackHttpFixture.h"
#include "TosAsyncClient.h"
#include "logger/logger.h"
#include <curl/curl.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace AllocationFault {
thread_local bool armed = false;
thread_local bool hit = false;
thread_local std::size_t remaining = 0;
std::atomic<unsigned> failures{0};
void Arm(std::size_t index) noexcept {
    remaining = index;
    hit = false;
    armed = true;
}
void Disarm() noexcept { armed = false; }
bool Fail() noexcept {
    if (!armed) return false;
    if (remaining != 0) {
        --remaining;
        return false;
    }
    armed = false;
    hit = true;
    failures.fetch_add(1);
    return true;
}
}  // namespace AllocationFault

void* operator new(std::size_t size) {
    if (AllocationFault::Fail()) throw std::bad_alloc();
    if (void* memory = std::malloc(size == 0 ? 1 : size)) return memory;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

using namespace VolcengineTos;
using Server = tos_test::LoopbackHttpFixture;
namespace VolcengineTos {
extern std::atomic<uint64_t> g_req_ctx_create_cnt;
extern std::atomic<uint64_t> g_req_ctx_destroy_cnt;
}  // namespace VolcengineTos

#define CHECK(expression)                                                                                 \
    do {                                                                                                 \
        if (!(expression))                                                                               \
            throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " + #expression); \
    } while (false)

namespace {
constexpr const char* kReason = "intentional receive failure before diagnostic formatting";
enum class Fault { Standard, NonStandard, NextDiagnosticAllocation, EscapingDiagnosticAllocation, None };
struct Lifetime {};

// The first stream-buffer allocation can be caught by iostream and converted
// into badbit. Keep the requested next-new case, but also find an allocation in
// this same toString state whose exception really escapes (e.g. CopyReasons).
// The scan is finite, local, and requires an observed bad_alloc, not a skip.
std::size_t EscapingDiagnosticAllocation() {
    AsyncEvent event;
    event.markFailed(kReason);
    (void)event.toString();  // Warm any implementation-specific locale state.
    for (std::size_t index = 0; index != 16; ++index) {
        bool escaped = false;
        AllocationFault::Arm(index);
        try {
            (void)event.toString();
        } catch (const std::bad_alloc&) {
            escaped = true;
        } catch (...) {
            AllocationFault::Disarm();
            throw;
        }
        const bool hit = AllocationFault::hit;
        AllocationFault::Disarm();
        CHECK(hit);
        if (escaped) return index;
    }
    throw std::runtime_error("no escaping toString allocation found within finite scan");
}

struct Observation {
    explicit Observation(Fault selected, std::size_t diagnostic_index)
        : fault(selected), diagnostic_index(diagnostic_index) {}
    const Fault fault;
    const std::size_t diagnostic_index;
    mutable std::mutex mutex;
    std::condition_variable cv;
    unsigned calls{0};
    bool success{false};
    int curl_error{0};
    bool client_error{false};
    bool injection_hit{false};
    bool injection_still_armed{false};
    std::atomic<unsigned> receives{0};
    std::string bytes;
    std::weak_ptr<Lifetime> lifetime;

    std::size_t Receive(char* data, std::size_t length, AsyncEvent* event) {
        ++receives;
        if (fault == Fault::Standard) throw std::runtime_error("intentional standard receive exception");
        if (fault == Fault::NonStandard) throw 17;
        if (fault == Fault::NextDiagnosticAllocation || fault == Fault::EscapingDiagnosticAllocation) {
            CHECK(event != nullptr);
            event->markFailed(kReason);
            AllocationFault::Arm(fault == Fault::NextDiagnosticAllocation ? 0 : diagnostic_index);
            return length;  // Terminal event, not short-consume, causes the diagnostic path.
        }
        std::lock_guard<std::mutex> lock(mutex);
        bytes.append(data, length);
        return length;
    }
    void Record(Outcome<TosError, GetObjectAsyncOutput>& result) {
        const bool hit = AllocationFault::hit;
        const bool armed = AllocationFault::armed;
        AllocationFault::Disarm();
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        success = result.isSuccess();
        curl_error = result.error().getCurlErrCode();
        client_error = result.error().isClientError();
        injection_hit = hit;
        injection_still_armed = armed;
        cv.notify_all();
    }
    void WaitAndCheck() {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls != 0; }));
        CHECK(calls == 1 && receives.load() >= 1);
        if (fault == Fault::None) {
            CHECK(success && curl_error == CURLE_OK && bytes == "healthy worker");
        } else {
            CHECK(!success && client_error && curl_error == CURLE_WRITE_ERROR);
            CHECK(bytes.empty() && receives.load() == 1);
        }
        if (fault == Fault::NextDiagnosticAllocation || fault == Fault::EscapingDiagnosticAllocation)
            CHECK(injection_hit && !injection_still_armed);
    }
    unsigned Calls() const {
        std::lock_guard<std::mutex> lock(mutex);
        return calls;
    }
};

ClientConfig Config(const Server& server) {
    ClientConfig config;
    config.endPoint = server.Endpoint();
    config.isCustomDomain = true;
    config.event_thread_count_ = 1;
    config.async_transport_mode_ = AsyncTransportMode::Isolated;
    config.maxConnections = 2;
    config.max_request_queue_ = 8;
    config.maxRetryCount = 0;
    config.requestTimeout = 2000;
    config.connectionTimeout = 1000;
    config.enableCRC = false;
    config.enableDebug = false;
    config.detail_log_ = false;
    config.connection_reuse_ = false;
    return config;
}

void ReceiveExceptionsStayInsideCurl() {
    const auto diagnostic_index = EscapingDiagnosticAllocation();
    const auto failures_before = AllocationFault::failures.load();
    const auto contexts_created = g_req_ctx_create_cnt.load();
    const auto contexts_destroyed = g_req_ctx_destroy_cnt.load();
    const std::vector<Fault> faults{Fault::Standard, Fault::None, Fault::NonStandard, Fault::None,
                                  Fault::NextDiagnosticAllocation, Fault::None,
                                  Fault::EscapingDiagnosticAllocation, Fault::None};
    std::vector<Server::Step> steps;
    for (std::size_t i = 0; i != faults.size(); ++i) {
        Server::Response response;
        response.body = faults[i] == Fault::None ? "healthy worker" : "reject these bytes";
        response.headers = {{"ETag", "\"receive-contract\""}, {"x-tos-request-id", "loopback-only"}};
        steps.push_back({"GET", "/receive-" + std::to_string(i), std::move(response)});
    }
    Server server(std::move(steps));
    std::vector<std::shared_ptr<Observation>> observations;
    observations.reserve(faults.size());
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    for (std::size_t i = 0; i != faults.size(); ++i) {
        auto observation = std::make_shared<Observation>(faults[i], diagnostic_index);
        auto token = std::make_shared<Lifetime>();
        observation->lifetime = token;
        observations.push_back(observation);
        GetObjectAsyncInput input("fixture-bucket", "receive-" + std::to_string(i));
        client.getObjectAsync(
                input,
                [observation, token](char* bytes, std::size_t length, AsyncEvent* event) {
                    return observation->Receive(bytes, length, event);
                },
                [observation, token](auto& result) { observation->Record(result); });
        token.reset();
        observation->WaitAndCheck();
    }
    client.close();  // Control thread, after all callbacks; joins this isolated worker.
    server.AssertDone();
    CHECK(server.Requests().size() == faults.size());
    CHECK(AllocationFault::failures.load() == failures_before + 2);
    for (const auto& observation : observations) {
        CHECK(observation->Calls() == 1);
        CHECK(observation->lifetime.expired());
    }
    // Diagnostic counts after close prove physical context release for this
    // isolated client; they are not a new per-request production drain API.
    CHECK(g_req_ctx_create_cnt.load() - contexts_created == faults.size());
    CHECK(g_req_ctx_destroy_cnt.load() - contexts_destroyed == faults.size());
}
}  // namespace

int main() {
    try {
        Logger::getInstance().setAsyncLogLevel(ERROR);
        ReceiveExceptionsStayInsideCurl();
        std::cout << "async SDK receive: 4 exception paths, 4 recovery GETs and close release passed\n";
        return 0;
    } catch (const std::exception& error) {
        AllocationFault::Disarm();
        std::cerr << "async SDK receive failed: " << error.what() << '\n';
        return 1;
    }
}
