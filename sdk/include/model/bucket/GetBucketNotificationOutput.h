#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "../src/external/json/json.hpp"
#include "CloudFunctionConfiguration.h"

namespace VolcengineTos {
class GetBucketNotificationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<CloudFunctionConfiguration>& getCloudFunctionConfigurations() const {
        return CloudFunctionConfigurations_;
    }
    void setCloudFunctionConfigurations(const std::vector<CloudFunctionConfiguration>& cloudFunctionConfigurations) {
        CloudFunctionConfigurations_ = cloudFunctionConfigurations;
    }
    void fromJsonString(const std::string& input) {
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
    }

private:
    RequestInfo requestInfo_;
    std::vector<CloudFunctionConfiguration> CloudFunctionConfigurations_;
};
}  // namespace VolcengineTos
