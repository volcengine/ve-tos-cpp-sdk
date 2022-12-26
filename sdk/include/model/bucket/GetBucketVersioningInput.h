#pragma once
#include <string>
#include <utility>
namespace VolcengineTos {
class GetBucketVersioningInput {
public:
    explicit GetBucketVersioningInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketVersioningInput() = default;
    virtual ~GetBucketVersioningInput() = default;
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
