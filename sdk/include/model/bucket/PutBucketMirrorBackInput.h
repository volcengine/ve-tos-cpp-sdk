#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class PutBucketMirrorBackInput {
public:
    explicit PutBucketMirrorBackInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketMirrorBackInput(std::string bucket, std::vector<MirrorBackRule> rules)
            : bucket_(std::move(bucket)), rules_(std::move(rules)) {
    }
    PutBucketMirrorBackInput() = default;
    virtual ~PutBucketMirrorBackInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<MirrorBackRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<MirrorBackRule>& rules) {
        rules_ = rules;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::vector<MirrorBackRule> rules_;
};
}  // namespace VolcengineTos
