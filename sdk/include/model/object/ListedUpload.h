#pragma once

#include <string>
#include "model/acl/Owner.h"
#include "Type.h"
namespace VolcengineTos {
class ListedUpload {
public:
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadid) {
        uploadID_ = uploadid;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageclass) {
        storageClass_ = storageclass;
    }
    time_t getInitiated() const {
        return initiated_;
    }
    void setInitiated(time_t initiated) {
        initiated_ = initiated;
    }

private:
    std::string key_;
    std::string uploadID_;
    Owner owner_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    std::time_t initiated_ = 0;
};
}  // namespace VolcengineTos
