// Six actual native SDK entry points; no HTTP request is permitted. Invalid
// bucket input reaches their real inline validation only after factory setup.
#include "TosAsyncClient.h"
#include "executor/ProcessingPipline.h"
#include "logger/logger.h"

#include <atomic>
#include <cstdlib>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

namespace AllocationFault {
thread_local bool armed = false;
thread_local bool hit = false;
thread_local std::size_t remaining = 0;
thread_local std::size_t attempts = 0;

void Arm(std::size_t index) noexcept {
    armed = true;
    hit = false;
    remaining = index;
    attempts = 0;
}
void Disarm() noexcept { armed = false; }
bool Fail() noexcept {
    if (!armed) return false;
    ++attempts;
    if (remaining != 0) {
        --remaining;
        return false;
    }
    armed = false;  // Error delivery/cleanup may allocate after this one failure.
    hit = true;
    return true;
}
}  // namespace AllocationFault

namespace ResponseBufferWatch {
thread_local bool armed = false;
thread_local std::size_t expected_size = 0;
thread_local std::uintptr_t address = 0;
thread_local unsigned allocations = 0;
thread_local bool released = false;
void Allocated(void* memory, std::size_t size) noexcept {
    if (armed && size == expected_size) {
        ++allocations;
        address = reinterpret_cast<std::uintptr_t>(memory);
    }
}
void Deleted(void* memory) noexcept {
    if (memory && reinterpret_cast<std::uintptr_t>(memory) == address) released = true;
}
}  // namespace ResponseBufferWatch

void* operator new(std::size_t size) {
    if (AllocationFault::Fail()) throw std::bad_alloc();
    if (void* memory = std::malloc(size == 0 ? 1 : size)) {
        ResponseBufferWatch::Allocated(memory, size);
        return memory;
    }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept {
    ResponseBufferWatch::Deleted(memory);
    std::free(memory);
}
void operator delete[](void* memory) noexcept { ::operator delete(memory); }
void operator delete(void* memory, std::size_t) noexcept { ::operator delete(memory); }
void operator delete[](void* memory, std::size_t) noexcept { ::operator delete(memory); }

using namespace VolcengineTos;
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
enum class Api { Head, Get, List, Symlink, Put, Delete };
constexpr Api kApis[]{Api::Head, Api::Get, Api::List, Api::Symlink, Api::Put, Api::Delete};
struct CopyFailure {};
struct Lifetime {};
struct Observation {
    unsigned calls{0};
    bool success{false};
    bool closed_transport_error{false};
    bool throw_copy{false};
    bool fail_pool_allocation{false};
};

// std::function copies this actual completion while configuring the pipeline.
// A throwing copy is a deterministic pre-submit exception, not a transport mock.
struct CompletionCookie {
    std::shared_ptr<Lifetime> lifetime;
    std::shared_ptr<Observation> observation;
    CompletionCookie(std::shared_ptr<Lifetime> token, std::shared_ptr<Observation> state)
        : lifetime(std::move(token)), observation(std::move(state)) {}
    CompletionCookie(const CompletionCookie& other)
        : lifetime(other.lifetime), observation(other.observation) {
        if (observation->throw_copy) throw CopyFailure{};
    }
    template <typename Output>
    void operator()(Outcome<TosError, Output>& result) const {
        ++observation->calls;
        observation->success = result.isSuccess();
        observation->closed_transport_error = !result.isSuccess() && result.error().isClientError() &&
                result.error().getCode() == "UnhandledException" &&
                result.error().getMessage() == "asynchronous client is closed";
        if (observation->fail_pool_allocation) AllocationFault::Arm(0);
    }
};

struct Prepared {
    std::function<void()> invoke;
    std::weak_ptr<Lifetime> lifetime;
    std::shared_ptr<Observation> observation;
};

Prepared Prepare(TosAsyncClient& client, Api api, bool valid = false) {
    auto token = std::make_shared<Lifetime>();
    auto observation = std::make_shared<Observation>();
    CompletionCookie cookie(token, observation);
    Prepared prepared;
    prepared.lifetime = token;
    prepared.observation = observation;
    // Longer-than-SSO values exercise real input and builder-capture copying.
    const std::string key = "setup-contract/a-long-object-key-with-no-network";
    const std::string bucket = valid ? "contract-bucket" : "!invalid-bucket!";
    switch (api) {
        case Api::Head: {
            HeadObjectAsyncInput input(bucket, key);
            std::function<void(Outcome<TosError, HeadObjectAsyncOutput>&)> done(cookie);
            prepared.invoke = [&client, input, done] { client.headObjectAsync(input, done); };
            break;
        }
        case Api::Get: {
            GetObjectAsyncInput input(bucket, key);
            input.setRange("bytes=0-0");
            std::function<void(Outcome<TosError, GetObjectAsyncOutput>&)> done(cookie);
            OnDataReceiveWithEvent receive = [token](char*, std::size_t count, AsyncEvent*) { return count; };
            prepared.invoke = [&client, input, receive, done] { client.getObjectAsync(input, receive, done); };
            break;
        }
        case Api::List: {
            ListObjectsType2Input input(bucket);
            input.setPrefix(key);
            input.setMaxKeys(3);
            input.setAsyncResponseLimits({4096, 16, 1024});
            std::function<void(Outcome<TosError, ListObjectsType2Output>&)> done(cookie);
            prepared.invoke = [&client, input, done] { client.listObjectsType2Async(input, done); };
            break;
        }
        case Api::Symlink: {
            GetSymlinkAsyncInput input(bucket, key);
            input.setIfMatch("\"setup-only-selector\"");
            input.setVersionId("setup-only-version");
            std::function<void(Outcome<TosError, GetSymlinkAsyncOutput>&)> done(cookie);
            prepared.invoke = [&client, input, done] { client.getSymlinkAsync(input, done); };
            break;
        }
        case Api::Put: {
            PutObjectAsyncInput input(bucket, key, TransferEncoding::ContentLength);
            input.setContentLength(1);
            input.setIfNoneMatch("*");
            std::function<void(Outcome<TosError, PutObjectAsyncOutput>&)> done(cookie);
            OnDataSendWithEvent send = [token](char*, std::size_t, AsyncEvent*) -> size_t {
                throw std::runtime_error("unexpected data source dispatch");
            };
            prepared.invoke = [&client, input, send, done] { client.putObjectAsync(input, send, done); };
            break;
        }
        case Api::Delete: {
            DeleteObjectAsyncInput input(bucket, key);
            input.setIfMatch("\"setup-only-selector\"");
            std::function<void(Outcome<TosError, DeleteObjectAsyncOutput>&)> done(cookie);
            prepared.invoke = [&client, input, done] { client.deleteObjectAsync(input, done); };
            break;
        }
    }
    return prepared;
}

uint32_t LivePipelines() {
    return TosAsyncClient::g_pipline_create_cnt.load() - TosAsyncClient::g_pipline_destroy_cnt.load();
}

void CheckReleased(Prepared& prepared) {
    prepared.invoke = nullptr;
    CHECK(prepared.lifetime.expired());
}

void HealthyInlineValidation(TosAsyncClient& client, Api api) {
    auto prepared = Prepare(client, api);
    prepared.invoke();
    CHECK(prepared.observation->calls == 1 && !prepared.observation->success);
    CheckReleased(prepared);
}

void ClearOneCachedInstance(TosAsyncClient& client, Api api) {
    HealthyInlineValidation(client, api);  // Guarantees exactly one cached instance for this API.
    const auto before = LivePipelines();
    auto prepared = Prepare(client, api);
    prepared.observation->throw_copy = true;
    bool threw = false;
    try {
        prepared.invoke();
    } catch (const CopyFailure&) {
        threw = true;
    }
    CHECK(threw && prepared.observation->calls == 0);
    CheckReleased(prepared);
    CHECK(LivePipelines() + 1 == before);
}

void FirstCacheAllocationFailureDeletes(TosAsyncClient& client) {
    // Run before warming these six separate typed pools. Their first cache
    // insertion is the next allocation after inline completion returns.
    for (const auto api : kApis) {
        const auto before = LivePipelines();
        auto prepared = Prepare(client, api);
        prepared.observation->fail_pool_allocation = true;
        prepared.invoke();
        const bool injected = AllocationFault::hit;
        AllocationFault::Disarm();
        CHECK(injected && prepared.observation->calls == 1 && !prepared.observation->success);
        CheckReleased(prepared);
        CHECK(LivePipelines() == before);  // Not an untracked instance lost by a swallowed finalizer error.
    }
}

void AllocationSweep(TosAsyncClient& client) {
    for (const auto api : kApis) {
        for (const bool warm : {false, true}) {
            std::size_t failures = 0;
            bool reached_end = false;
            for (std::size_t index = 0; index < 512; ++index) {
                ClearOneCachedInstance(client, api);
                if (warm) HealthyInlineValidation(client, api);
                auto prepared = Prepare(client, api);
                bool threw = false;
                AllocationFault::Arm(index);
                try {
                    prepared.invoke();
                } catch (const std::bad_alloc&) {
                    threw = true;
                } catch (...) {
                    AllocationFault::Disarm();
                    throw;
                }
                const bool injected = AllocationFault::hit;
                AllocationFault::Disarm();
                CHECK(prepared.observation->calls <= 1 && !prepared.observation->success);
                CHECK(threw ? prepared.observation->calls == 0 : prepared.observation->calls == 1);
                CheckReleased(prepared);
                if (!injected) {
                    reached_end = true;
                    break;
                }
                ++failures;
            }
            CHECK(reached_end && failures >= 8);
            HealthyInlineValidation(client, api);  // A setup failure must not poison subsequent reuse.
            ClearOneCachedInstance(client, api);
        }
    }
}

void ClosedClientRejectsBeforeTransport(const ClientConfig& config) {
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), config);
    client.close();
    const auto requests = g_req_ctx_create_cnt.load();
    const auto pipelines = LivePipelines();
    for (const auto api : kApis) {
        auto prepared = Prepare(client, api, true);  // Valid inputs reach the sender.
        prepared.invoke();
        CHECK(prepared.observation->calls == 1 && !prepared.observation->success);
        CHECK(prepared.observation->closed_transport_error);
        CheckReleased(prepared);
        ClearOneCachedInstance(client, api);
    }
    client.close();
    CHECK(g_req_ctx_create_cnt.load() == requests && LivePipelines() == pipelines);
}

struct BufferOutput {
    int value{0};
    int64_t getRetryAfter() const { return 0; }
};

struct BufferLease {
    bool& destroyed;
    bool& buffer_released_when_destroyed;
    BufferLease(bool& destroyed, bool& released)
        : destroyed(destroyed), buffer_released_when_destroyed(released) {}
    ~BufferLease() {
        destroyed = true;
        buffer_released_when_destroyed = ResponseBufferWatch::released;
    }
};

// Exercise the real public pipeline buffering/parser/finalizer path. Only the
// physical sender is replaced. The sole done capture models LIST's last lease:
// its destruction must not return a large raw-buffer charge before deallocation.
void ResponseStoragePrecedesCompletionCaptureRelease() {
    for (const std::size_t capacity : {std::size_t{64 * 1024}, std::size_t{128 * 1024}}) {
        bool lease_destroyed = false;
        bool released_at_lease_destruction = false;
        unsigned replies = 0;
        unsigned finalizers = 0;
        bool success = false;
        OnDataReceiveWithEvent receive;
        std::function<void(std::shared_ptr<HttpResponse>)> finish;
        ResponseBufferWatch::armed = false;
        ResponseBufferWatch::address = 0;
        ResponseBufferWatch::allocations = 0;
        ResponseBufferWatch::released = false;
        {
            ProcessingPipline<int, BufferOutput> pipeline(1);
            auto lease = std::make_shared<BufferLease>(lease_destroyed, released_at_lease_destruction);
            pipeline
                .input2HttpRequest([](int&) {
                    auto request = std::make_shared<HttpRequest>();
                    request->setUrl(Url("http://127.0.0.1:1/buffer-recycle-contract"));
                    return request;
                })
                .httpRequestSender([&](auto, const auto& sink, const auto&, const auto& done, const auto&,
                                       const auto&, const auto&) {
                    receive = sink;
                    finish = done;
                })
                .expectResponse2Outcome([](auto, auto, TosError&, BufferOutput&) {})
                .defaultUnexpectResponse2Outcome()
                .json2Output([](BufferOutput& output, json& document) {
                    output.value = document.at("value").get<int>();
                })
                .responseJsonBounded({capacity, 4, 100})
                .onRequestDone([lease, &replies, &success](auto& outcome) {
                    ++replies;
                    success = outcome.isSuccess() && outcome.result().value == 7;
                })
                .afterPiplineFinish([&] {
                    ++finalizers;
                    pipeline.recycle();
                });
            lease.reset();
            pipeline.asyncExecute();
            CHECK(receive && finish && !lease_destroyed);
            std::string body = R"({"value":7})";
            ResponseBufferWatch::expected_size = capacity;
            ResponseBufferWatch::armed = true;
            const auto consumed = receive(body.data(), body.size(), nullptr);
            ResponseBufferWatch::armed = false;
            CHECK(consumed == body.size());
            CHECK(ResponseBufferWatch::allocations == 1 && ResponseBufferWatch::address != 0);
            CHECK(!ResponseBufferWatch::released && !lease_destroyed);
            auto response = std::make_shared<HttpResponse>();
            response->setStatus(0);
            response->setStatusCode(200);
            finish(response);
            finish(response);  // Recycled generation still rejects duplicate completion.
            CHECK(replies == 1 && finalizers == 1 && success && lease_destroyed);
            CHECK(released_at_lease_destruction == (capacity > 64 * 1024));
            CHECK(ResponseBufferWatch::released == (capacity > 64 * 1024));
            receive = nullptr;
            finish = nullptr;
        }
        // The existing <=64 KiB capacity cache survives recycle, but not the
        // physical pipeline. Both sizes must be released by this point.
        CHECK(ResponseBufferWatch::released);
        ResponseBufferWatch::address = 0;
    }
}
}  // namespace

int main() {
    try {
        Logger::getInstance().setAsyncLogLevel(ERROR);
        const auto request_created = g_req_ctx_create_cnt.load();
        const auto request_destroyed = g_req_ctx_destroy_cnt.load();
        const auto pipelines = LivePipelines();
        ClientConfig config;
        config.endPoint = "http://127.0.0.1:1";  // Unused: SDK input validation must reject before dispatch.
        config.isCustomDomain = true;
        config.event_thread_count_ = 1;
        config.async_transport_mode_ = AsyncTransportMode::Isolated;
        config.maxRetryCount = 0;
        config.requestTimeout = 1000;
        config.connectionTimeout = 1000;
        config.enableCRC = false;
        config.enableDebug = false;
        config.detail_log_ = false;
        TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), config);
        FirstCacheAllocationFailureDeletes(client);
        AllocationSweep(client);
        client.close();
        CHECK(LivePipelines() == pipelines);
        CHECK(g_req_ctx_create_cnt.load() == request_created);
        CHECK(g_req_ctx_destroy_cnt.load() == request_destroyed);
        ClosedClientRejectsBeforeTransport(config);
        ResponseStoragePrecedesCompletionCaptureRelease();
        std::cout << "async SDK setup: 6 APIs, cold/warm allocation sweeps, closed-client rejection, cache fallback and buffer release order passed\n";
        return 0;
    } catch (const std::exception& error) {
        AllocationFault::Disarm();
        std::cerr << "async SDK setup contract failed: " << error.what() << '\n';
        return 1;
    } catch (...) {
        AllocationFault::Disarm();
        std::cerr << "async SDK setup contract failed: unexpected exception\n";
        return 1;
    }
}
