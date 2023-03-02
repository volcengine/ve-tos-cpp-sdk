#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "ReplicationRule.h"

namespace VolcengineTos {
class PutBucketReplicationInput {
public:
    explicit PutBucketReplicationInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketReplicationInput(std::string bucket, std::string role, std::vector<ReplicationRule> rules)
            : bucket_(std::move(bucket)), role_(std::move(role)), rules_(std::move(rules)) {
    }
    PutBucketReplicationInput() = default;
    virtual ~PutBucketReplicationInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getRole() const {
        return role_;
    }
    void setRole(const std::string& role) {
        role_ = role;
    }
    const std::vector<ReplicationRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<ReplicationRule>& rules) {
        rules_ = rules;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::string role_;
    std::vector<ReplicationRule> rules_;
};
}  // namespace VolcengineTos
