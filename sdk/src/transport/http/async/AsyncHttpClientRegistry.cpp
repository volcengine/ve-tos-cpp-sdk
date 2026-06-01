#include "transport/http/async/AsyncHttpClientRegistry.h"


#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <tuple>

#include "logger/logger.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "utils/LogUtils.h"

using namespace VolcengineTos;
namespace VolcengineTos {
namespace {
struct AsyncHttpClientSharedConfigSnapshot {
    bool enable_verify_ssl = true;
    std::string proxy_host;
    int proxy_port = -1;
    std::string proxy_username;
    std::string proxy_password;
    int dns_cache_time = 0;
    int max_connections = 1000;
    int socket_timeout = 30000;
    std::string ca_path;
    std::string ca_file;
    int high_latency_log_threshold = 100;
    std::string client_crt;
    std::string client_key;
    int event_thread_count = 0;
    int max_request_queue = 0;
    bool connection_reuse = true;
    bool detail_log = true;
    int detail_log_interval_s = 60;
    int curl_multi_wait_timeout_ms = 50;
    bool enable_crc = false;
};

AsyncHttpClientSharedConfigSnapshot MakeSnapshot(const TransportConfig& config, const bool enable_crc) {
    AsyncHttpClientSharedConfigSnapshot snapshot;
    snapshot.enable_verify_ssl = config.isEnableVerifySsl();
    snapshot.proxy_host = config.getProxyHost();
    snapshot.proxy_port = config.getProxyPort();
    snapshot.proxy_username = config.getProxyUsername();
    snapshot.proxy_password = config.getProxyPassword();
    snapshot.dns_cache_time = config.getDnsCacheTime();
    snapshot.max_connections = config.getMaxConnections();
    snapshot.socket_timeout = config.getSocketTimeout();
    snapshot.ca_path = config.getCaPath();
    snapshot.ca_file = config.getCaFile();
    snapshot.high_latency_log_threshold = config.getHighLatencyLogThreshold();
    snapshot.client_crt = config.getClientCrt();
    snapshot.client_key = config.getClientKey();
    snapshot.event_thread_count = config.getEventThreadCount();
    snapshot.max_request_queue = config.getMaxRequestQueue();
    snapshot.connection_reuse = config.isConnectionReuse();
    snapshot.detail_log = config.isDetailLog();
    snapshot.detail_log_interval_s = config.getDetailLogInterval();
    snapshot.curl_multi_wait_timeout_ms = config.getCurlMultiWaitTimeoutMs();
    snapshot.enable_crc = enable_crc;
    return snapshot;
}

std::string DescribeSnapshot(const AsyncHttpClientSharedConfigSnapshot& snapshot) {
    std::ostringstream oss;
    oss << "verify_ssl=" << snapshot.enable_verify_ssl << ", proxy_host=" << snapshot.proxy_host
        << ", proxy_port=" << snapshot.proxy_port << ", dns_cache_time=" << snapshot.dns_cache_time
        << ", max_connections=" << snapshot.max_connections << ", socket_timeout=" << snapshot.socket_timeout
        << ", event_thread_count=" << snapshot.event_thread_count
        << ", max_request_queue=" << snapshot.max_request_queue
        << ", connection_reuse=" << snapshot.connection_reuse << ", detail_log=" << snapshot.detail_log
        << ", detail_log_interval_s=" << snapshot.detail_log_interval_s
        << ", curl_multi_wait_timeout_ms=" << snapshot.curl_multi_wait_timeout_ms
        << ", enable_crc=" << snapshot.enable_crc;
    return oss.str();
}

bool SameSnapshot(const AsyncHttpClientSharedConfigSnapshot& lhs, const AsyncHttpClientSharedConfigSnapshot& rhs) {
    return std::tie(lhs.enable_verify_ssl, lhs.proxy_host, lhs.proxy_port, lhs.proxy_username, lhs.proxy_password,
                    lhs.dns_cache_time, lhs.max_connections, lhs.socket_timeout, lhs.ca_path, lhs.ca_file,
                    lhs.high_latency_log_threshold, lhs.client_crt, lhs.client_key, lhs.event_thread_count,
                    lhs.max_request_queue, lhs.connection_reuse, lhs.detail_log, lhs.detail_log_interval_s,
                    lhs.curl_multi_wait_timeout_ms, lhs.enable_crc) ==
           std::tie(rhs.enable_verify_ssl, rhs.proxy_host, rhs.proxy_port, rhs.proxy_username, rhs.proxy_password,
                    rhs.dns_cache_time, rhs.max_connections, rhs.socket_timeout, rhs.ca_path, rhs.ca_file,
                    rhs.high_latency_log_threshold, rhs.client_crt, rhs.client_key, rhs.event_thread_count,
                    rhs.max_request_queue, rhs.connection_reuse, rhs.detail_log, rhs.detail_log_interval_s,
                    rhs.curl_multi_wait_timeout_ms, rhs.enable_crc);
}

std::mutex g_async_http_client_registry_mu;
std::weak_ptr<AsyncHttpClient> g_shared_async_http_client;
AsyncHttpClientSharedConfigSnapshot g_shared_async_http_client_snapshot;
bool g_shared_async_http_client_snapshot_valid = false;

}  // namespace

std::shared_ptr<AsyncHttpClient> AcquireAsyncHttpClient(const TransportConfig& config, const bool enable_crc) {
    if (config.getAsyncTransportMode() != AsyncTransportMode::Shared) {
        return std::make_shared<AsyncHttpClient>(config, 0, enable_crc);
    }

    const AsyncHttpClientSharedConfigSnapshot snapshot = MakeSnapshot(config, enable_crc);
    std::lock_guard<std::mutex> lock(g_async_http_client_registry_mu);

    auto existing = g_shared_async_http_client.lock();
    if (existing != nullptr) {
        if (g_shared_async_http_client_snapshot_valid && !SameSnapshot(g_shared_async_http_client_snapshot, snapshot)) {
            Logger::getInstance().warn(
                "AsyncHttpClient shared transport config mismatch, reuse first client config, first=",
                DescribeSnapshot(g_shared_async_http_client_snapshot), ", current=", DescribeSnapshot(snapshot));
        } else {
            Logger::getInstance().info("AsyncHttpClient shared transport reuse, config=",
                                       DescribeSnapshot(g_shared_async_http_client_snapshot_valid
                                                            ? g_shared_async_http_client_snapshot
                                                            : snapshot));
        }
        return existing;
    }

    auto created = std::make_shared<AsyncHttpClient>(config, 0, enable_crc);
    g_shared_async_http_client = created;
    g_shared_async_http_client_snapshot = snapshot;
    g_shared_async_http_client_snapshot_valid = true;
    Logger::getInstance().info("AsyncHttpClient shared transport create, config=", DescribeSnapshot(snapshot));
    return created;
}

}  // namespace VolcengineTos
