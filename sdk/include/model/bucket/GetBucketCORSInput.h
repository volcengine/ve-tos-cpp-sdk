#pragma once

#include <string>
#include <utility>
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketCORSInput : public GenericInput {
public:
    explicit GetBucketCORSInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketCORSInput() = default;
    virtual ~GetBucketCORSInput() = default;
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
