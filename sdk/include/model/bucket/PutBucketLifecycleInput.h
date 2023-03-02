#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "LifecycleRule.h"

namespace VolcengineTos {
class PutBucketLifecycleInput {
public:
    explicit PutBucketLifecycleInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketLifecycleInput(std::string bucket, std::vector<LifecycleRule> rules)
            : bucket_(std::move(bucket)), rules_(std::move(rules)) {
    }
    PutBucketLifecycleInput() = default;
    virtual ~PutBucketLifecycleInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<LifecycleRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<LifecycleRule>& rules) {
        rules_ = rules;
    }

    std::string toJsonString() const;

private:
    std::string bucket_;
    std::vector<LifecycleRule> rules_;
};
}  // namespace VolcengineTos
