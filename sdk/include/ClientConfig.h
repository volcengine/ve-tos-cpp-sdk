#pragma once
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
              socketTimeout(30000),
              maxConnections(25),
              highLatencyLogThreshold(100) {
    }
    ~ClientConfig() = default;

    std::string endPoint;
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
    int socketTimeout;
    int maxConnections;
    bool isCustomDomain = false;
    std::string caPath;
    std::string caFile;
    int highLatencyLogThreshold;

    // int MaxConnections;
    // int IdleConnectionTime;
};
}  // namespace VolcengineTos
