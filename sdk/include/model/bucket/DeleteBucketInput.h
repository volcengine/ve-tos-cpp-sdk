#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketInput : public GenericInput {
public:
    explicit DeleteBucketInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    DeleteBucketInput() = default;
    ~DeleteBucketInput() = default;

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
