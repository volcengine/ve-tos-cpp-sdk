#pragma once
#include <string>
namespace VolcengineTos {
class PutBucketPolicyInput {
public:
    explicit PutBucketPolicyInput(const std::string& bucket) : bucket_(bucket) {
    }
    PutBucketPolicyInput(std::string bucket, std::string policy)
            : bucket_(std::move(bucket)), policy_(std::move(policy)) {
    }
    PutBucketPolicyInput() = default;
    virtual ~PutBucketPolicyInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getPolicy() const {
        return policy_;
    }
    void setPolicy(const std::string& policy) {
        policy_ = policy;
    }

private:
    std::string bucket_;
    std::string policy_;
};
}  // namespace VolcengineTos
