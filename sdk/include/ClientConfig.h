#pragma once
#include <cstddef>
#include <cstdint>

#include "AsyncTransportMode.h"
#include "common/Common.h"
#include <string>
namespace VolcengineTos {
class ClientConfig {
   public:
    ClientConfig()
        : autoRecognizeContentType(true),
          maxRetryCount(3),
          connectionTimeout(10000),
          requestTimeout(0),
          proxyHost(http::SchemeHTTP),
          proxyPort(-1),
          enableCRC(true),
          enableVerifySSL(true),
          dnsCacheTime(0),
          enableDnsIpBalancing(false),
          dnsCacheHostCapacity(1024),
          socketTimeout(30000),
          maxConnections(25),
          sslCtxCallback(nullptr),
          sslCtxCallbackUserData(nullptr),
          highLatencyLogThreshold(100) {}
    ~ClientConfig() = default;

    std::string endPoint;
    std::string controlEndPoint;
    bool autoRecognizeContentType;
    int maxRetryCount;
    int connectionTimeout;
    int requestTimeout;
    std::string proxyHost;
    int proxyPort;
    std::string proxyUsername;
    std::string proxyPassword;
    bool enableCRC;
    bool enableVerifySSL;
    int dnsCacheTime;
    bool enableDnsIpBalancing;
    int dnsCacheHostCapacity;
    int socketTimeout;
    int maxConnections;
    bool isCustomDomain = false;
    std::string caPath;
    std::string caFile;
    int highLatencyLogThreshold;
    std::string userAgentProductName;
    std::string userAgentSoftName;
    std::string userAgentSoftVersion;
    std::map<std::string, std::string> userAgentCustomizedKeyValues;
    std::string clientCrt;
    std::string clientKey;
    std::string netInterface;
    SslCtxCallback sslCtxCallback;
    void* sslCtxCallbackUserData;
    std::string sse;
    std::string sseKmsKeyId;
    bool tosStatFallback = false;
    bool enableDebug = false;

    int event_thread_count_ = 1;
    int max_request_queue_ = 0;
    bool connection_reuse_ = true;
    bool detail_log_ = true;
    int detail_log_interval_s_ = 60;
    int curl_multi_wait_timeout_ms_ = 10;
    AsyncTransportMode async_transport_mode_ = AsyncTransportMode::Isolated;

    std::string fileTransferBackend = "auto";
    std::size_t fileTransferChunkSize = 1024 * 1024;
    std::size_t fileTransferIoDepth = 8;
    std::uint64_t fileTransferAutoUploadIoUringMaxBytes = 0;

    // int MaxConnections;
    // int IdleConnectionTime;
};
}  // namespace VolcengineTos
