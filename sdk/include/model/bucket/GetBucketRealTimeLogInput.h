#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "MirrorBackRule.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetBucketRealTimeLogInput : public GenericInput {
public:
    explicit GetBucketRealTimeLogInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    GetBucketRealTimeLogInput() = default;
    virtual ~GetBucketRealTimeLogInput() = default;
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
