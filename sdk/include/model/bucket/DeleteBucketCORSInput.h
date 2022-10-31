#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class DeleteBucketCORSInput {
public:
    explicit DeleteBucketCORSInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketCORSInput() = default;
    virtual ~DeleteBucketCORSInput() = default;
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
