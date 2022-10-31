#pragma once

#include <vector>
#include "../src/external/json/json.hpp"
#include "model/RequestInfo.h"
#include "CORSRule.h"
namespace VolcengineTos {
class GetBucketCORSOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::vector<CORSRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<CORSRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        nlohmann::json rules = j.at("CORSRules");
        for (auto& r : rules) {
            CORSRule rule;
            int maxAgeSeconds_ = 0;
            if (r.contains("AllowedOrigins")) {
                rule.setAllowedOrigins(r.at("AllowedOrigins").get<std::vector<std::string>>());
            }
            if (r.contains("AllowedMethods")) {
                rule.setAllowedMethods(r.at("AllowedMethods").get<std::vector<std::string>>());
            }
            if (r.contains("AllowedHeaders")) {
                rule.setAllowedHeaders(r.at("AllowedHeaders").get<std::vector<std::string>>());
            }
            if (r.contains("ExposeHeaders")) {
                rule.setExposeHeaders(r.at("ExposeHeaders").get<std::vector<std::string>>());
            }
            if (r.contains("MaxAgeSeconds")) {
                rule.setMaxAgeSeconds(r.at("MaxAgeSeconds").get<int>());
            }
            rules_.push_back(rule);
        }
    }

private:
    RequestInfo requestInfo_;
    std::vector<CORSRule> rules_;
};
}  // namespace VolcengineTos
