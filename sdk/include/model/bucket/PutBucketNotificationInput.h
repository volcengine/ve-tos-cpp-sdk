#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "CloudFunctionConfiguration.h"
#include "RocketMQConfiguration.h"
namespace VolcengineTos {
class PutBucketNotificationInput {
public:
    explicit PutBucketNotificationInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketNotificationInput(std::string bucket, std::vector<CloudFunctionConfiguration> cloudFunctionConfigurations)
            : bucket_(std::move(bucket)), cloudFunctionConfigurations_(std::move(cloudFunctionConfigurations)) {
    }
    PutBucketNotificationInput(std::string bucket, std::vector<RocketMQConfiguration> rocketMQConfigurations)
            : bucket_(std::move(bucket)), rocketMQConfigurations_(std::move(rocketMQConfigurations)) {
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
        return cloudFunctionConfigurations_;
    }
    void setCloudFunctionConfigurations(const std::vector<CloudFunctionConfiguration>& cloudFunctionConfigurations) {
        cloudFunctionConfigurations_ = cloudFunctionConfigurations;
    }
    const std::vector<RocketMQConfiguration>& getRocketMqConfigurations() const {
        return rocketMQConfigurations_;
    }
    void setRocketMqConfigurations(const std::vector<RocketMQConfiguration>& rocketMqConfigurations) {
        rocketMQConfigurations_ = rocketMqConfigurations;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::vector<CloudFunctionConfiguration> cloudFunctionConfigurations_;
    std::vector<RocketMQConfiguration> rocketMQConfigurations_;
};
}  // namespace VolcengineTos
