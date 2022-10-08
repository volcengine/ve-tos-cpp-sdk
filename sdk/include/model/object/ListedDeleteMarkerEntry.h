#pragma once

#include <string>
#include "model/acl/Owner.h"
namespace VolcengineTos {
class [[gnu::deprecated]] ListedDeleteMarkerEntry {
public:
    bool isLatest() const {
        return isLatest_;
    }
    void setIsLatest(bool isLatest) {
        isLatest_ = isLatest;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getLastModified() const {
        return lastModified_;
    }
    void setLastModified(const std::string& lastModified) {
        lastModified_ = lastModified;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }

private:
    bool isLatest_;
    std::string key_;
    std::string lastModified_;
    Owner owner_;
    std::string versionID_;
};
}  // namespace VolcengineTos
