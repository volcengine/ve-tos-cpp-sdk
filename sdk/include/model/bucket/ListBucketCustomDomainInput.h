#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "MirrorBackRule.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListBucketCustomDomainInput : public GenericInput {
public:
    explicit ListBucketCustomDomainInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    ListBucketCustomDomainInput() = default;
    virtual ~ListBucketCustomDomainInput() = default;
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
