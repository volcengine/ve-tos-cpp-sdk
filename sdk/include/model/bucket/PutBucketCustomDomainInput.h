#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "../src/external/json/json.hpp"
#include "CustomDomainRule.h"

namespace VolcengineTos {
class PutBucketCustomDomainInput {
public:
    explicit PutBucketCustomDomainInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketCustomDomainInput(std::string bucket, const CustomDomainRule& rule)
            : bucket_(std::move(bucket)), rule_(rule) {
    }
    PutBucketCustomDomainInput() = default;
    virtual ~PutBucketCustomDomainInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const CustomDomainRule& getRules() const {
        return rule_;
    }
    void setRules(const CustomDomainRule& rules) {
        rule_ = rules;
    }

    std::string toJsonString() const {
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

private:
    std::string bucket_;
    CustomDomainRule rule_;
};
}  // namespace VolcengineTos
