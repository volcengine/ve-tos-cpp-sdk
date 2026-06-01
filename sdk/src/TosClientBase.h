#pragma once

#include "../include/ClientConfig.h"
#include "../include/RequestOptionBuilder.h"
#include "../include/Config.h"
#include "../include/auth/FederationCredentials.h"
#include "../include/SchemeHostParameter.h"
#include "../include/TosClient.h"
#include "../include/auth/StaticCredentials.h"
#include "transport/http/async/AsyncHttpClient.h"
#include "transport/http/async/AsyncHttpClientRegistry.h"
#include "utils/LockFreeCacheMap.h"

namespace VolcengineTos {

class TosClientBase {
public:
    TosClientBase(const std::string& endpoint, const std::string& region, const StaticCredentials& cred);
    TosClientBase(const std::string& endpoint, const std::string& region, const FederationCredentials& cred);
    TosClientBase(const std::string& endpoint, const std::string& region, const StaticCredentials& cred,
                  const ClientConfig& config);
    TosClientBase(const std::string& endpoint, const std::string& region, const FederationCredentials& cred,
                  const ClientConfig& config);
    TosClientBase(const std::string& endpoint, const std::string& region, const std::shared_ptr<Credentials>& cred);
    TosClientBase(const std::string& endpoint, const std::string& region, const std::shared_ptr<Credentials>& cred,
                  const ClientConfig& config);

    ~TosClientBase() = default;

    void setCredential(const std::string& accessKeyId, const std::string& secretKeyId);
    void setCredential(const std::string& accessKeyId, const std::string& secretKeyId,
                       const std::string& securityToken);
    std::string getAK() const;
    std::string getSK() const;
    std::string getSecurityToken() const;
    const std::string& getRegion() const;
    const std::string& getEndpoint() const;
    void setRegion(const std::string& region);
    void setRegionEndpoint(const std::string& region, const std::string& endpoint);
    RequestBuilder newBuilder(const std::string& bucket, const std::string& object, const time_t& date,
                              const std::map<std::string, std::string>& input_headers,
                              const std::map<std::string, std::string>& input_queries,
                              const int64_t& content_length) const;

    Config getConfig() const;
    LockFreeCache<std::string, BucketCache>& getBucketCache();

    std::shared_ptr<AsyncHttpClient> getAsyncTransport() {
        return async_transport;
    }
    bool isSharedAsyncTransport() const {
        return shared_async_transport_;
    }
    void closeAsyncTransport(bool force_close = false);

protected:
    /**
     * URL_MODE_DEFAULT url pattern is http(s)://{bucket}.domain/{object}
     */
    static const int URL_MODE_DEFAULT = 0;

private:
    std::string scheme_;
    std::string host_;
    std::string controlHost_;
    int urlMode_ = URL_MODE_DEFAULT;
    std::string userAgent_ = DefaultUserAgent();
    std::shared_ptr<Credentials> credentials_;
    std::shared_ptr<AsyncHttpClient> async_transport;
    bool shared_async_transport_{false};
    Config config_;

    std::shared_ptr<Signer> signer_;

    std::map<std::string, std::string> supportedRegion_ = {{"cn-beijing", "https://tos-cn-beijing.volces.com"},
                                                           {"cn-guangzhou", "https://tos-cn-guangzhou.volces.com"},
                                                           {"cn-shanghai", "https://tos-cn-shanghai.volces.com"},
                                                           {"ap-southeast-1", "https://tos-ap-southeast-1.volces.com"}};

    std::map<std::string, std::string> supportedRegionToControlEndpoint_ = {
            {"cn-beijing", "https://tos-control-cn-beijing.volces.com"},
            {"cn-guangzhou", "https://tos-control-cn-guangzhou.volces.com"},
            {"cn-shanghai", "https://tos-control-cn-shanghai.volces.com"},
            {"ap-southeast-1", "https://tos-control-ap-southeast-1.volces.com"}};

    LockFreeCache<std::string, BucketCache> bucket_cache_;

    void initWithoutConfig(const std::string& endpoint, const std::string& region);
    void init(const std::string& endpoint, const std::string& controlEndpoint, const std::string& region);
    void initWithConfig(const std::string& endpoint, const std::string& region, const ClientConfig& config);
    void init(const std::string& endpoint, const std::string& controlEndpoint, const std::string& region,
              const ClientConfig& config);
    void initRegionEndpoint(const std::string& endpoint, const std::string& region);
    SchemeHostParameter initSchemeAndHost(const std::string& endpoint);
};
}  // namespace VolcengineTos
