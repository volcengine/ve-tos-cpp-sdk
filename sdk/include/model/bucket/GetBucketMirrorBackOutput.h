
#pragma once

#include "model/RequestInfo.h"
#include "MirrorBackRule.h"
namespace VolcengineTos {
class GetBucketMirrorBackOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<MirrorBackRule>& getRules() const {
        return rules;
    }
    void setRules(const std::vector<MirrorBackRule>& rules) {
        GetBucketMirrorBackOutput::rules = rules;
    }

private:
    RequestInfo requestInfo_;
    std::vector<MirrorBackRule> rules;
};
}  // namespace VolcengineTos
