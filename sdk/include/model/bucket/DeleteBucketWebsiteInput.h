#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketWebsiteInput : public GenericInput {
public:
    explicit DeleteBucketWebsiteInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketWebsiteInput() = default;
    virtual ~DeleteBucketWebsiteInput() = default;
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
