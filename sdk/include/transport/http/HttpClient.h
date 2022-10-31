#pragma once

#include <memory>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "curl/curl.h"
namespace VolcengineTos {
static bool hasInitHttpClient = false;
struct HttpConfig {
    int requestTimeout;
    int dialTimeout;
    int tcpKeepAlive;
    int connectTimeout;
    bool enableVerifySSL;
    std::string proxyHost;
    int proxyPort;
    std::string proxyUsername;
    std::string proxyPassword;
    int dnsCacheTime;
};
class HttpClient {
public:
    HttpClient() {
        if (!hasInitHttpClient) {
            initGlobalState();
            hasInitHttpClient = true;
        }
    }
    HttpClient(const HttpConfig& config);
    virtual ~HttpClient() {
        cleanupGlobalState();
        hasInitHttpClient = false;
    }

    static void initGlobalState();
    static void cleanupGlobalState();

    std::shared_ptr<HttpResponse> doRequest(const std::shared_ptr<HttpRequest>& request);

private:
    void setShareHandle(void* curl_handle, int cacheTime);
    void removeDNS(void* curl_handle, const std::shared_ptr<HttpRequest>& request);
    CURLSH* share_handle = nullptr;

private:
    int requestTimeout_ = 120000;
    int dialTimeout_;
    int tcpKeepAlive_;
    int connectTimeout_ = 10000;
    bool enableVerifySSL_ = true;
    std::string proxyHost_;
    int proxyPort_ = -1;
    std::string proxyUsername_;
    std::string proxyPassword_;
    int dnsCacheTime_ = 0;
    std::mutex mu_;
};
}  // namespace VolcengineTos
