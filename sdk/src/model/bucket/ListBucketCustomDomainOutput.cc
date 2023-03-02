#include "model/bucket/ListBucketCustomDomainOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::ListBucketCustomDomainOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);

    if (j.contains("CustomDomainRules")) {
        auto rules = j.at("CustomDomainRules");
        for (const auto& r : rules) {
            CustomDomainRule rule_;
            if (r.contains("Domain")) {
                rule_.setDomain(r.at("Domain").get<std::string>());
            }
            if (r.contains("Cname")) {
                rule_.setCname(r.at("Cname").get<std::string>());
            }
            if (r.contains("Forbidden")) {
                rule_.setForbidden(r.at("Forbidden").get<bool>());
            }
            if (r.contains("ForbiddenReason")) {
                rule_.setForbiddenReason(r.at("ForbiddenReason").get<std::string>());
            }
            if (r.contains("CertId")) {
                rule_.setCertId(r.at("CertId").get<std::string>());
            }
            if (r.contains("CertStatus")) {
                auto status = r.at("CertStatus").get<std::string>();
                rule_.setCertStatus(StringtoCertStatusType[status]);
            }
            rules_.emplace_back(rule_);
        }
    }
}
