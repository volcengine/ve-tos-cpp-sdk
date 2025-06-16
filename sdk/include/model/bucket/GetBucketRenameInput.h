#pragma once
#include <string>
#include <utility>
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketRenameInput : public GenericInput {
public:
    explicit GetBucketRenameInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketRenameInput() = default;
    virtual ~GetBucketRenameInput() = default;
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
