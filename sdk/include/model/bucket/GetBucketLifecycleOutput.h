#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "LifecycleRule.h"
namespace VolcengineTos {
class GetBucketLifecycleOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<LifecycleRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<LifecycleRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<LifecycleRule> rules_;
};
}  // namespace VolcengineTos
