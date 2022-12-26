#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "../src/external/json/json.hpp"
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
    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json configArray = nlohmann::json::array();
        for (auto& c : CloudFunctionConfigurations_) {
            nlohmann::json config;
            if (!c.getId().empty()) {
                config["RuleId"] = c.getId();
            }
            if (!c.getEvents().empty()) {
                nlohmann::json events(c.getEvents());
                config["Events"] = events;
            }
            if (!c.getCloudFunction().empty()) {
                config["CloudFunction"] = c.getCloudFunction();
            }
            const auto& filter_ = c.getFilter();
            const auto& key_ = filter_.getKey();
            nlohmann::json filter;
            nlohmann::json key;
            nlohmann::json filterArray = nlohmann::json::array();
            for (auto& r : key_.getRules()) {
                nlohmann::json rule;
                if (!r.getName().empty()) {
                    rule["Name"] = r.getName();
                }
                if (!r.getValue().empty()) {
                    rule["Value"] = r.getValue();
                }
                if (!rule.empty()) {
                    filterArray.emplace_back(rule);
                }
            }
            if (!filterArray.empty()) {
                key["FilterRules"] = filterArray;
            }
            if (!key.empty()) {
                filter["TOSKey"] = key;
            }
            if (!filter.empty()) {
                config["Filter"] = filter;
            }
            configArray.push_back(config);
        }

        if (!configArray.empty())
            j["CloudFunctionConfigurations"] = configArray;
        return j.dump();
    }

private:
    std::string bucket_;
    std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations_;
};
}  // namespace VolcengineTos
