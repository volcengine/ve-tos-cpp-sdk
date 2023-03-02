#include "model/acl/GetBucketAclOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::GetBucketAclOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);

    if (j.contains("Owner")) {
        if (j.at("Owner").contains("ID")) {
            owner_.setId(j.at("Owner").at("ID").get<std::string>());
        }
        if (j.at("Owner").contains("DisplayName")) {
            owner_.setDisplayName(j.at("Owner").at("DisplayName").get<std::string>());
        }
    }
    nlohmann::json grants = j.at("Grants");
    for (auto& grant : grants) {
        GrantV2 g;
        GranteeV2 ge;
        if (grant.contains("Grantee")) {
            auto grantee = grant.at("Grantee");
            if (grantee.contains("ID"))
                ge.setId(grantee.at("ID").get<std::string>());
            if (grantee.contains("DisplayName"))
                ge.setDisplayName(grantee.at("DisplayName").get<std::string>());
            if (grantee.contains("Type"))
                ge.setType(StringtoGranteeType[grantee.at("Type").get<std::string>()]);
            if (grantee.contains("Canned"))
                ge.setCanned(StringtoCannedType[grantee.at("Canned").get<std::string>()]);
        }
        g.setGrantee(ge);
        if (grant.contains("Permission"))
            g.setPermission(StringtoPermissionType[grant.at("Permission").get<std::string>()]);
        grant_.emplace_back(g);
    }
}
