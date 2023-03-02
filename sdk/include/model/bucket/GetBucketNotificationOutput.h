#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "CloudFunctionConfiguration.h"

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
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations_;
};
}  // namespace VolcengineTos
