#pragma once
#include "common/Common.h"
#include <string>

namespace VolcengineTos {
class ClientConfig {
public:
    ClientConfig()
            : endPoint(""),
              autoRecognizeContentType(true),
              maxRetryCount(3),
              connectionTimeout(10000),
              requestTimeout(120000),
              proxyHost(http::SchemeHTTP),
              proxyPort(-1),
              proxyPassword(""),
              proxyUsername(""),
              enableCRC(true),
              enableVerifySSL(true),
              dnsCacheTime(0) {
    }
    ~ClientConfig() = default;

    std::string endPoint;
    bool autoRecognizeContentType;
    bool enableCRC;
    int maxRetryCount;
    int connectionTimeout;
    int requestTimeout;
    std::string proxyHost;
    int proxyPort;
    std::string proxyUsername;
    std::string proxyPassword;
    // int MaxConnections;
    // int IdleConnectionTime;
    bool enableVerifySSL;
    int dnsCacheTime;
};
}  // namespace VolcengineTos
