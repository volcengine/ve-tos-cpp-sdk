#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class PutBucketMirrorBackInput {
public:
    explicit PutBucketMirrorBackInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketMirrorBackInput(std::string bucket, std::vector<MirrorBackRule> rules)
            : bucket_(std::move(bucket)), rules_(std::move(rules)) {
    }
    PutBucketMirrorBackInput() = default;
    virtual ~PutBucketMirrorBackInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<MirrorBackRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<MirrorBackRule>& rules) {
        rules_ = rules;
    }
    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json ruleArray = nlohmann::json::array();
        for (auto& r : rules_) {
            nlohmann::json rule;
            if (!r.getId().empty()) {
                rule["ID"] = r.getId();
            }
            if (r.getCondition().getHttpCode() != 0) {
                rule["Condition"]["HttpCode"] = r.getCondition().getHttpCode();
            }
            nlohmann::json redirect;
            auto redirect_ = r.getRedirect();
            auto redirecTypeStr = RedirectTypetoString[redirect_.getRedirectType()];
            if (!redirecTypeStr.empty()) {
                redirect["RedirectType"] = redirecTypeStr;
            }
            if (redirect_.isFetchSourceOnRedirect()) {
                redirect["FetchSourceOnRedirect"] = redirect_.isFetchSourceOnRedirect();
            }
            if (redirect_.isPassQuery()) {
                redirect["PassQuery"] = redirect_.isPassQuery();
            }
            if (redirect_.isFollowRedirect()) {
                redirect["FollowRedirect"] = redirect_.isFollowRedirect();
            }
            auto mirrorHeader_ = redirect_.getMirrorHeader();
            nlohmann::json mirrorHeader;

            if (mirrorHeader_.isPassAll()) {
                mirrorHeader["PassAll"] = mirrorHeader_.isPassAll();
            }
            if (!mirrorHeader_.getPass().empty()) {
                nlohmann::json pass(mirrorHeader_.getPass());
                mirrorHeader["Pass"] = pass;
            }
            if (!mirrorHeader_.getRemove().empty()) {
                nlohmann::json remove(mirrorHeader_.getRemove());
                mirrorHeader["Remove"] = remove;
            }
            if (!mirrorHeader.empty()) {
                redirect["MirrorHeader"] = mirrorHeader;
            }

            auto publicSource_ = redirect_.getPublicSource();
            nlohmann::json publicSource;
            if (!publicSource_.getSourceEndpoint().getPrimary().empty()) {
                nlohmann::json primary(publicSource_.getSourceEndpoint().getPrimary());
                publicSource["SourceEndpoint"]["Primary"] = primary;
            }
            if (!publicSource_.getSourceEndpoint().getFollower().empty()) {
                nlohmann::json follower(publicSource_.getSourceEndpoint().getFollower());
                publicSource["SourceEndpoint"]["Follower"] = follower;
            }
            if (!publicSource.empty()) {
                redirect["PublicSource"] = publicSource;
            }
            if (!redirect.empty())
                rule["Redirect"] = redirect;
            ruleArray.push_back(rule);
        }
        if (!ruleArray.empty())
            j["Rules"] = ruleArray;
        return j.dump();
    }

private:
    std::string bucket_;
    std::vector<MirrorBackRule> rules_;
};
}  // namespace VolcengineTos
