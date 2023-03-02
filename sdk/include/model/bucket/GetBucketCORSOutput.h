#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "CORSRule.h"
namespace VolcengineTos {
class GetBucketCORSOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::vector<CORSRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<CORSRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<CORSRule> rules_;
};
}  // namespace VolcengineTos
