#pragma once
#include <string>
#include <utility>
namespace VolcengineTos {
class GetBucketPolicyInput {
public:
    explicit GetBucketPolicyInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketPolicyInput() = default;
    virtual ~GetBucketPolicyInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }

private:
    std::string bucket_;
};
}  // namespace VolcengineTos
