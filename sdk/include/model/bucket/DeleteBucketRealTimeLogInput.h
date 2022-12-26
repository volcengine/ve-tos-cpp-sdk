#pragma once

#include <string>
#include <Type.h>
#include <vector>

namespace VolcengineTos {
class DeleteBucketRealTimeLogInput {
public:
    explicit DeleteBucketRealTimeLogInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketRealTimeLogInput() = default;
    virtual ~DeleteBucketRealTimeLogInput() = default;
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
