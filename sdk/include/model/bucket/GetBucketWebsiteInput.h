#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class GetBucketWebsiteInput {
public:
    explicit GetBucketWebsiteInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketWebsiteInput() = default;
    virtual ~GetBucketWebsiteInput() = default;
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
