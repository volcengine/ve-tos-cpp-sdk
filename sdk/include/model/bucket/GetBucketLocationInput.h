#pragma once

#include <string>
#include <Type.h>
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketLocationInput : public GenericInput {
public:
    explicit GetBucketLocationInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketLocationInput() = default;
    virtual ~GetBucketLocationInput() = default;
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
