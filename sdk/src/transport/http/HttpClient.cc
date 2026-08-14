#include <iostream>
#include <set>
#include <utility>
#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#endif
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

namespace {
std::atomic<uint64_t> g_dnsCacheStartOffset{0};

std::string defaultPortForScheme(const std::string& scheme) {
    return scheme == "http" ? "80" : "443";
}

bool isDigits(const std::string& value) {
    return !value.empty() &&
           std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

std::string formatResolveAddress(const std::string& ip) {
    if (ip.find(':') != std::string::npos && (ip.front() != '[' || ip.back() != ']')) {
        return "[" + ip + "]";
    }
    return ip;
}

bool shouldEvictResolvedIp(CURLcode res) {
    return res == CURLE_COULDNT_CONNECT || res == CURLE_OPERATION_TIMEDOUT;
}

std::pair<std::string, std::string> splitHostAndPort(const Url& url) {
    std::string host = url.host();
    std::string port = url.port();
    if (!port.empty()) {
        return {host, port};
    }
    auto pos = host.rfind(':');
    if (pos != std::string::npos && host.find(':') == pos) {
        auto candidatePort = host.substr(pos + 1);
        if (isDigits(candidatePort)) {
            return {host.substr(0, pos), candidatePort};
        }
    }
    return {host, defaultPortForScheme(url.scheme())};
}

#ifdef _WIN32
std::vector<std::string> defaultDnsLookup(const std::string& host) {
    (void)host;
    return {};
}
#else
std::vector<std::string> defaultDnsLookup(const std::string& host) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;
    addrinfo* result = nullptr;
    std::vector<std::string> ipList;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0 || result == nullptr) {
        return ipList;
    }

    std::set<std::string> uniq;
    for (addrinfo* current = result; current != nullptr; current = current->ai_next) {
        char ipBuffer[INET6_ADDRSTRLEN] = {0};
        void* addr = nullptr;
        if (current->ai_family == AF_INET) {
            addr = &reinterpret_cast<sockaddr_in*>(current->ai_addr)->sin_addr;
        } else if (current->ai_family == AF_INET6) {
            addr = &reinterpret_cast<sockaddr_in6*>(current->ai_addr)->sin6_addr;
        }
        if (addr == nullptr) {
            continue;
        }
        if (inet_ntop(current->ai_family, addr, ipBuffer, sizeof(ipBuffer)) != nullptr) {
            uniq.insert(ipBuffer);
        }
    }
    freeaddrinfo(result);

    ipList.assign(uniq.begin(), uniq.end());
    return ipList;
}
#endif
}  // namespace

bool HostIpCache::isExpired(const CacheEntry& entry) const {
    return entry.expireAt <= std::chrono::steady_clock::now();
}

void HostIpCache::eraseUnlocked(const std::string& host) {
    auto it = data_.find(host);
    if (it == data_.end()) {
        return;
    }
    order_.erase(it->second.orderIt);
    data_.erase(it);
}

void HostIpCache::touchUnlocked(CacheEntry& entry, const std::string& host) {
    order_.erase(entry.orderIt);
    order_.push_back(host);
    entry.orderIt = std::prev(order_.end());
}

void HostIpCache::trimToCapacityUnlocked() {
    if (maxHosts_ == 0) {
        data_.clear();
        order_.clear();
        return;
    }
    while (data_.size() > maxHosts_ && !order_.empty()) {
        auto evictHost = order_.front();
        eraseUnlocked(evictHost);
    }
}

std::vector<std::string> HostIpCache::Get(const std::string& host) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = data_.find(host);
    if (it == data_.end()) {
        return {};
    }
    if (isExpired(it->second)) {
        eraseUnlocked(host);
        return {};
    }
    touchUnlocked(it->second, host);
    return it->second.ipList;
}

void HostIpCache::Put(const std::string& host, const std::vector<std::string>& ipList,
                      const std::chrono::steady_clock::time_point& expireAt) {
    if (host.empty() || ipList.empty()) {
        ClearHost(host);
        return;
    }

    std::lock_guard<std::mutex> lock(mu_);
    eraseUnlocked(host);
    CacheEntry entry;
    entry.ipList = ipList;
    entry.expireAt = expireAt;
    entry.nextIndex = static_cast<size_t>(g_dnsCacheStartOffset.fetch_add(1, std::memory_order_relaxed) %
                                          entry.ipList.size());
    order_.push_back(host);
    entry.orderIt = std::prev(order_.end());
    data_[host] = std::move(entry);
    trimToCapacityUnlocked();
}

std::string HostIpCache::Select(const std::string& host) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = data_.find(host);
    if (it == data_.end()) {
        return "";
    }
    if (isExpired(it->second)) {
        eraseUnlocked(host);
        return "";
    }
    if (it->second.ipList.empty()) {
        eraseUnlocked(host);
        return "";
    }
    auto& entry = it->second;
    touchUnlocked(entry, host);
    auto selected = entry.ipList[entry.nextIndex % entry.ipList.size()];
    entry.nextIndex = (entry.nextIndex + 1) % entry.ipList.size();
    return selected;
}

bool HostIpCache::RemoveIp(const std::string& host, const std::string& ip) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = data_.find(host);
    if (it == data_.end()) {
        return false;
    }
    auto& ipList = it->second.ipList;
    auto removeIt = std::remove(ipList.begin(), ipList.end(), ip);
    if (removeIt == ipList.end()) {
        return false;
    }
    ipList.erase(removeIt, ipList.end());
    if (ipList.empty()) {
        eraseUnlocked(host);
        return true;
    }
    touchUnlocked(it->second, host);
    if (it->second.nextIndex >= ipList.size()) {
        it->second.nextIndex %= ipList.size();
    }
    return true;
}

void HostIpCache::ClearHost(const std::string& host) {
    std::lock_guard<std::mutex> lock(mu_);
    eraseUnlocked(host);
}

size_t HostIpCache::Size() const {
    std::lock_guard<std::mutex> lock(mu_);
    return data_.size();
}

static bool isOpenSslCompatibleBackend() {
    auto* versionInfo = curl_version_info(CURLVERSION_NOW);
    if (versionInfo == nullptr || versionInfo->ssl_version == nullptr) {
        return false;
    }
    std::string sslVersion(versionInfo->ssl_version);
    return sslVersion.find("OpenSSL") != std::string::npos || sslVersion.find("BoringSSL") != std::string::npos ||
           sslVersion.find("LibreSSL") != std::string::npos;
}

static CURLcode curlSslCtxCallbackBridge(CURL* curl, void* sslCtx, void* userData) {
    auto* client = static_cast<HttpClient*>(userData);
    if (client == nullptr) {
        return CURLE_OK;
    }
    (void)curl;
    return client->invokeSslCtxCallback(sslCtx);
}

static void processHandler(const DataTransferStatusChange& handler, int64_t consumedBytes, int64_t totalBytes,
                           int64_t rwOnceBytes, DataTransferType type, void* userData) {
    if (!handler) {
        return;
    }
    DataTransferStatus dataTransferStatus{consumedBytes, totalBytes, rwOnceBytes, type, userData};
    auto data = std::make_shared<DataTransferStatus>(dataTransferStatus);
    handler(data);
}

static size_t sendBody(char* ptr, size_t size, size_t nmemb, void* data) {
    auto* resourceMan = static_cast<ResourceManager*>(data);

    if (resourceMan == nullptr || resourceMan->httpReq == nullptr) {
        if (resourceMan != nullptr) {
            resourceMan->dataTransferType = 4;
            processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0,
                           resourceMan->dataTransferType, resourceMan->userData);
        }

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

    if (got > 0 && resourceMan->enableCrc64) {
        resourceMan->sendCrc64Value = CRC64::CalcCRC(resourceMan->sendCrc64Value, (void*)ptr, got);
    }
    return got;
}

static size_t recvBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* resourceMan = static_cast<ResourceManager*>(userdata);
    const size_t wanted = size * nmemb;

    auto logger = LogUtils::GetLogger();

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
        if (resourceMan != nullptr) {
            resourceMan->dataTransferType = 4;
            processHandler(resourceMan->progress, resourceMan->send, resourceMan->total, 0,
                           resourceMan->dataTransferType, resourceMan->userData);
        }

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
        if (logger != nullptr) {
            logger->error("recvBody content init err");
        }
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
        if (logger != nullptr) {
            logger->error("recvBody content write err, please check disk space, file permission, network, etc.");
        }
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

void HttpClient::cleanupGlobalState() {}

#ifdef _WIN32
size_t acquire_lock(void* clientp, curl_lock_data data, curl_lock_access access, void* userp) {
    auto tt = curlShareLock;
    EnterCriticalSection(&curlShareLock);
    return 0;
}
void release_lock(void* clientp, curl_lock_data data, void* userp) { LeaveCriticalSection(&curlShareLock); }
#else
void acquire_lock(CURL* handle, curl_lock_data data, curl_lock_access access, void* userptr) {
    pthread_mutex_lock(&curlShareLock);
}
void release_lock(CURL* handle, curl_lock_data data, void* userptr) { pthread_mutex_unlock(&curlShareLock); }
#endif

HttpClient::HttpClient() : dnsLookupCallback_(defaultDnsLookup) {
    curlContainer_ = new CurlContainer(25, 12000, 10000);
    if (dnsCacheTime_ > 0) {
        share_handle = curl_share_init();
        // 共享 DNS 信息，带锁
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, acquire_lock);
        curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, release_lock);
    }
}

HttpClient::HttpClient(const HttpConfig& config)
        : dnsIpCache_(config.dnsCacheHostCapacity > 0 ? static_cast<size_t>(config.dnsCacheHostCapacity) : 1024),
          dnsLookupCallback_(config.dnsLookupCallback ? config.dnsLookupCallback : defaultDnsLookup) {
    curlContainer_ = new CurlContainer(config.maxConnections, config.socketTimeout, config.connectTimeout);
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
    enableDnsIpBalancing_ = config.enableDnsIpBalancing && dnsCacheTime_ > 0;
    caPath_ = config.caPath;
    caFile_ = config.caFile;
    clientCrt_ = config.clientCrt_;
    clientKey_ = config.clientKey_;
    netInterface_ = config.netInterface_;
    sslCtxCallback_ = config.sslCtxCallback;
    sslCtxCallbackUserData_ = config.sslCtxCallbackUserData;
    highLatencyLogThreshold_ = config.highLatencyLogThreshold;
    if (dnsCacheTime_ > 0) {
        share_handle = curl_share_init();
        // 共享 DNS 信息，带锁
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(share_handle, CURLSHOPT_LOCKFUNC, acquire_lock);
        curl_share_setopt(share_handle, CURLSHOPT_UNLOCKFUNC, release_lock);
    }
}

CURLcode HttpClient::invokeSslCtxCallback(void* sslCtx) const {
    if (sslCtxCallback_ == nullptr) {
        return CURLE_OK;
    }
    int ret = sslCtxCallback_(sslCtx, sslCtxCallbackUserData_);
    return ret == 0 ? CURLE_OK : CURLE_SSL_CERTPROBLEM;
}

void HttpClient::setShareHandle(CURL* curl_handle, int cacheTime) {
    // 当前 curl handler 使用共享 handle 的数据
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);
    // 在内存中保存DNS信息的时间
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, cacheTime * 60);
}

void HttpClient::removeDNS(void* curl, const std::shared_ptr<HttpRequest>& request) {
    // 无法感知 IP 时，直接踢出该 host 对应的 curl DNS 映射信息，并清理 SDK 内缓存。
    curl_slist* dns_list = nullptr;
    auto hostAndPort = splitHostAndPort(request->url());
    auto servicehost = hostAndPort.first;
    auto port = hostAndPort.second;
    auto rmHost = "-" + servicehost + ":" + port;
    dns_list = curl_slist_append(dns_list, rmHost.c_str());
    std::lock_guard<std::mutex> lock(mu_);
    curl_easy_setopt(curl, CURLOPT_RESOLVE, dns_list);
    dnsIpCache_.ClearHost(servicehost);
    curl_slist_free_all(dns_list);
}

std::vector<std::string> HttpClient::lookupHostIpList(const std::string& host) const {
    if (!dnsLookupCallback_ || host.empty()) {
        return {};
    }
    return dnsLookupCallback_(host);
}

std::vector<std::string> HttpClient::getHostIpList(const std::string& host) {
    auto ipList = dnsIpCache_.Get(host);
    if (!ipList.empty()) {
        return ipList;
    }

    ipList = lookupHostIpList(host);
    if (!ipList.empty()) {
        dnsIpCache_.Put(host, ipList, std::chrono::steady_clock::now() + std::chrono::minutes(dnsCacheTime_));
        auto logger = LogUtils::GetLogger();
        if (logger != nullptr) {
            logger->debug("Refreshed SDK DNS cache, host:{}, ip_count:{}", host, ipList.size());
        }
    }
    return ipList;
}

ResolveBinding HttpClient::buildResolveBinding(const std::shared_ptr<HttpRequest>& request) {
    ResolveBinding binding;
    if (!enableDnsIpBalancing_ || dnsCacheTime_ <= 0 || request == nullptr) {
        return binding;
    }

    auto hostAndPort = splitHostAndPort(request->url());
    binding.host = hostAndPort.first;
    binding.port = hostAndPort.second;
    if (binding.host.empty() || !NetUtils::isNotIP(binding.host)) {
        return ResolveBinding{};
    }

    auto ipList = getHostIpList(binding.host);
    if (ipList.empty()) {
        return ResolveBinding{};
    }

    binding.selectedIp = dnsIpCache_.Select(binding.host);
    if (binding.selectedIp.empty()) {
        return ResolveBinding{};
    }

    auto resolveEntry = binding.host + ":" + binding.port + ":" + formatResolveAddress(binding.selectedIp);
    binding.resolveList = curl_slist_append(nullptr, resolveEntry.c_str());
    auto logger = LogUtils::GetLogger();
    if (logger != nullptr) {
        logger->debug("Selected SDK DNS IP, host:{}, ip:{}, port:{}", binding.host, binding.selectedIp, binding.port);
    }
    return binding;
}

void HttpClient::releaseResolveBinding(ResolveBinding& binding) const {
    if (binding.resolveList != nullptr) {
        curl_slist_free_all(binding.resolveList);
        binding.resolveList = nullptr;
    }
}

void HttpClient::removeFailedIp(const ResolveBinding& binding) {
    if (!binding.active()) {
        return;
    }
    if (dnsIpCache_.RemoveIp(binding.host, binding.selectedIp)) {
        auto logger = LogUtils::GetLogger();
        if (logger != nullptr) {
            logger->info("Evicted failed SDK DNS IP, host:{}, ip:{}", binding.host, binding.selectedIp);
        }
    }
}

std::shared_ptr<HttpResponse> HttpClient::doRequest(const std::shared_ptr<HttpRequest>& request) {
    ResolveBinding resolveBinding;
    if (enableDnsIpBalancing_) {
        resolveBinding = buildResolveBinding(request);
    }

    // init curl for this request
    CURL* curl = curlContainer_->Acquire();
    auto response = std::make_shared<HttpResponse>();
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
        // 当用户设置了 sslCtxCallback 但没有配置 caFile/caPath 时，禁用默认验证，
        // 让用户的回调函数完全控制证书验证逻辑。
        if (sslCtxCallback_ != nullptr && caFile_.empty() && caPath_.empty()) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        } else {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        }
    }
    if (!caPath_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, caPath_.c_str());
    }
    if (!caFile_.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, caFile_.c_str());
    }
    if (!clientCrt_.empty()) {
        curl_easy_setopt(curl, CURLOPT_SSLCERT, clientCrt_.c_str());
    }
    if (!clientKey_.empty()) {
        curl_easy_setopt(curl, CURLOPT_SSLKEY, clientKey_.c_str());
    }
    if (sslCtxCallback_ != nullptr) {
        if (!isOpenSslCompatibleBackend()) {
            response->setStatus(http::otherErr);
            response->setStatusCode(http::otherErr);
            response->setStatusMsg(std::string("curlCode: ") + std::to_string(CURLE_NOT_BUILT_IN) +
                                   ", ssl ctx callback requires OpenSSL-compatible libcurl backend");
            response->setCurlErrCode(CURLE_NOT_BUILT_IN);
            curlContainer_->Release(curl, false);
            releaseResolveBinding(resolveBinding);
            return response;
        }
        auto res = curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, curlSslCtxCallbackBridge);
        if (res == CURLE_OK) {
            res = curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, this);
        }
        if (res != CURLE_OK) {
            response->setStatus(http::otherErr);
            response->setStatusCode(http::otherErr);
            response->setStatusMsg(std::string("curlCode: ") + std::to_string(res) + ", " + curl_easy_strerror(res));
            response->setCurlErrCode(res);
            curlContainer_->Release(curl, false);
            releaseResolveBinding(resolveBinding);
            return response;
        }
    }
    if (!netInterface_.empty()) {
        curl_easy_setopt(curl, CURLOPT_INTERFACE, netInterface_.c_str());
    }
    // set req specific params
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
        if (p.second.empty()) continue;
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
        if (resolveBinding.active()) {
            curl_easy_setopt(curl, CURLOPT_RESOLVE, resolveBinding.resolveList);
        }
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
    char* primaryIp = nullptr;
    curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &nameLookUp);
    curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connectTime);
    curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &tlsConnect);
    curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &startTrans);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
    curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &primaryIp);
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
                "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, "
                "TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request "
                "time:{} ms, Selected IP:{}, Primary IP:{}",
                request->method(), request->url().host(), request->url().path(), (long)(nameLookUp * 1000),
                (long)(connectTime * 1000), (long)(tlsConnect * 1000), (long)(startTrans * 1000),
                (long)((totalTime - startTrans) * 1000), (long)(totalTime * 1000), resolveBinding.selectedIp,
                primaryIp == nullptr ? "" : primaryIp);
        } else {
            logger->debug(
                "Method:{}, Host:{}, request uri:{}, DNS resolution time:{} ms, TCP establish connection time:{} ms, "
                "TLS handshake time:{} ms, start transfer time:{} ms, Data sending time:{} ms, Total HTTP request "
                "time:{} ms, Selected IP:{}, Primary IP:{}",
                request->method(), request->url().host(), request->url().path(), (long)(nameLookUp * 1000),
                (long)(connectTime * 1000), (long)(tlsConnect * 1000), (long)(startTrans * 1000),
                (long)((totalTime - startTrans) * 1000), (long)(totalTime * 1000), resolveBinding.selectedIp,
                primaryIp == nullptr ? "" : primaryIp);
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
        ss << "Total HTTP request time:" << (long)(totalTime * 1000) << " ms, ";
        ss << "Selected IP:" << resolveBinding.selectedIp << ", ";
        ss << "Primary IP:" << (primaryIp == nullptr ? "" : primaryIp);
        std::cout << ss.str() << std::endl;
    }

    if (res != CURLE_OK && dnsCacheTime_ > 0) {
        if (resolveBinding.active()) {
            if (shouldEvictResolvedIp(res)) {
                removeFailedIp(resolveBinding);
            }
        } else {
            removeDNS(curl, request);
        }
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
    releaseResolveBinding(resolveBinding);
    curl_slist_free_all(list);
    return response;
}
