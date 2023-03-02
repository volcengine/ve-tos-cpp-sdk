#include "model/bucket/PutBucketCORSInput.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PutBucketCORSInput::toJsonString() const {
    nlohmann::json j;
    nlohmann::json ruleArray = nlohmann::json::array();
    for (auto& r : rules_) {
        nlohmann::json rule;
        if (!r.getAllowedOrigins().empty()) {
            nlohmann::json ao(r.getAllowedOrigins());
            rule["AllowedOrigins"] = ao;
        }
        if (!r.getAllowedMethods().empty()) {
            nlohmann::json am(r.getAllowedMethods());
            rule["AllowedMethods"] = am;
        }
        if (!r.getAllowedHeaders().empty()) {
            nlohmann::json ah(r.getAllowedHeaders());
            rule["AllowedHeaders"] = ah;
        }
        if (!r.getExposeHeaders().empty()) {
            nlohmann::json eh(r.getExposeHeaders());
            rule["ExposeHeaders"] = eh;
        }
        if (r.getMaxAgeSeconds() != 0) {
            rule["MaxAgeSeconds"] = r.getMaxAgeSeconds();
        }
        ruleArray.push_back(rule);
    }
    if (!ruleArray.empty())
        j["CORSRules"] = ruleArray;
    return j.dump();
}
