#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class GetBucketReplicationInput {
public:
    GetBucketReplicationInput(std::string bucket, std::string ruleId)
            : bucket_(std::move(bucket)), ruleId_(std::move(ruleId)) {
    }
    GetBucketReplicationInput() = default;
    virtual ~GetBucketReplicationInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getRuleId() const {
        return ruleId_;
    }
    void setRuleId(const std::string& ruleId) {
        ruleId_ = ruleId;
    }

private:
    std::string bucket_;
    std::string ruleId_;
};
}  // namespace VolcengineTos
