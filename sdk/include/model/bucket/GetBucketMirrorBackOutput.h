
#pragma once
#include "MirrorBackRule.h"
#include "model/RequestInfo.h"
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
        return rules_;
    }
    void setRules(const std::vector<MirrorBackRule>& rules) {
        rules_ = rules;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<MirrorBackRule> rules_;
};
}  // namespace VolcengineTos
