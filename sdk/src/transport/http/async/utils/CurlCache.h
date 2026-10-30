#pragma once
#include <algorithm>
#include <queue>
#include <utility>
#include <stdexcept>
#include <curl/curl.h>
namespace VolcengineTos {
class CurlCache {
public:
    explicit CurlCache(TransportConfig transport_config, const int pool_size)
            : transport_config_(std::move(transport_config)) {
        maxPoolSize_ = pool_size;
        initShareHandle();
    }
    // 禁用拷贝移动
    CurlCache(const CurlCache&) = delete;
    CurlCache& operator=(const CurlCache&) = delete;

    ~CurlCache() {
        while (!idleHandles_.empty()) {
            CURL* handle = idleHandles_.front();
            idleHandles_.pop();
            curl_easy_cleanup(handle);
        }

        if (share_handle_) {
            curl_share_cleanup(share_handle_);
            share_handle_ = nullptr;
        }
    }

    unsigned getMaxPoolSize() const {
        return maxPoolSize_;
    }

    std::string printStatus() const {
        std::stringstream ss;
        const size_t idle_size = idleHandles_.size();
        const std::string share_status = (share_handle_ != nullptr) ? "OK" : "NULL";

        ss << "[CurlCache] IdleHandles=" << idle_size << ", PoolSize=" << poolSize_ << ", MaxPoolSize=" << maxPoolSize_
           << ", ShareHandle=" << share_status
           << ", UsedRatio=" << (maxPoolSize_ > 0 ? (poolSize_ * 100.0 / maxPoolSize_) : 0) << "%";
        return ss.str();
    }

    // 从线程本地池获取句柄
    CURL* Acquire() {
        if (!idleHandles_.empty()) {
            CURL* handle = idleHandles_.front();
            idleHandles_.pop();
            return handle;
        }
        if (growPool()) {
            return Acquire();
        }
        // 极端情况：创建临时句柄（避免阻塞）
        //        CURL* handle = curl_easy_init();
        //        if (handle)
        //            SetDefaultOptions(handle);
        return nullptr;
    }

    // 归还句柄到线程本地池（复用）
    void Release(CURL* handle, bool force = false) noexcept {
        if (!handle)
            return;

        if (force) {
            curl_easy_cleanup(handle);
            handle = curl_easy_init();
            if (!handle) {
                if (poolSize_ > 0) --poolSize_;
                return;
            }
        }

        try {
            // 手动重置请求相关选项（保留DNS缓存等全局状态）
            resetRequestOptions(handle);
            SetDefaultOptions(handle);  // 确保默认选项有效

            if (poolSize_ <= maxPoolSize_) {
                idleHandles_.push(handle);
            } else {
                curl_easy_cleanup(handle);
                poolSize_--;
            }
        } catch (...) {
            // Release is used by RequestContext's destructor. A failed proxy
            // option string or idle-queue allocation must not terminate the
            // worker or leak the detached handle.
            curl_easy_cleanup(handle);
            if (poolSize_ > 0) --poolSize_;
        }
    }

    // Shared engine applies client/request options on every acquisition. The
    // cache itself must never learn one client's timeout or callback captures.
    void Configure(CURL* curl, const TransportConfig& config, CURLSH* shared,
                   long remaining_ms, const std::string& environment_proxy,
                   const std::string& environment_no_proxy) {
        curl_easy_reset(curl);
        auto check = [](CURLcode code) {
            if (code != CURLE_OK) throw std::runtime_error("shared curl option setup failed");
        };
        if (shared) check(curl_easy_setopt(curl, CURLOPT_SHARE, shared));
        SetDefaultOptions(curl, config, false);
        check(curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, remaining_ms));
        if (!config.getNetInterface().empty())
            check(curl_easy_setopt(curl, CURLOPT_INTERFACE, config.getNetInterface().c_str()));
        // Preserve environment proxy routing, frozen at engine creation. Empty
        // snapshot values explicitly disable a later environment mutation.
        if (config.getProxyPort() < 0 || config.getProxyHost().empty())
            check(curl_easy_setopt(curl, CURLOPT_PROXY, environment_proxy.c_str()));
        check(curl_easy_setopt(curl, CURLOPT_NOPROXY, environment_no_proxy.c_str()));
    }

private:
    void initShareHandle() {
        // 1. 初始化共享句柄
        share_handle_ = curl_share_init();
        if (!share_handle_) {
            // 可添加日志：共享句柄初始化失败，降级为单句柄DNS缓存
            return;
        }

        // 2. 配置共享资源：仅共享DNS缓存（核心需求）
        // 其他资源（如Cookie、SSL会话）无需共享，避免耦合
        curl_share_setopt(share_handle_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);

        // 3. 线程本地场景：无需设置锁回调（单线程访问无并发）
    }

    // 扩容线程本地池
    bool growPool() {
        if (poolSize_ >= maxPoolSize_)
            return false;
        const unsigned add = std::min((poolSize_ > 0 ? poolSize_ : 1) * 2, maxPoolSize_ - poolSize_);
        unsigned added = 0;
        for (unsigned i = 0; i < add; ++i) {
            CURL* h = curl_easy_init();
            if (h) {
                try {
                    SetDefaultOptions(h);
                    idleHandles_.push(h);
                } catch (...) {
                    curl_easy_cleanup(h);
                    throw;
                }
                ++poolSize_;
                added++;
            } else
                break;
        }
        return added > 0;
    }

    // 设置句柄默认选项（与业务无关的全局配置）
    void SetDefaultOptions(CURL* curl) const {
        SetDefaultOptions(curl, transport_config_, true);
    }
    void SetDefaultOptions(CURL* curl, const TransportConfig& transport_config_, bool local_share) const {
        auto set = [&](CURLoption option, auto value) {
            const auto code = curl_easy_setopt(curl, option, value);
            if (!local_share && code != CURLE_OK)
                throw std::runtime_error("shared curl profile option failed");
        };
        if (transport_config_.getDnsCacheTime() > 0) {
            if (local_share && share_handle_) {
                set(CURLOPT_SHARE, share_handle_);
            }
            set(CURLOPT_DNS_CACHE_TIMEOUT, transport_config_.getDnsCacheTime() * 60);
        }

        set(CURLOPT_BUFFERSIZE, 65535);  // libcurl内部缓存大小
        set(CURLOPT_NOSIGNAL, 1L);
        set(CURLOPT_TCP_NODELAY, 1L);
        set(CURLOPT_NETRC, CURL_NETRC_IGNORED);
        set(CURLOPT_CONNECTTIMEOUT_MS, transport_config_.getConnectTimeout());
        set(CURLOPT_LOW_SPEED_LIMIT, 1L);
        set(CURLOPT_LOW_SPEED_TIME, transport_config_.getSocketTimeout() / 1000);

        if (transport_config_.getRequestTimeout() != 0) {
            set(CURLOPT_TIMEOUT_MS, transport_config_.getRequestTimeout());
        }

        if (transport_config_.getProxyPort() != -1 && !transport_config_.getProxyHost().empty()) {
            const std::string proxy =
                    transport_config_.getProxyHost() + ":" + std::to_string(transport_config_.getProxyPort());
            const std::string proxyUserPwd =
                    transport_config_.getProxyUsername() + ":" + transport_config_.getProxyPassword();
            set(CURLOPT_PROXY, proxy.c_str());
            set(CURLOPT_PROXYUSERPWD, proxyUserPwd.c_str());
            set(CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        }

        // 支持忽略SSL证书校验
        if (!transport_config_.isEnableVerifySsl()) {
            set(CURLOPT_SSL_VERIFYHOST, 0L);
            set(CURLOPT_SSL_VERIFYPEER, 0L);
        } else {
            set(CURLOPT_SSL_VERIFYPEER, 1L);
            set(CURLOPT_SSL_VERIFYHOST, 2L);
        }
        if (!transport_config_.getCaPath().empty()) {
            set(CURLOPT_CAPATH, transport_config_.getCaPath().c_str());
        }
        if (!transport_config_.getCaFile().empty()) {
            set(CURLOPT_CAINFO, transport_config_.getCaFile().c_str());
        }
        if (!transport_config_.getClientCrt().empty()) {
            set(CURLOPT_SSLCERT, transport_config_.getClientCrt().c_str());
        }
        if (!transport_config_.getClientKey().empty()) {
            set(CURLOPT_SSLKEY, transport_config_.getClientKey().c_str());
        }

        if (!transport_config_.isConnectionReuse()) {
            set(CURLOPT_FORBID_REUSE, 1L);
        } else {
            set(CURLOPT_FORBID_REUSE, 0L);
            // set(CURLOPT_FRESH_CONNECT, 1L);
        }

        if (transport_config_.isDetailLog()) {
            // set(CURLOPT_VERBOSE, 1L);  // 启用详细日志
        }

        if (transport_config_.getKeepAlive() > 0) {
            set(CURLOPT_TCP_KEEPALIVE, 1L);  // 启用TCP保活
            set(CURLOPT_TCP_KEEPIDLE,
                             transport_config_.getKeepAlive());  // 连接空闲30秒后发送保活探测
            set(CURLOPT_TCP_KEEPINTVL, 10L);  // 保活探测间隔10秒
        }

        //        set(CURLOPT_MAXAGE_CONN, 1800L);       // 秒 // todo:某写配置应该跟libcurl版本有关
        set(CURLOPT_MAXCONNECTS, 100L);
        set(CURLOPT_SSL_SESSIONID_CACHE, 1L);  // 复用SSL会话
        set(CURLOPT_ACCEPTTIMEOUT_MS, transport_config_.getRequestTimeout());
    }

    // 重置仅与单次请求相关的选项（保留DNS缓存等）
    static void resetRequestOptions(CURL* curl) {
        curl_easy_setopt(curl, CURLOPT_URL, nullptr);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl, CURLOPT_READDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_PRIVATE, nullptr);        // 清除RequestContext指针
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);  // 清除HTTP方法
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);              // 重置上传标记
    }

    std::queue<CURL*> idleHandles_;  // 线程本地空闲句柄队列（无锁）
    unsigned maxPoolSize_;
    unsigned poolSize_ = 0;
    CURLSH* share_handle_ = nullptr;
    TransportConfig transport_config_;
};
}  // namespace VolcengineTos
