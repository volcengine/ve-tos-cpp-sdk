#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "curl/curl.h"

namespace VolcengineTos {
#ifndef VOLCENGINE_TOS_SSL_CTX_CALLBACK_DEFINED
#define VOLCENGINE_TOS_SSL_CTX_CALLBACK_DEFINED
using SslCtxCallback = int (*)(void* sslCtx, void* userData);
#endif

static bool hasInitHttpClient = false;
#ifdef _WIN32
static CRITICAL_SECTION curlShareLock;
#else
static pthread_mutex_t curlShareLock;
#endif
struct HttpConfig {
    int maxConnections;
    int socketTimeout;
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
    bool enableDnsIpBalancing = false;
    int dnsCacheHostCapacity = 1024;
    std::string caPath;
    std::string caFile;
    int highLatencyLogThreshold;
    std::string clientCrt_;
    std::string clientKey_;
    std::string netInterface_;
    SslCtxCallback sslCtxCallback;
    void* sslCtxCallbackUserData;
    std::function<std::vector<std::string>(const std::string&)> dnsLookupCallback;
};

class HostIpCache {
   public:
    explicit HostIpCache(size_t maxHosts = 1024) : maxHosts_(maxHosts) {}

    std::vector<std::string> Get(const std::string& host);
    void Put(const std::string& host, const std::vector<std::string>& ipList,
             const std::chrono::steady_clock::time_point& expireAt);
    std::string Select(const std::string& host);
    bool RemoveIp(const std::string& host, const std::string& ip);
    void ClearHost(const std::string& host);
    size_t Size() const;

   private:
    struct CacheEntry {
        std::vector<std::string> ipList;
        std::chrono::steady_clock::time_point expireAt;
        size_t nextIndex = 0;
        std::list<std::string>::iterator orderIt;
    };

    void eraseUnlocked(const std::string& host);
    bool isExpired(const CacheEntry& entry) const;
    void touchUnlocked(CacheEntry& entry, const std::string& host);
    void trimToCapacityUnlocked();

   private:
    mutable std::mutex mu_;
    std::map<std::string, CacheEntry> data_;
    std::list<std::string> order_;
    size_t maxHosts_;
};

struct ResolveBinding {
    std::string host;
    std::string port;
    std::string selectedIp;
    curl_slist* resolveList = nullptr;

    bool active() const {
        return resolveList != nullptr && !selectedIp.empty();
    }
};

template <typename RESOURCE_TYPE>
class ResourceManager_ {
   public:
    ResourceManager_() : m_shutdown(false) {}
    RESOURCE_TYPE Acquire() {
        std::unique_lock<std::mutex> locker(m_queueLock);
        while (!m_shutdown.load() && m_resources.size() == 0) {
            m_semaphore.wait(locker, [&]() { return m_shutdown.load() || m_resources.size() > 0; });
        }

        assert(!m_shutdown.load());

        RESOURCE_TYPE resource = m_resources.back();
        m_resources.pop_back();

        return resource;
    }

    bool HasResourcesAvailable() {
        std::lock_guard<std::mutex> locker(m_queueLock);
        return m_resources.size() > 0 && !m_shutdown.load();
    }

    bool TryAcquire(RESOURCE_TYPE& resource) {
        std::lock_guard<std::mutex> locker(m_queueLock);
        if (m_shutdown.load() || m_resources.empty()) {
            return false;
        }
        resource = m_resources.back();
        m_resources.pop_back();
        return true;
    }

    void Release(RESOURCE_TYPE resource) {
        std::unique_lock<std::mutex> locker(m_queueLock);
        m_resources.push_back(resource);
        locker.unlock();
        m_semaphore.notify_one();
    }

    void PutResource(RESOURCE_TYPE resource) { m_resources.push_back(resource); }

    std::vector<RESOURCE_TYPE> ShutdownAndWait(size_t resourceCount) {
        std::vector<RESOURCE_TYPE> resources;
        std::unique_lock<std::mutex> locker(m_queueLock);
        m_shutdown = true;
        while (m_resources.size() < resourceCount) {
            m_semaphore.wait(locker, [&]() { return m_resources.size() == resourceCount; });
        }
        resources = m_resources;
        m_resources.clear();
        return resources;
    }

   private:
    std::vector<RESOURCE_TYPE> m_resources;
    std::mutex m_queueLock;
    std::condition_variable m_semaphore;
    std::atomic<bool> m_shutdown;
};

class CurlContainer {
   public:
    explicit CurlContainer(unsigned maxSize = 25, long socketTimeout = 30000, long connectTimeout = 10000)
        : maxPoolSize_(maxSize), socketTimeout_(socketTimeout), connectTimeout_(connectTimeout), poolSize_(0) {}

    ~CurlContainer() {
        for (CURL* handle : handleContainer_.ShutdownAndWait(poolSize_)) {
            curl_easy_cleanup(handle);
        }
    }

    CURL* Acquire() {
        if (!handleContainer_.HasResourcesAvailable()) {
            growPool();
        }
        CURL* handle = handleContainer_.Acquire();
        return handle;
    }

    void Release(CURL* handle, bool force) {
        if (handle) {
            curl_easy_reset(handle);
            if (force) {
                CURL* newhandle = curl_easy_init();
                if (newhandle) {
                    curl_easy_cleanup(handle);
                    handle = newhandle;
                }
            }
            SetDefaultOptions(handle, connectTimeout_, socketTimeout_);
            handleContainer_.Release(handle);
        }
    }

    static void SetDefaultOptions(CURL* curl, unsigned long connectTimeout, unsigned long socketTimeout) {
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
        curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, socketTimeout / 1000);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

   private:
    CurlContainer(const CurlContainer&) = delete;
    const CurlContainer& operator=(const CurlContainer&) = delete;
    CurlContainer(const CurlContainer&&) = delete;
    const CurlContainer& operator=(const CurlContainer&&) = delete;

    bool growPool() {
        std::lock_guard<std::mutex> locker(containerLock_);
        if (poolSize_ < maxPoolSize_) {
            unsigned multiplier = poolSize_ > 0 ? poolSize_ : 1;
            unsigned amountToAdd = (std::min)(multiplier * 2, maxPoolSize_ - poolSize_);

            unsigned actuallyAdded = 0;
            for (unsigned i = 0; i < amountToAdd; ++i) {
                CURL* curlHandle = curl_easy_init();
                if (curlHandle) {
                    SetDefaultOptions(curlHandle, connectTimeout_, socketTimeout_);
                    handleContainer_.Release(curlHandle);
                    ++actuallyAdded;
                } else {
                    break;
                }
            }
            poolSize_ += actuallyAdded;
            return actuallyAdded > 0;
        }
        return false;
    }

   private:
    ResourceManager_<CURL*> handleContainer_;
    unsigned maxPoolSize_;
    unsigned long socketTimeout_;
    unsigned long connectTimeout_;
    unsigned poolSize_;
    std::mutex containerLock_;
};

class HttpClient {
   public:
    HttpClient();

    explicit HttpClient(const HttpConfig& config);
    virtual ~HttpClient() {
        if (curlContainer_ != nullptr) {
            delete curlContainer_;
        }
    }

    static void initGlobalState();
    static void cleanupGlobalState();

    std::shared_ptr<HttpResponse> doRequest(const std::shared_ptr<HttpRequest>& request);
    CURLcode invokeSslCtxCallback(void* sslCtx) const;

   protected:
    void setShareHandle(void* curl_handle, int cacheTime);
    void removeDNS(void* curl_handle, const std::shared_ptr<HttpRequest>& request);
    ResolveBinding buildResolveBinding(const std::shared_ptr<HttpRequest>& request);
    void releaseResolveBinding(ResolveBinding& binding) const;
    void removeFailedIp(const ResolveBinding& binding);
    std::vector<std::string> getHostIpList(const std::string& host);
    std::vector<std::string> lookupHostIpList(const std::string& host) const;
    CURLSH* share_handle = nullptr;

   protected:
    int requestTimeout_ = 0;
    int socketTimeout_ = 30000;
    int dialTimeout_{};
    int tcpKeepAlive_{};
    int connectTimeout_ = 10000;
    bool enableVerifySSL_ = true;
    std::string proxyHost_;
    int proxyPort_ = -1;
    std::string proxyUsername_;
    std::string proxyPassword_;
    int dnsCacheTime_ = 0;
    bool enableDnsIpBalancing_ = false;
    std::string caPath_;
    std::string caFile_;
    int highLatencyLogThreshold_ = 100;
    std::mutex mu_;
    VolcengineTos::CurlContainer* curlContainer_{};
    std::string clientCrt_;  // 客户端证书路径
    std::string clientKey_;  // 客户端私钥路径
    std::string netInterface_;
    SslCtxCallback sslCtxCallback_ = nullptr;
    void* sslCtxCallbackUserData_ = nullptr;
    HostIpCache dnsIpCache_;
    std::function<std::vector<std::string>(const std::string&)> dnsLookupCallback_;
};
}  // namespace VolcengineTos
