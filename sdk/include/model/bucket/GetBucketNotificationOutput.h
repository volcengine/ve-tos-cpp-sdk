#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "CloudFunctionConfiguration.h"
#include "RocketMQConfiguration.h"

namespace VolcengineTos {
class GetBucketNotificationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<CloudFunctionConfiguration>& getCloudFunctionConfigurations() const {
        return CloudFunctionConfigurations_;
    }
    void setCloudFunctionConfigurations(const std::vector<CloudFunctionConfiguration>& cloudFunctionConfigurations) {
        CloudFunctionConfigurations_ = cloudFunctionConfigurations;
    }
    const std::vector<RocketMQConfiguration>& getRocketMqConfigurations() const {
        return rocketMQConfigurations_;
    }
    void setRocketMqConfigurations(const std::vector<RocketMQConfiguration>& rocketMqConfigurations) {
        rocketMQConfigurations_ = rocketMqConfigurations;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations_;
    std::vector<RocketMQConfiguration> rocketMQConfigurations_;
};
}  // namespace VolcengineTos
