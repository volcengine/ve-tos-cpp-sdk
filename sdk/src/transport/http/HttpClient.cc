#include <iostream>
#include "curl/curl.h"

#include "transport/http/HttpClient.h"
#include "common/Common.h"
#include "utils/BaseUtils.h"
#include "TosClient.h"
#include "utils/crc64.h"
#include "../../utils/LogUtils.h"

namespace VolcengineTos {
struct ResourceManager {
    HttpClient* client;
    CURL* curl;
    HttpRequest* httpReq;
    HttpResponse* httpResp;
    int64_t send;   // request body send
    int64_t total;  // request body content-length
    bool firstRecv;
    DataTransferStatusChange progress;
    void* userData;
    DataTransferType dataTransferType;
    bool enableCrc64;
    uint64_t sendCrc64Value;
    uint64_t recvCrc64Value;
    std::shared_ptr<RateLimiter> rateLimiter;
};

static void processHandler(const DataTransferStatusChange& handler, int64_t consumedBytes, int64_t totalBytes,
                           int64_t rwOnceBytes, DataTransferType type, void* userData) {
    DataTransferStatus dataTransferStatus{consumedBytes, totalBytes, rwOnceBytes, type, userData};
    auto data = std::make_shared<DataTransferStatus>(dataTransferStatus);
    handler(data);
}

static size_t sendBody(char* ptr, size_t size, size_t nmemb, void* data) {
    auto* resourceMan = static_cast<ResourceManager*>(data);

    if (resourceMan == nullptr || resourceMan->httpReq == nullptr) {
        resourceMan->dataTransferType = 4;
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        return 0;
    }
    std::shared_ptr<std::iostream>& content = resourceMan->httpReq->Body();
    const size_t wanted = size * nmemb;

    auto rateLimiter = resourceMan->rateLimiter;
    if (rateLimiter != nullptr) {
        auto rateLimiterRes = rateLimiter->Acquire(wanted);
        while (!rateLimiterRes.first) {
            TimeUtils::sleepMilliSecondTimes(rateLimiterRes.second);
            rateLimiterRes = rateLimiter->Acquire(wanted);
        }
    }

    // 第一次回调
    if (resourceMan->progress && resourceMan->dataTransferType == 1) {
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        resourceMan->dataTransferType = 2;
    }

    size_t got = 0;
    if (content != nullptr && wanted > 0) {
        size_t read = wanted;
        if (resourceMan->total > 0) {
            int64_t remains = resourceMan->total - resourceMan->send;
            if (remains < static_cast<int64_t>(wanted)) {
                read = static_cast<size_t>(remains);
            }
        }
        content->read(ptr, read);
        got = static_cast<size_t>(content->gcount());
    }

    resourceMan->send += got;
    if (resourceMan->progress) {
        // 数据发送完成
        if (resourceMan->total == resourceMan->send) {
            resourceMan->dataTransferType = 3;
        }
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, got, resourceMan->dataTransferType,
                       resourceMan->userData);
    }

    if (resourceMan->enableCrc64) {
        resourceMan->sendCrc64Value = CRC64::CalcCRC(resourceMan->sendCrc64Value, (void*)ptr, got);
    }
    return got;
}

static size_t recvBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* resourceMan = static_cast<ResourceManager*>(userdata);
    const size_t wanted = size * nmemb;

    // 第一次回调
    if (resourceMan->progress && resourceMan->dataTransferType == 1) {
        if (resourceMan->total == -1) {
            double dval;
            curl_easy_getinfo(resourceMan->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
            resourceMan->total = (int64_t)dval;
        }
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        resourceMan->dataTransferType = 2;
    }

    auto rateLimiter = resourceMan->rateLimiter;
    if (rateLimiter != nullptr) {
        auto rateLimiterRes = rateLimiter->Acquire(wanted);
        while (!rateLimiterRes.first) {
            TimeUtils::sleepMilliSecondTimes(rateLimiterRes.second);
            rateLimiterRes = rateLimiter->Acquire(wanted);
        }
    }

    if (resourceMan == nullptr || resourceMan->httpResp == nullptr || wanted == 0) {
        resourceMan->dataTransferType = 4;
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        return -1;
    }
    // 第一次receive response body , 初始化state->resposne->body
    // 如果200使用传入的iostream接收数据，反之生成一个stringstream接收错误信息
    if (resourceMan->firstRecv) {
        int64_t response_code = 0;
        curl_easy_getinfo(resourceMan->curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code / 100 == 2) {
            resourceMan->httpResp->setBody(resourceMan->httpReq->responseOutput());
        } else {
            resourceMan->httpResp->setBody(std::make_shared<std::stringstream>());
        }
        resourceMan->firstRecv = false;
    }
    std::shared_ptr<std::iostream>& content = resourceMan->httpResp->Body();
    if (content == nullptr || content->fail()) {
        resourceMan->dataTransferType = 4;
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        return -2;
    }
    content->write(ptr, static_cast<std::streamsize>(wanted));
    if (content->bad()) {
        resourceMan->dataTransferType = 4;
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0, resourceMan->dataTransferType,
                       resourceMan->userData);
        return -3;
    }

    resourceMan->send += wanted;
    if (resourceMan->progress) {
        if (resourceMan->total == resourceMan->send) {
            resourceMan->dataTransferType = 3;
        }
        processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, wanted,
                       resourceMan->dataTransferType, resourceMan->userData);
    }

    if (resourceMan->enableCrc64) {
        resourceMan->recvCrc64Value = CRC64::CalcCRC(resourceMan->recvCrc64Value, (void*)ptr, wanted);
    }

    return wanted;
}

static size_t recvHeaders(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* resourceMan = static_cast<ResourceManager*>(userdata);
    const size_t length = nitems * size;

    std::string line(buffer);
    auto pos = line.find(':');
    if (pos != std::string::npos) {
        size_t posEnd = line.rfind('\r');
        if (posEnd != std::string::npos) {
            posEnd = posEnd - pos - 2;
        }
        std::string name = line.substr(0, pos);
        std::string value = line.substr(pos + 2, posEnd);
        resourceMan->httpResp->setHeader(name, value);
    }
    if (length == 2 && (buffer[0] == 0x0D) && (buffer[1] == 0x0A)) {
        if (resourceMan->httpResp->hasHeader(http::HEADER_CONTENT_LENGTH)) {
            double dval;
            curl_easy_getinfo(resourceMan->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
            resourceMan->total = (int64_t)dval;
        }
    }
    return length;
}

}  // namespace VolcengineTos

using namespace VolcengineTos;

void HttpClient::initGlobalState() {
    // init twice here, check why
    curl_global_init(CURL_GLOBAL_ALL);
}

void HttpClient::cleanupGlobalState() {
    curl_global_cleanup();
}

HttpClient::HttpClient(const HttpConfig& config) {
    if (!hasInitHttpClient) {
        initGlobalState();
        hasInitHttpClient = true;
    }
    tcpKeepAlive_ = config.tcpKeepAlive;
    dialTimeout_ = config.dialTimeout;
    requestTimeout_ = config.requestTimeout;
    connectTimeout_ = config.connectTimeout;
    enableVerifySSL_ = config.enableVerifySSL;
    proxyHost_ = config.proxyHost;
    proxyPort_ = config.proxyPort;
    proxyUsername_ = config.proxyUsername;
    proxyPassword_ = config.proxyPassword;
    dnsCacheTime_ = config.dnsCacheTime;
}
void HttpClient::setShareHandle(CURL* curl_handle, int cacheTime) {
    std::lock_guard<std::mutex> lock(mu_);
    if (!share_handle) {
        share_handle = curl_share_init();
        // 共享 DNS 信息，带锁
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    }
    // 当前 curl handler 使用共享 handle 的数据
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);
    // 在内存中保存DNS信息的时间
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, cacheTime * 60);
}

void HttpClient::removeDNS(void* curl, const std::shared_ptr<HttpRequest>& request) {
    // 无法感知 IP，超时时将会直接踢出该 host 对应的 DNS 映射信息
    curl_slist* dns_list = nullptr;
    auto servicehost = request->url().host();
    std::string port = request->url().scheme() == "http" ? "80" : "443";
    auto rmHost = "-" + servicehost + ":" + port;
    dns_list = curl_slist_append(dns_list, rmHost.c_str());
    std::lock_guard<std::mutex> lock(mu_);
    curl_easy_setopt(curl, CURLOPT_RESOLVE, dns_list);
}

std::shared_ptr<HttpResponse> HttpClient::doRequest(const std::shared_ptr<HttpRequest>& request) {
    // init curl for this request
    CURL* curl = curl_easy_init();

    /* enable TCP keep-alive for this request */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    /* keep-alive idle time to 120 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    /* interval time between keep-alive probes: 60 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
    curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout_);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, requestTimeout_);

    if (proxyPort_ != -1 && !proxyHost_.empty()) {
        std::string proxy = proxyHost_ + ":" + std::to_string(proxyPort_);
        std::string proxyUserPwd = proxyUsername_ + ":" + proxyPassword_;
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyUserPwd.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    }

    // 支持忽略SSL证书校验
    if (!enableVerifySSL_) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    }
    // set req specific params
    auto response = std::make_shared<HttpResponse>();
    auto processHandler = request->getDataTransferListener().dataTransferStatusChange_;
    auto userData = request->getDataTransferListener().userData_ == nullptr
                            ? nullptr
                            : request->getDataTransferListener().userData_;

    auto rateLimiter = request->getRateLimiter();
    bool checkCrc64 = request->isCheckCrc64();
    uint64_t initCRC64 = request->getPreHashCrc64Ecma();
    ResourceManager resourceMan = {this,       curl,      request.get(),  response.get(), 0,
                                   -1,         true,      processHandler, userData,       1,
                                   checkCrc64, initCRC64, initCRC64,      rateLimiter};

    resourceMan.total = request->getContentLength();

    curl_easy_setopt(curl, CURLOPT_URL, request->url().toString().c_str());

    // set opt for different http methods
    if (request->method() == http::MethodHead) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    } else if (request->method() == http::MethodPut) {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        // make sure httpRequest content length has been set
        curl_off_t bodySize = request->getContentLength();
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodySize);
    } else if (request->method() == http::MethodPost) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request->getContentLength());
        curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 0L);
    } else if (request->method() == http::MethodDelete) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    // add headers
    curl_slist* list = nullptr;

    auto& headers = request->Headers();
    for (const auto& p : headers) {
        if (p.second.empty())
            continue;
        std::string str(p.first);
        str.append(":").append(p.second);
        list = curl_slist_append(list, str.c_str());
    }

    // Disable Expect: 100-continue
    list = curl_slist_append(list, "Expect:");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    // add user-agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, VolcengineTos::DefaultUserAgent().c_str());

    // set call back func
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resourceMan);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeaders);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resourceMan);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBody);

    curl_easy_setopt(curl, CURLOPT_READDATA, &resourceMan);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBody);

    // 使用缓存 dns
    if (dnsCacheTime_ > 0) {
        setShareHandle(curl, dnsCacheTime_);
    }
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_COULDNT_CONNECT) {
        response->setStatus(http::Refused);
        response->setStatusMsg("connection refused");
        if (dnsCacheTime_ > 0) {
            removeDNS(curl, request);
        }

    } else if (res == CURLE_OPERATION_TIMEDOUT) {
        response->setStatus(http::otherErr);
        response->setStatusMsg("operation timeout");
        if (dnsCacheTime_ > 0) {
            removeDNS(curl, request);
        }
    } else if (res != CURLE_OK) {
        response->setStatus(http::otherErr);
        response->setStatusMsg("curl error code is " + std::to_string(res));
    } else {
        response->setStatus(http::Success);

#ifdef CURL_VERSION_7610
        long nameLookUp = 0;
        long connectTime = 0;
        long tlsConnect = 0;
        long startTrans = 0;
        long totalTime = 0;
        curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME_T, &nameLookUp);
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &connectTime);
        curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME_T, &tlsConnect);
        curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME_T, &startTrans);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &totalTime);
        auto logger = LogUtils::GetLogger();
        if (logger != nullptr) {
            logger->debug(
                    "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request time:{} ms",
                    request->method(), request->url().host(), request->url().path(), nameLookUp / 1000,
                    connectTime / 1000, tlsConnect / 1000, startTrans / 1000, (totalTime - startTrans) / 1000,
                    totalTime / 1000);
        }
#else
        double nameLookUp = 0;
        double connectTime = 0;
        double tlsConnect = 0;
        double startTrans = 0;
        double totalTime = 0;
        curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &nameLookUp);
        curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connectTime);
        curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &tlsConnect);
        curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &startTrans);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
        auto logger = LogUtils::GetLogger();
        if (logger != nullptr) {
            logger->debug(
                    "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request time:{} ms",
                    request->method(), request->url().host(), request->url().path(), (long)(nameLookUp * 1000),
                    (long)(connectTime * 1000), (long)(tlsConnect * 1000), (long)(startTrans * 1000),
                    (long)((totalTime - startTrans) * 1000), (long)(totalTime * 1000));
        }
#endif
    }
    int response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response->status() == http::Refused) {
        response_code = -1;
    }
    if (response->status() == http::otherErr) {
        response_code = -2;
    }
    response->setStatusCode(response_code);

    auto method_ = request->method();
    if (resourceMan.sendCrc64Value == 0 && resourceMan.recvCrc64Value == 0) {
    } else if (request->method() == http::MethodPost || request->method() == http::MethodPut) {
        response->setHashCrc64Result(resourceMan.sendCrc64Value);
    } else {
        response->setHashCrc64Result(resourceMan.recvCrc64Value);
    }

    request->setTransferedBytes(resourceMan.send);
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    return response;
}
