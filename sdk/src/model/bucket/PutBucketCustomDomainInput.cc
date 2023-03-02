#include "model/bucket/PutBucketCustomDomainInput.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

std::string VolcengineTos::PutBucketCustomDomainInput::toJsonString() const {
    nlohmann::json j;
    nlohmann::json rule;
    if (!rule_.getCertId().empty()) {
        rule["CertId"] = rule_.getCertId();
    }
    if (rule_.getCertStatus() != CertStatusType::NotSet) {
        rule["CertStatus"] = CertStatusTypetoString[rule_.getCertStatus()];
    }
    if (!rule_.getDomain().empty()) {
        rule["Domain"] = rule_.getDomain();
    }
    if (rule_.isForbidden()) {
        rule["Forbidden"] = rule_.isForbidden();
    }
    if (!rule_.getForbiddenReason().empty()) {
        rule["ForbiddenReason"] = rule_.getForbiddenReason();
    }
    if (!rule.empty()) {
        j["CustomDomainRule"] = rule;
    }
    return j.dump();
}
