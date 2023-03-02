#include "model/bucket/PutBucketWebsiteInput.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PutBucketWebsiteInput::toJsonString() const {
    nlohmann::json j;

    nlohmann::json re;
    if (redirectAllRequestsTo_ != nullptr) {
        auto re_ = *redirectAllRequestsTo_;
        if (!re_.getHostName().empty()) {
            re["HostName"] = re_.getHostName();
        }
        if (!re_.getProtocol().empty()) {
            re["Protocol"] = re_.getProtocol();
        }
    }
    if (!re.empty()) {
        j["RedirectAllRequestsTo"] = re;
    }
    nlohmann::json in;
    if (indexDocument_ != nullptr) {
        auto in_ = *indexDocument_;
        if (!in_.getSuffix().empty()) {
            in["Suffix"] = in_.getSuffix();
        }
        in["ForbiddenSubDir"] = in_.isForbiddenSubDir();
    }
    if (!in.empty()) {
        j["IndexDocument"] = in;
    }
    nlohmann::json er;
    if (errorDocument_ != nullptr) {
        auto er_ = *errorDocument_;
        if (!er_.getKey().empty()) {
            er["Key"] = er_.getKey();
        }
    }
    if (!er.empty()) {
        j["ErrorDocument"] = er;
    }

    nlohmann::json rule;
    nlohmann::json ruleArray = nlohmann::json::array();
    for (auto& r : routingRules_) {
        nlohmann::json rule;
        const auto& condition_ = r.getCondition();
        nlohmann::json condition;
        if (!condition_.getKeyPrefixEquals().empty()) {
            condition["KeyPrefixEquals"] = condition_.getKeyPrefixEquals();
        }
        if (condition_.getHttpErrorCodeReturnedEquals() != 0) {
            condition["HttpErrorCodeReturnedEquals"] = condition_.getHttpErrorCodeReturnedEquals();
        }
        if (!condition.empty()) {
            rule["Condition"] = condition;
        }
        const auto& redirect_ = r.getRedirect();
        nlohmann::json redirect;
        if (!redirect_.getHostName().empty()) {
            redirect["HostName"] = redirect_.getHostName();
        }
        if (redirect_.getHttpRedirectCode() != 0) {
            redirect["HttpRedirectCode"] = redirect_.getHttpRedirectCode();
        }
        if (redirect_.getProtocolType() != ProtocolType::NotSet) {
            redirect["Protocol"] = ProtocolTypetoString[redirect_.getProtocolType()];
        }
        if (!redirect_.getReplaceKeyPrefixWith().empty()) {
            redirect["ReplaceKeyPrefixWith"] = redirect_.getReplaceKeyPrefixWith();
        }
        if (!redirect_.getReplaceKeyWith().empty()) {
            redirect["ReplaceKeyWith"] = redirect_.getReplaceKeyWith();
        }
        if (!redirect.empty()) {
            rule["Redirect"] = redirect;
        }
        ruleArray.emplace_back(rule);
    }
    if (!ruleArray.empty()) {
        j["RoutingRules"] = ruleArray;
    }

    return j.dump();
}
