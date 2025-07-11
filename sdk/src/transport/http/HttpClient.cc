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
    //    std::shared_ptr<DataConsumeCallBack> callBack;
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
        long response_code = 0;
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
//    if (resourceMan->callBack != nullptr) {
//        resourceMan->callBack->Consume(wanted);
//    }

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
}

void HttpClient::cleanupGlobalState() {
}

#ifdef _WIN32
size_t acquire_lock(void* clientp, curl_lock_data data, curl_lock_access access, void* userp) {
    auto tt = curlShareLock;
    EnterCriticalSection(&curlShareLock);
    return 0;
}
void release_lock(void* clientp, curl_lock_data data, void* userp) {
    LeaveCriticalSection(&curlShareLock);
}
#else
void acquire_lock(CURL* handle, curl_lock_data data, curl_lock_access access, void* userptr) {
    pthread_mutex_lock(&curlShareLock);
}
void release_lock(CURL* handle, curl_lock_data data, void* userptr) {
    pthread_mutex_unlock(&curlShareLock);
}
#endif

HttpClient::HttpClient() {
    curlContainer_ = new CurlContainer(25, 12000, 10000);
    if (dnsCacheTime_ > 0) {
        share_handle = curl_share_init();
        // 共享 DNS 信息，带锁
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, acquire_lock);
        curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, release_lock);
    }
}

HttpClient::HttpClient(const HttpConfig& config) {
    curlContainer_ = new CurlContainer(config.maxConnections,config.socketTimeout,config.connectTimeout);
    tcpKeepAlive_ = config.tcpKeepAlive;
    dialTimeout_ = config.dialTimeout;
    requestTimeout_ = config.requestTimeout;
    connectTimeout_ = config.connectTimeout;
    enableVerifySSL_ = config.enableVerifySSL;
    proxyHost_ = config.proxyHost;
    proxyPort_ = config.proxyPort;
    proxyUsername_ = config.proxyUsername;
    proxyPassword_ = config.proxyPassword;
#ifdef _WIN32
    dnsCacheTime_ = 0;
#else
    dnsCacheTime_ = config.dnsCacheTime;
#endif
    caPath_ = config.caPath;
    caFile_ = config.caFile;
    clientCrt_ = config.clientCrt_;
    clientKey_ = config.clientKey_;
    highLatencyLogThreshold_ = config.highLatencyLogThreshold;
    if (dnsCacheTime_ > 0) {
        share_handle = curl_share_init();
        // 共享 DNS 信息，带锁
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, acquire_lock);
        curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, release_lock);
    }
}

void HttpClient::setShareHandle(CURL* curl_handle, int cacheTime) {
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
    CURL * curl = curlContainer_->Acquire();
    if (requestTimeout_ != 0) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, requestTimeout_);
    }


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
    if(!caPath_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, caPath_.c_str());
    }
    if(!caFile_.empty()){
        curl_easy_setopt(curl, CURLOPT_CAINFO, caFile_.c_str());
    }
    if(!clientCrt_.empty()){
        curl_easy_setopt(curl, CURLOPT_SSLCERT, clientCrt_.c_str());
    }
    if(!clientKey_.empty()){
        curl_easy_setopt(curl, CURLOPT_SSLKEY, clientKey_.c_str());
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
    if (request->method() == http::MethodGet) {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if (request->method() == http::MethodHead) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
    } else if (request->method() == http::MethodPut) {
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
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
        std::stringstream ss;
        ss << "curlCode: " << res << ", " << curl_easy_strerror(res);
        response->setStatusMsg(ss.str());
        response->setCurlErrCode(res);
    } else if (res != CURLE_OK) {
        response->setStatus(http::otherErr);
        std::stringstream ss;
        ss << "curlCode: " << res << ", " << curl_easy_strerror(res);
        response->setStatusMsg(ss.str());
        response->setCurlErrCode(res);
    } else {
        response->setStatus(http::Success);
    }

    double nameLookUp = 0;
    double connectTime = 0;
    double tlsConnect = 0;
    double startTrans = 0;
    double totalTime = 0;
    double speed = 0;
    bool isHighLatencyReq = false;
    curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &nameLookUp);
    curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connectTime);
    curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &tlsConnect);
    curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &startTrans);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
    if (request->method() == http::MethodPut || request->method() == http::MethodPost) {
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed);
    } else if (request->method() == http::MethodGet) {
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
    }

    if (request->isCheckHighLatency() && speed < highLatencyLogThreshold_ * 1024 && totalTime * 1000 > 500) {
        isHighLatencyReq = true;
        response->setIsHighLatencyReq(isHighLatencyReq);
    }
    auto logger = LogUtils::GetLogger();

    if (logger != nullptr) {
        auto logger = LogUtils::GetLogger();
        if (isHighLatencyReq) {
            logger->warn(
                    "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request time:{} ms",
                    request->method(), request->url().host(), request->url().path(), (long)(nameLookUp * 1000),
                    (long)(connectTime * 1000), (long)(tlsConnect * 1000), (long)(startTrans * 1000),
                    (long)((totalTime - startTrans) * 1000), (long)(totalTime * 1000));
        } else {
            logger->debug(
                    "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request time:{} ms",
                    request->method(), request->url().host(), request->url().path(), (long)(nameLookUp * 1000),
                    (long)(connectTime * 1000), (long)(tlsConnect * 1000), (long)(startTrans * 1000),
                    (long)((totalTime - startTrans) * 1000), (long)(totalTime * 1000));
        }
    } else if (isHighLatencyReq) {
        std::ostringstream ss;
        ss << "Method:" << request->method() << ", ";
        ss << "Host:" << request->url().host() << ", ";
        ss << "request uri:" << request->url().path() << ", ";
        ss << "DNS resolution time:" << (long)(nameLookUp * 1000) << " ms, ";
        ss << "TCP establish connection time:" << (long)(connectTime * 1000) << " ms, ";
        ss << "TLS handshake time:" << (long)(tlsConnect * 1000) << " ms, ";
        ss << "start transfer time:" << (long)(startTrans * 1000) << " ms, ";
        ss << "Data sending time:" << (long)((totalTime - startTrans) * 1000) << " ms, ";
        ss << "Total HTTP request time:" << (long)(totalTime * 1000) << " ms";
        std::cout << ss.str() << std::endl;
    }

    if (res != CURLE_OK && dnsCacheTime_ > 0) {
        removeDNS(curl, request);
    }
    long response_code = 0;
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
    curlContainer_->Release(curl, (res != CURLE_OK));
    curl_slist_free_all(list);
    return response;
}
