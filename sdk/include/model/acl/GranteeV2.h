#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class GranteeV2 {
public:
    GranteeV2() = default;
    ~GranteeV2() = default;
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    const std::string& getDisplayName() const {
        return displayName_;
    }
    void setDisplayName(const std::string& displayName) {
        displayName_ = displayName;
    }
    const GranteeType& getType() const {
        return type_;
    }
    void setType(const GranteeType& type) {
        type_ = type;
    }
    CannedType getCanned() const {
        return canned_;
    }
    void setCanned(const CannedType& canned) {
        canned_ = canned;
    }
    const std::string& getStringFormatType() const {
        return GranteeTypetoString[type_];
    }
    const std::string& getStringFormatCanned() const {
        return CannedTypetoString[canned_];
    }

private:
    std::string id_;
    std::string displayName_;
    GranteeType type_ = GranteeType::NotSet;
    CannedType canned_ = CannedType::NotSet;
};
}  // namespace VolcengineTos
