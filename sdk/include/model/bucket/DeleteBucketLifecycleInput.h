#pragma once

#include <string>
#include <Type.h>
#include <vector>

namespace VolcengineTos {
class DeleteBucketLifecycleInput {
public:
    explicit DeleteBucketLifecycleInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketLifecycleInput() = default;
    virtual ~DeleteBucketLifecycleInput() = default;
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
