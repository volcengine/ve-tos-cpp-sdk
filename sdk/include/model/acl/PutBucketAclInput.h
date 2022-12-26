#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "Owner.h"
#include "GrantV2.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class PutBucketAclInput {
public:
    explicit PutBucketAclInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketAclInput() = default;
    virtual ~PutBucketAclInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    ACLType getAcl() const {
        return acl_;
    }
    void setAcl(ACLType acl) {
        acl_ = acl;
    }
    const std::string& getGrantFullControl() const {
        return grantFullControl_;
    }
    void setGrantFullControl(const std::string& grantfullcontrol) {
        grantFullControl_ = grantfullcontrol;
    }
    const std::string& getGrantRead() const {
        return grantRead_;
    }
    void setGrantRead(const std::string& grantread) {
        grantRead_ = grantread;
    }
    const std::string& getGrantReadAcp() const {
        return grantReadAcp_;
    }
    void setGrantReadAcp(const std::string& grantreadacp) {
        grantReadAcp_ = grantreadacp;
    }
    const std::string& getGrantWriteAcp() const {
        return grantWriteAcp_;
    }
    void setGrantWriteAcp(const std::string& grantwriteacp) {
        grantWriteAcp_ = grantwriteacp;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<GrantV2>& getGrants() const {
        return grants_;
    }
    void setGrants(const std::vector<GrantV2>& grants) {
        grants_ = grants;
    }
    const std::string& getGrantWrite() const {
        return grantWrite_;
    }
    void setGrantWrite(const std::string& grantWrite) {
        grantWrite_ = grantWrite;
    }
    std::string toJsonString() const {
        nlohmann::json j;
        if (!owner_.getId().empty())
            j["Owner"]["ID"] = owner_.getId();
        if (!owner_.getDisplayName().empty())
            j["Owner"]["DisplayName"] = owner_.getDisplayName();
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

private:
    std::string bucket_;

    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWrite_;
    std::string grantWriteAcp_;
    Owner owner_;
    std::vector<GrantV2> grants_;
};
}  // namespace VolcengineTos
