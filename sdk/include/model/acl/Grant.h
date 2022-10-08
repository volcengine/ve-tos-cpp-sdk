#pragma once

#include "Grantee.h"
namespace VolcengineTos {
class Grant {
public:
    Grantee getGrantee() const {
        return grantee_;
    }
    void setGrantee(const Grantee& grantee) {
        grantee_ = grantee;
    }
    const std::string& getPermission() const {
        return permission_;
    }
    void setPermission(const std::string& permission) {
        Grant::permission_ = permission;
    }

private:
    Grantee grantee_;
    std::string permission_;
};
}  // namespace VolcengineTos
