#pragma once

#include <string>
#include <vector>
#include "Type.h"
#include "Owner.h"
#include "GrantV2.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class PutObjectAclV2Input {
public:
    PutObjectAclV2Input(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    PutObjectAclV2Input(std::string bucket, std::string key, ACLType acl)
            : bucket_(std::move(bucket)), key_(std::move(key)), acl_(acl) {
    }
    PutObjectAclV2Input() = default;
    ~PutObjectAclV2Input() = default;

    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getVersionId() const {
        return versionId_;
    }
    void setVersionId(const std::string& versionid) {
        versionId_ = versionid;
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
    bool isBucketOwnerEntrusted() const {
        return bucketOwnerEntrusted_;
    }
    void setBucketOwnerEntrusted(bool bucketOwnerEntrusted) {
        bucketOwnerEntrusted_ = bucketOwnerEntrusted;
    }
    std::string toJsonString() const {
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

private:
    std::string bucket_;
    std::string key_;
    std::string versionId_;

    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;
    Owner owner_;
    std::vector<GrantV2> grants_;
    bool bucketOwnerEntrusted_ = false;
};
}  // namespace VolcengineTos
