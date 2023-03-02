#include "model/acl/PutObjectAclV2Input.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::PutObjectAclV2Input::toJsonString() const {
    nlohmann::json j;
    if (!owner_.getId().empty())
        j["Owner"]["ID"] = owner_.getId();
    if (!owner_.getDisplayName().empty())
        j["Owner"]["DisplayName"] = owner_.getDisplayName();
    if (bucketOwnerEntrusted_) {
        j["BucketOwnerEntrusted"] = bucketOwnerEntrusted_;
    }
    nlohmann::json grantArray = nlohmann::json::array();
    for (auto& g : grants_) {
        nlohmann::json grant;
        if (!g.getGrantee().getId().empty())
            grant["Grantee"]["ID"] = g.getGrantee().getId();
        if (!g.getGrantee().getDisplayName().empty())
            grant["Grantee"]["DisplayName"] = g.getGrantee().getDisplayName();
        auto type_ = GranteeTypetoString[g.getGrantee().getType()];
        if (!type_.empty())
            grant["Grantee"]["Type"] = type_;
        auto canned_ = CannedTypetoString[g.getGrantee().getCanned()];
        if (!canned_.empty())
            grant["Grantee"]["Canned"] = canned_;
        auto permission_ = PermissionTypetoString[g.getPermission()];
        if (!permission_.empty())
            grant["Permission"] = permission_;
        grantArray.push_back(grant);
    }
    if (!grantArray.empty())
        j["Grants"] = grantArray;
    return j.dump();
}
