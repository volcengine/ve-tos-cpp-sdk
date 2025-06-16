#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketReplicationInput : public GenericInput {
public:
    explicit DeleteBucketReplicationInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketReplicationInput() = default;
    virtual ~DeleteBucketReplicationInput() = default;
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
