#pragma once

#include <string>
#include <utility>
#include <vector>
#include "CORSRule.h"
namespace VolcengineTos {
class PutBucketCORSInput {
public:
    explicit PutBucketCORSInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketCORSInput(std::string bucket, std::vector<CORSRule> rules)
            : bucket_(std::move(bucket)), rules_(std::move(rules)) {
    }
    PutBucketCORSInput() = default;
    virtual ~PutBucketCORSInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<CORSRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<CORSRule>& rules) {
        rules_ = rules;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::vector<CORSRule> rules_;
};
}  // namespace VolcengineTos
