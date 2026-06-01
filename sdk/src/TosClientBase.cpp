#include "TosClientBase.h"

#include "auth/SignV4.h"
#include "model/async/base/BaseHttp.h"
#include "model/async/base/BucketBaseInput.h"
#include "transport/DefaultTransport.h"

#include <cstring>


using namespace VolcengineTos;
LockFreeCache<std::string, BucketCache>& TosClientBase::getBucketCache() { return bucket_cache_; }

void TosClientBase::initWithoutConfig(const std::string& endpoint, const std::string& region) {
    std::string tmpEndpoint = endpoint;
    std::string tmpControlEndpoint;
    if (endpoint.empty() && supportedRegion_.count(region) != 0) {
        tmpEndpoint = supportedRegion_[region];
    }
    if (supportedRegionToControlEndpoint_.count(region) != 0) {
        tmpControlEndpoint = supportedRegionToControlEndpoint_[region];
    }

    init(tmpEndpoint, tmpControlEndpoint, region);
}

void TosClientBase::init(const std::string& endpoint, const std::string& controlEndpoint,
                         const std::string& region) {
    TransportConfig conf;
    conf.setAsyncTransportMode(AsyncTransportMode::Isolated);
    shared_async_transport_ = false;
    async_transport = AcquireAsyncHttpClient(conf, false);
    config_.setEndpoint(endpoint);
    config_.setControlEndpoint(controlEndpoint);
    config_.setRegion(region, endpoint);
    auto schemeHostParameter = initSchemeAndHost(endpoint);
    scheme_ = schemeHostParameter.scheme_;
    host_ = schemeHostParameter.host_;
    auto controlSchemeHostParameter = initSchemeAndHost(controlEndpoint);
    controlHost_ = controlSchemeHostParameter.host_;
    urlMode_ = schemeHostParameter.urlMode_;

    if (config_.isEnableCrc()) {
        async_transport->enable_crc();
    }
}

TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const StaticCredentials& cred) {
    initWithoutConfig(endpoint, region);
    credentials_ = std::make_shared<StaticCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}

TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const FederationCredentials& cred) {
    initWithoutConfig(endpoint, region);
    credentials_ = std::make_shared<FederationCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}

TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const StaticCredentials& cred, const ClientConfig& config) {
    initWithConfig(endpoint, region, config);
    credentials_ = std::make_shared<StaticCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}
TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const FederationCredentials& cred, const ClientConfig& config) {
    initWithConfig(endpoint, region, config);
    credentials_ = std::make_shared<FederationCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}

TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const std::shared_ptr<Credentials>& cred) {
    initWithoutConfig(endpoint, region);
    credentials_ = cred;
    signer_ = std::make_shared<SignV4>(credentials_, region);
}

TosClientBase::TosClientBase(const std::string& endpoint, const std::string& region,
                             const std::shared_ptr<Credentials>& cred, const ClientConfig& config) {
    initWithConfig(endpoint, region, config);
    credentials_ = cred;
    signer_ = std::make_shared<SignV4>(credentials_, region);
}

void TosClientBase::initWithConfig(const std::string& endpoint, const std::string& region,
                                   const ClientConfig& config) {
    std::string tmpEndpoint = endpoint;
    std::string tmpControlEndpoint = config.controlEndPoint;
    if (endpoint.empty() && supportedRegion_.count(region) != 0) {
        tmpEndpoint = supportedRegion_[region];
    }
    if (config.controlEndPoint.empty() && supportedRegionToControlEndpoint_.count(region) != 0) {
        tmpControlEndpoint = supportedRegionToControlEndpoint_[region];
    }

    init(tmpEndpoint, tmpControlEndpoint, region, config);
}

void TosClientBase::init(const std::string& endpoint, const std::string& controlEndpoint,
                         const std::string& region, const ClientConfig& config) {
    TransportConfig conf;
    // 涉及到 HttpClient 的参数放到这里
    // 需要同步修改 DefaultTransport(const TransportConfig& config) 以传参给 HttpConfig
    conf.setEnableVerifySsl(config.enableVerifySSL);
    conf.setRequestTimeout(config.requestTimeout);
    conf.setConnectTimeout(config.connectionTimeout);
    conf.setProxyHost(config.proxyHost);
    conf.setProxyPort(config.proxyPort);
    conf.setProxyUsername(config.proxyUsername);
    conf.setProxyPassword(config.proxyPassword);
    conf.setDnsCacheTime(config.dnsCacheTime);
    conf.setMaxConnections(config.maxConnections);
    conf.setSocketTimeout(config.socketTimeout);
    conf.setCaFile(config.caFile);
    conf.setCaPath(config.caPath);
    conf.setClientCrt(config.clientCrt);
    conf.setClientKey(config.clientKey);
    conf.setNetInterface(config.netInterface);
    conf.setSslCtxCallback(config.sslCtxCallback);
    conf.setSslCtxCallbackUserData(config.sslCtxCallbackUserData);
    conf.setHighLatencyLogThreshold(config.highLatencyLogThreshold);
    conf.setDetailLog(config.detail_log_);
    conf.setDetailLogInterval(config.detail_log_interval_s_);
    conf.setEventThreadCount(config.event_thread_count_);
    conf.setMaxRequestQueue(config.max_request_queue_);
    conf.setConnectionReuse(config.connection_reuse_);
    conf.setCurlMultiWaitTimeoutMs(config.curl_multi_wait_timeout_ms_);
    conf.setAsyncTransportMode(config.async_transport_mode_);

    shared_async_transport_ = (config.async_transport_mode_ == AsyncTransportMode::Shared);
    async_transport = AcquireAsyncHttpClient(conf, config.enableCRC);

    config_.setTransportConfig(conf);
    config_.setEndpoint(endpoint);
    config_.setControlEndpoint(controlEndpoint);
    config_.setRegion(region, endpoint);
    config_.setIsCustomDomain(config.isCustomDomain);
    config_.setEnableCrc(config.enableCRC);
    config_.setAutoRecognizeContentType(config.autoRecognizeContentType);
    config_.setMaxRetryCount(config.maxRetryCount);
    config_.setTosStatFallback(config.tosStatFallback);
    auto schemeHostParameter = initSchemeAndHost(endpoint);
    scheme_ = schemeHostParameter.scheme_;
    host_ = schemeHostParameter.host_;
    auto controlSchemeHostParameter = initSchemeAndHost(controlEndpoint);
    controlHost_ = controlSchemeHostParameter.host_;
    urlMode_ = schemeHostParameter.urlMode_;

    // user_agent扩展设置
    if (!config.userAgentProductName.empty() || !config.userAgentSoftName.empty() ||
        !config.userAgentSoftVersion.empty() || !config.userAgentCustomizedKeyValues.empty()) {
        std::stringstream ss;
        ss << userAgent_;

        if (!config.userAgentProductName.empty()) {
            ss << "--" << config.userAgentProductName << "/";
        } else {
            ss << "--undefined/";
        }
        if (!config.userAgentSoftName.empty()) {
            ss << config.userAgentSoftName << "/";
        } else {
            ss << "undefined/";
        }
        if (!config.userAgentSoftVersion.empty()) {
            ss << config.userAgentSoftVersion << "";
        } else {
            ss << "undefined";
        }
        for (auto it = config.userAgentCustomizedKeyValues.begin();
             it != config.userAgentCustomizedKeyValues.end(); ++it) {
            if (it == config.userAgentCustomizedKeyValues.begin()) {
                ss << "(";
            }
            if (std::next(it) == config.userAgentCustomizedKeyValues.end()) {
                ss << it->first << "/" << it->second << ")";
                break;
            }
            ss << it->first << "/" << it->second << ";";
        }
        userAgent_ = ss.str();
    }
}

void TosClientBase::closeAsyncTransport(const bool force_close) {
    if (async_transport == nullptr) {
        return;
    }

    if (shared_async_transport_ && !force_close) {
        async_transport.reset();
        return;
    }

    async_transport->closeClient();
    async_transport.reset();
}

SchemeHostParameter TosClientBase::initSchemeAndHost(const std::string& endpoint) {
    // set scheme and host
    SchemeHostParameter schemeHostParameter;
    if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTPS)) {
        schemeHostParameter.scheme_ = http::SchemeHTTPS;
        schemeHostParameter.host_ = endpoint.substr(std::strlen(http::SchemeHTTPS) + 3,
                                                    endpoint.length() - std::strlen(http::SchemeHTTPS) - 3);
    } else if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTP)) {
        schemeHostParameter.scheme_ = http::SchemeHTTP;
        schemeHostParameter.host_ = endpoint.substr(std::strlen(http::SchemeHTTP) + 3,
                                                    endpoint.length() - std::strlen(http::SchemeHTTP) - 3);
    } else {
        schemeHostParameter.scheme_ = http::SchemeHTTPS;
        schemeHostParameter.host_ = endpoint;
    }

    if (userAgent_.empty()) userAgent_ = DefaultUserAgent();
    schemeHostParameter.urlMode_ = URL_MODE_DEFAULT;
    return schemeHostParameter;
}

void TosClientBase::initRegionEndpoint(const std::string& endpoint, const std::string& region) {
    config_.setEndpoint(endpoint);
    config_.setRegion(region, endpoint);
    auto schemeHostParameter = initSchemeAndHost(endpoint);
    scheme_ = schemeHostParameter.scheme_;
    host_ = schemeHostParameter.host_;
    urlMode_ = schemeHostParameter.urlMode_;
    // if (!NetUtils::isNotIP(host_)) {
    //     connectWithIP_ = true;
    // }
    // if (NetUtils::isS3Endpoint(host_)) {
    //     connectWithS3EndPoint_ = true;
    // }
}

void TosClientBase::setCredential(const std::string& accessKeyId, const std::string& secretKeyId) {
    auto cred = StaticCredentials(accessKeyId, secretKeyId);
    credentials_ = std::make_shared<StaticCredentials>(cred);
}

void TosClientBase::setCredential(const std::string& accessKeyId, const std::string& secretKeyId,
                                  const std::string& securityToken) {
    auto cred = StaticCredentials(accessKeyId, secretKeyId, securityToken);
    credentials_ = std::make_shared<StaticCredentials>(cred);
}

std::string TosClientBase::getAK() const { return credentials_->credential().getAccessKeyId(); }

std::string TosClientBase::getSK() const { return credentials_->credential().getAccessKeySecret(); }

std::string TosClientBase::getSecurityToken() const {
    return credentials_->credential().getSecurityToken();
}

const std::string& TosClientBase::getRegion() const { return config_.getRegion(); }

const std::string& TosClientBase::getEndpoint() const { return config_.getEndpoint(); }

void TosClientBase::setRegion(const std::string& region) {
    initRegionEndpoint(supportedRegion_[region], region);
}

void TosClientBase::setRegionEndpoint(const std::string& region, const std::string& endpoint) {
    if (endpoint.empty()) {
        if (supportedRegion_.count(region) != 0) {
            initRegionEndpoint(supportedRegion_[region], region);
        } else {
            initRegionEndpoint(endpoint, region);
        }
    } else {
        initRegionEndpoint(endpoint, region);
    }
}

RequestBuilder TosClientBase::newBuilder(const std::string& bucket, const std::string& object,
                                         const time_t& date,
                                         const std::map<std::string, std::string>& input_headers,
                                         const std::map<std::string, std::string>& input_queries,
                                         const int64_t& content_length) const {
    const std::map<std::string, std::string> headers;
    const std::map<std::string, std::string> queries;
    auto rb = RequestBuilder(signer_, scheme_, host_, "", "", bucket, object, urlMode_, headers, queries,
                             config_.isCustomDomain());
    rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
    rb.setRequestDate(date);

    rb.setRequestHeader(input_headers);
    rb.setQuery(input_queries);
    rb.setContentLength(content_length);
    return rb;
}

Config TosClientBase::getConfig() const { return config_; }
