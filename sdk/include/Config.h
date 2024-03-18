#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <string>
#include "transport/TransportConfig.h"
namespace VolcengineTos {
class Config {
public:
    Config() = default;
    ~Config() = default;
    const std::string& getEndpoint() const {
        return endpoint_;
    }
    void setEndpoint(const std::string& endpoint) {
        endpoint_ = endpoint;
    }
    const std::string& getRegion() const {
        return region_;
    }
    void setRegion(const std::string& region, const std::string& endpoint) {
        // 当 region 为空，但 endpoint 设置的情况，将 region 设置为默认值 unset，在不校验 region 的情况下可以算签名成功
        if (region.empty() && !endpoint.empty()) {
            region_ = "unset";
        } else {
            region_ = region;
        }
    }
    const TransportConfig& getTransportConfig() const {
        return transportConfig_;
    }
    void setTransportConfig(const TransportConfig& transportConfig) {
        transportConfig_ = transportConfig;
    }
    bool isEnableCrc() const {
        return enableCRC_;
    }
    void setEnableCrc(bool enablecrc) {
        enableCRC_ = enablecrc;
    }
    bool isAutoRecognizeContentType() const {
        return autoRecognizeContentType_;
    }
    void setAutoRecognizeContentType(bool autorecognizecontenttype) {
        autoRecognizeContentType_ = autorecognizecontenttype;
    }
    int getMaxRetryCount() const {
        return maxRetryCount_;
    }
    void setMaxRetryCount(int maxretrycount) {
        maxRetryCount_ = maxretrycount;
    }
    long getRetrySleepScale() const {
        return retrySleepScale;
    }
    void setRetrySleepScale(long retrysleepscale) {
        retrySleepScale = retrysleepscale;
    }
    bool isCustomDomain() const {
        return isCustomDomain_;
    }
    void setIsCustomDomain(bool isCustomDomain) {
        isCustomDomain_ = isCustomDomain;
    }

private:
    std::string endpoint_;
    std::string region_;
    bool enableCRC_ = true;
    bool autoRecognizeContentType_ = true;
    int maxRetryCount_ = 3;
    TransportConfig transportConfig_;
    long retrySleepScale = 100;
    bool isCustomDomain_ = false;
};
}  // namespace VolcengineTos

#ifdef __cplusplus
}
#endif