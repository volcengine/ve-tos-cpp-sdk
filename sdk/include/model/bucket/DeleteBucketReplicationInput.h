#pragma once

#include <string>
#include <Type.h>
#include <vector>

namespace VolcengineTos {
class DeleteBucketReplicationInput {
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
