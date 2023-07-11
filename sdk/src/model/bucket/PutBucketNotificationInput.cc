#include "model/bucket/PutBucketNotificationInput.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PutBucketNotificationInput::toJsonString() const {
    nlohmann::json j;
    nlohmann::json configArray = nlohmann::json::array();
    for (auto& c : cloudFunctionConfigurations_) {
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

    nlohmann::json mqConfigArray = nlohmann::json::array();
    for (auto& c : rocketMQConfigurations_) {
        nlohmann::json config;
        if (!c.getId().empty()) {
            config["RuleId"] = c.getId();
        }
        if (!c.getRole().empty()) {
            config["Role"] = c.getRole();
        }
        if (!c.getEvents().empty()) {
            nlohmann::json events(c.getEvents());
            config["Events"] = events;
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
        if (!c.getRocketMq().getAccessKeyId().empty()) {
            nlohmann::json accessKeyId(c.getRocketMq().getAccessKeyId());
            config["RocketMQ"]["AccessKeyId"] = accessKeyId;
        }
        if (!c.getRocketMq().getInstanceId().empty()) {
            nlohmann::json instanceId(c.getRocketMq().getInstanceId());
            config["RocketMQ"]["InstanceId"] = instanceId;
        }
        if (!c.getRocketMq().getTopic().empty()) {
            nlohmann::json topic(c.getRocketMq().getTopic());
            config["RocketMQ"]["Topic"] = topic;
        }
        mqConfigArray.push_back(config);
    }

    if (!mqConfigArray.empty())
        j["RocketMQConfigurations"] = mqConfigArray;

    return j.dump();
}
