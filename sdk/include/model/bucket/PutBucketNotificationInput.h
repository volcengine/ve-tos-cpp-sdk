#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "CloudFunctionConfiguration.h"

namespace VolcengineTos {
class PutBucketNotificationInput {
public:
    explicit PutBucketNotificationInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketNotificationInput(std::string bucket, std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations)
            : bucket_(std::move(bucket)), CloudFunctionConfigurations_(std::move(CloudFunctionConfigurations)) {
    }
    PutBucketNotificationInput() = default;
    virtual ~PutBucketNotificationInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<CloudFunctionConfiguration>& getCloudFunctionConfigurations() const {
        return CloudFunctionConfigurations_;
    }
    void setCloudFunctionConfigurations(const std::vector<CloudFunctionConfiguration>& CloudFunctionConfigurations) {
        CloudFunctionConfigurations_ = CloudFunctionConfigurations;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations_;
};
}  // namespace VolcengineTos
