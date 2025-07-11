#pragma once
#include <string>
#include <utility>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketPolicyInput : public GenericInput {
public:
    explicit DeleteBucketPolicyInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketPolicyInput() = default;
    virtual ~DeleteBucketPolicyInput() = default;
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
