#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketLifecycleInput : public GenericInput {
public:
    explicit GetBucketLifecycleInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketLifecycleInput() = default;
    virtual ~GetBucketLifecycleInput() = default;
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
