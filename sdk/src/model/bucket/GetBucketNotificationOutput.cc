#include "model/bucket/GetBucketNotificationOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::GetBucketNotificationOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("CloudFunctionConfigurations")) {
        nlohmann::json config = j.at("CloudFunctionConfigurations");
        CloudFunctionConfiguration cloudFunctionConfiguration;
        for (auto& r : config) {
            if (r.contains("RuleId")) {
                cloudFunctionConfiguration.setId(r.at("RuleId").get<std::string>());
            }
            if (r.contains("Events")) {
                cloudFunctionConfiguration.setEvents(r.at("Events").get<std::vector<std::string>>());
            }
            if (r.contains("CloudFunction")) {
                cloudFunctionConfiguration.setCloudFunction(r.at("CloudFunction").get<std::string>());
            }
            if (r.contains("Filter")) {
                auto filter = r.at("Filter");
                Filter filter_;
                if (filter.contains("TOSKey")) {
                    auto tosKey = filter.at("TOSKey");
                    FilterKey filterKey_;
                    std::vector<FilterRule> rules_;
                    if (tosKey.contains("FilterRules")) {
                        auto rules = tosKey.at("FilterRules");
                        for (const auto& rule : rules) {
                            FilterRule rule_;
                            if (rule.contains("Name")) {
                                rule_.setName(rule.at("Name").get<std::string>());
                            }
                            if (rule.contains("Value")) {
                                rule_.setValue(rule.at("Value").get<std::string>());
                            }
                            rules_.emplace_back(rule_);
                        }
                    }
                    filterKey_.setRules(rules_);
                    filter_.setKey(filterKey_);
                }
                cloudFunctionConfiguration.setFilter(filter_);
            }
            CloudFunctionConfigurations_.emplace_back(cloudFunctionConfiguration);
        }
    }
    if (j.contains("RocketMQConfigurations")) {
        nlohmann::json config = j.at("RocketMQConfigurations");
        RocketMQConfiguration rocketMqConfiguration;
        for (auto& r : config) {
            if (r.contains("RuleId")) {
                rocketMqConfiguration.setId(r.at("RuleId").get<std::string>());
            }
            if (r.contains("Events")) {
                rocketMqConfiguration.setEvents(r.at("Events").get<std::vector<std::string>>());
            }
            if (r.contains("Role")) {
                rocketMqConfiguration.setRole(r.at("Role").get<std::string>());
            }
            if (r.contains("Filter")) {
                auto filter = r.at("Filter");
                Filter filter_;
                if (filter.contains("TOSKey")) {
                    auto tosKey = filter.at("TOSKey");
                    FilterKey filterKey_;
                    std::vector<FilterRule> rules_;
                    if (tosKey.contains("FilterRules")) {
                        auto rules = tosKey.at("FilterRules");
                        for (const auto& rule : rules) {
                            FilterRule rule_;
                            if (rule.contains("Name")) {
                                rule_.setName(rule.at("Name").get<std::string>());
                            }
                            if (rule.contains("Value")) {
                                rule_.setValue(rule.at("Value").get<std::string>());
                            }
                            rules_.emplace_back(rule_);
                        }
                    }
                    filterKey_.setRules(rules_);
                    filter_.setKey(filterKey_);
                }
                rocketMqConfiguration.setFilter(filter_);
            }
            if (r.contains("RocketMQ")) {
                RocketMQConf rocketMqConf_;
                auto rocketMqConf = r.at("RocketMQ");
                if (rocketMqConf.contains("InstanceId")) {
                    rocketMqConf_.setInstanceId(rocketMqConf.at("InstanceId").get<std::string>());
                }
                if (rocketMqConf.contains("AccessKeyId")) {
                    rocketMqConf_.setAccessKeyId(rocketMqConf.at("AccessKeyId").get<std::string>());
                }
                if (rocketMqConf.contains("Topic")) {
                    rocketMqConf_.setTopic(rocketMqConf.at("Topic").get<std::string>());
                }
                rocketMqConfiguration.setRocketMq(rocketMqConf_);
            }
            rocketMQConfigurations_.emplace_back(rocketMqConfiguration);
        }
    }
}
