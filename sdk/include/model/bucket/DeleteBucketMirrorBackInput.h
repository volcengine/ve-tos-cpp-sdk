#pragma once

#include <string>
#include <utility>
#include <Type.h>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketMirrorBackInput : public GenericInput {
public:
    DeleteBucketMirrorBackInput() = default;
    virtual ~DeleteBucketMirrorBackInput() = default;
    explicit DeleteBucketMirrorBackInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
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
