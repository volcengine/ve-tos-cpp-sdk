#pragma once

#include <string>
#include "model/acl/Owner.h"
namespace VolcengineTos {
class ListedDeleteMarker {
public:
    bool isLatest() const {
        return isLatest_;
    }
    void setIsLatest(bool islatest) {
        isLatest_ = islatest;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(time_t lastmodified) {
        lastModified_ = lastmodified;
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
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }
    const std::string& getStringFormatLastModified() const {
        auto lastModified = lastModified_;
        return TimeUtils::transLastModifiedTimeToString(lastModified);
    }

private:
    bool isLatest_;
    std::string key_;
    std::time_t lastModified_ = 0;
    Owner owner_;
    std::string versionID_;
};
}  // namespace VolcengineTos
