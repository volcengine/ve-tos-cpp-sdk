#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "RealTimeLogConfiguration.h"

namespace VolcengineTos {
class PutBucketRealTimeLogInput {
public:
    explicit PutBucketRealTimeLogInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketRealTimeLogInput(std::string bucket, const RealTimeLogConfiguration& configuration)
            : bucket_(std::move(bucket)), configuration_(configuration) {
    }
    PutBucketRealTimeLogInput() = default;
    virtual ~PutBucketRealTimeLogInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const RealTimeLogConfiguration& getConfiguration() const {
        return configuration_;
    }
    void setConfiguration(const RealTimeLogConfiguration& configuration) {
        configuration_ = configuration;
    }

    std::string toJsonString() const;

private:
    std::string bucket_;
    RealTimeLogConfiguration configuration_;
};
}  // namespace VolcengineTos
