#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace VolcengineTos {
class AsyncEngineCore;
class AsyncHttpClient;

// IDs are engine-local. Events deliberately contain no URL, headers or credentials.
struct AsyncEngineTrace {
    enum class Phase { Queued, Started, Result, Released };
    Phase phase{Phase::Queued};
    uint64_t request_id{0};
    uint64_t client_id{0};
    uint64_t profile_id{0};
    size_t worker{0};
    int64_t elapsed_us{0};
    int curl_code{0};
    bool new_connection{false};
    int64_t dns_us{0}, connect_us{0}, tls_us{0}, first_byte_us{0};
};

struct AsyncEngineOptions {
    size_t worker_count{1};
    size_t max_connections{512};
    size_t max_host_connections{64}; // per origin within each worker/profile CURLM
    size_t max_requests{4096}; // accepted requests, including queued and retiring
    size_t max_requests_per_client{256};
    size_t max_profiles{128};
    size_t max_request_header_bytes{64 * 1024};
    size_t max_response_header_bytes{64 * 1024};
    // Reservation for headers and the bounded transport backpressure buffer.
    // Does not include caller-owned bodies, file pipelines or TLS/curl internals.
    size_t max_transport_buffer_bytes{64 * 1024 * 1024};
    int64_t idle_profile_timeout_ms{60000};
    size_t affinity_spill_threshold{16};
    // Must be nonblocking. Exceptions are contained. Accepted request events
    // execute on their owner worker. Configure before Create().
    std::function<void(const AsyncEngineTrace&)> trace;
};

struct AsyncClientSharingOptions {
    std::string isolation_domain;
    // Change when CA/client certificate contents at the same path change.
    // The caller keeps material immutable for the lifetime of that generation.
    std::string tls_generation;
};

struct AsyncEngineStats {
    size_t worker_count{0}, clients{0}, profiles{0}, multi_shards{0};
    size_t outstanding{0}, active{0}, reserved_buffer_bytes{0};
    size_t connection_credits{0};
    uint64_t accepted{0}, rejected{0}, results{0}, released{0};
    uint64_t timeouts{0}, cancelled{0}, new_connections{0}, reused_connections{0};
    uint64_t wakeups{0}, loop_iterations{0}, profile_evictions{0};
    uint64_t worker_cpu_ns{0}; // sampled by workers before waiting and at exit
};

// Linux native async transport only; independent of Folly and the sync SDK.
// Share one instance explicitly across clients.
// Proxy environment variables are snapshotted at Create(); explicit client
// proxies override them. Do not mutate the process environment concurrently.
//
// close() must finish before
// unloading the SDK DSO. A process-wide lifecycle joiner handles last-owner
// destruction from a callback without making a worker join itself.
class AsyncEngine {
public:
    static std::shared_ptr<AsyncEngine> Create(const AsyncEngineOptions& options = {});
    ~AsyncEngine();
    AsyncEngine(const AsyncEngine&) = delete;
    AsyncEngine& operator=(const AsyncEngine&) = delete;
    void beginClose() noexcept;
    // Cancels accepted work and joins workers. On any shared-engine worker, initiates
    // closing and returns false; the control plane must subsequently join.
    bool close();
    AsyncEngineStats stats() const;

private:
    friend class AsyncHttpClient;
    explicit AsyncEngine(std::shared_ptr<AsyncEngineCore> core);
    std::shared_ptr<AsyncEngineCore> core_;
};
} // namespace VolcengineTos
