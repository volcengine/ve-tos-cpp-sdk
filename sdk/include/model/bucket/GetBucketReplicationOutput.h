#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "ReplicationRule.h"

namespace VolcengineTos {
class GetBucketReplicationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<ReplicationRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<ReplicationRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::vector<ReplicationRule> rules_;
};
}  // namespace VolcengineTos
