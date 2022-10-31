#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "GrantV2.h"
#include "Owner.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class GetBucketAclOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<GrantV2>& getGrant() const {
        return grant_;
    }
    void setGrant(const std::vector<GrantV2>& grant) {
        grant_ = grant;
    }

    void fromJsonString(const std::string& input){
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



private:
    RequestInfo requestInfo_;
    Owner owner_;
    std::vector<GrantV2> grant_;
};
}  // namespace VolcengineTos
