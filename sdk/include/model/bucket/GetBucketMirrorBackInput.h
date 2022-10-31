#pragma once

#include <string>
#include <utility>
#include <Type.h>

namespace VolcengineTos {
class GetBucketMirrorBackInput {
public:
    explicit GetBucketMirrorBackInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    virtual ~GetBucketMirrorBackInput() = default;
    GetBucketMirrorBackInput() = default;
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
