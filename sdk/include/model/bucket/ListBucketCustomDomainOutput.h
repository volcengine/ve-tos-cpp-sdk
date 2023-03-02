#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "CustomDomainRule.h"

namespace VolcengineTos {
class ListBucketCustomDomainOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<CustomDomainRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<CustomDomainRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<CustomDomainRule> rules_;
};
}  // namespace VolcengineTos
