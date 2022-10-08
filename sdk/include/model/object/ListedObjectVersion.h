#pragma once

#include <string>
#include "model/acl/Owner.h"
namespace VolcengineTos {
class ListedObjectVersion {
public:
    const std::string& getEtag() const {
        return etag_;
    }
    void setEtag(const std::string& etag) {
        etag_ = etag;
    }
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
    int64_t getSize() const {
        return size_;
    }
    void setSize(int64_t size) {
        size_ = size;
    }
    const std::string& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const std::string& storageClass) {
        storageClass_ = storageClass;
    }
    const std::string& getType() const {
        return type_;
    }
    void setType(const std::string& type) {
        type_ = type;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }

private:
    std::string etag_;
    bool isLatest_;
    std::string key_;
    std::string lastModified_;
    Owner owner_;
    int64_t size_;
    std::string storageClass_;
    std::string type_;
    std::string versionID_;
};
}  // namespace VolcengineTos
