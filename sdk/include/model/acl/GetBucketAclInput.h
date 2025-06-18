#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketAclInput : public GenericInput {
public:
    explicit GetBucketAclInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketAclInput() = default;
    virtual ~GetBucketAclInput() = default;
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
