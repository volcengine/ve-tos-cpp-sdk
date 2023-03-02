#include "model/bucket/GetBucketCORSOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::GetBucketCORSOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("CORSRules")) {
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
}
