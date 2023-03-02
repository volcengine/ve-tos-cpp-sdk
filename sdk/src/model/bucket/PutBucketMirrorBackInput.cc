#include "../src/external/json/json.hpp"
#include "model/bucket/PutBucketMirrorBackInput.h"

std::string VolcengineTos::PutBucketMirrorBackInput::toJsonString() const {
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
