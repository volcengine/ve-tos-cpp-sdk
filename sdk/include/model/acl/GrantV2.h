#pragma once

#include "GranteeV2.h"
#include "Type.h"
namespace VolcengineTos {
class GrantV2 {
public:
    GranteeV2 getGrantee() const {
        return grantee_;
    }
    void setGrantee(const GranteeV2& grantee) {
        grantee_ = grantee;
    }
    const PermissionType& getPermission() const {
        return permission_;
    }
    void setPermission(const PermissionType& permission) {
        permission_ = permission;
    }

private:
    GranteeV2 grantee_;
    PermissionType permission_ = PermissionType::NotSet;
};
}  // namespace VolcengineTos
