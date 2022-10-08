#pragma once

#include <string>
#include "ObjectAclGrant.h"
#include "ObjectAclRules.h"
namespace VolcengineTos {
class [[gnu::deprecated]] PutObjectAclInput {
public:
    PutObjectAclInput() = default;
    ~PutObjectAclInput() = default;
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getVersionId() const {
        return versionId_;
    }
    void setVersionId(const std::string& versionId) {
        versionId_ = versionId;
    }
    const ObjectAclGrant& getAclGrant() const {
        return aclGrant_;
    }
    void setAclGrant(const ObjectAclGrant& aclGrant) {
        aclGrant_ = aclGrant;
    }
    const ObjectAclRules& getAclRules() const {
        return aclRules_;
    }
    void setAclRules(const ObjectAclRules& aclRules) {
        aclRules_ = aclRules;
    }

private:
    std::string key_;
    std::string versionId_;
    ObjectAclGrant aclGrant_;
    ObjectAclRules aclRules_;
};
}  // namespace VolcengineTos
