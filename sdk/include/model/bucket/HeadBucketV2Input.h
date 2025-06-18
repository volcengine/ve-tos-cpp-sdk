#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class HeadBucketV2Input : public GenericInput {
public:
    explicit HeadBucketV2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    HeadBucketV2Input() = default;
    ~HeadBucketV2Input() = default;
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
