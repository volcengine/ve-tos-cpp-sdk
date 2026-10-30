#include "AsyncHttpClientRegistry.h"
#include "AsyncHttpClient.h"

#include <mutex>
#include <tuple>

namespace VolcengineTos {
namespace {
std::mutex registry_mutex;
std::weak_ptr<AsyncEngine> shared_engine;
std::tuple<size_t, size_t, size_t> shared_limits;
}

// Legacy Shared callers get individual client leases on a common engine.
// Transport profiles may differ; process budgets may not silently change.
// Prefer explicitly injecting AsyncEngine when configuring process resources.
std::shared_ptr<AsyncHttpClient> AcquireAsyncHttpClient(const TransportConfig& config, bool enable_crc) {
    if (config.getAsyncTransportMode() != AsyncTransportMode::Shared)
        return std::make_shared<AsyncHttpClient>(config, 0, enable_crc);
    AsyncEngineOptions options;
    options.worker_count = static_cast<size_t>(std::max(1, config.getEventThreadCount()));
    options.max_connections = static_cast<size_t>(std::max(1, config.getMaxConnections()));
    if (config.getMaxRequestQueue() > 0)
        options.max_requests = static_cast<size_t>(config.getMaxRequestQueue());
    options.max_requests_per_client = options.max_requests;
    const auto limits = std::make_tuple(options.worker_count, options.max_connections, options.max_requests);
    std::shared_ptr<AsyncEngine> engine;
    {
        std::lock_guard<std::mutex> lock(registry_mutex);
        engine = shared_engine.lock();
        if (engine && shared_limits != limits)
            throw std::invalid_argument("shared engine budget mismatch; inject an explicit AsyncEngine");
        if (!engine) {
            engine = AsyncEngine::Create(options);
            shared_engine = engine;
            shared_limits = limits;
        }
    }
    return std::make_shared<AsyncHttpClient>(config, std::move(engine));
}
} // namespace VolcengineTos
