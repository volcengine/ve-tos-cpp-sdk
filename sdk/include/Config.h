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
    void setRegion(const std::string& region) {
        region_ = region;
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

private:
    std::string endpoint_;
    std::string region_;
    bool enableCRC_ = true;
    bool autoRecognizeContentType_ = true;
    int maxRetryCount_ = 3;
    TransportConfig transportConfig_;
    long retrySleepScale = 100;
};
}  // namespace VolcengineTos

#ifdef __cplusplus
}
#endif