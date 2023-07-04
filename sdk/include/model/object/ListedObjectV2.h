#pragma once

#include <string>
#include "model/acl/Owner.h"
#include "Type.h"
#include "utils/BaseUtils.h"
namespace VolcengineTos {
class ListedObjectV2 {
public:
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
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    int64_t getSize() const {
        return size_;
    }
    void setSize(int64_t size) {
        size_ = size;
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
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }

    //    static ListedObjectV2 parseListedObjectV2(const nlohmann::json& object);
    const std::string& getStringFormatStorageClass() const {
        return StorageClassTypetoString[storageClass_];
    }
    std::string getStringFormatLastModified() const {
        auto lastModified = lastModified_;
        return TimeUtils::transLastModifiedTimeToString(lastModified);
    }

private:
    std::string key_;
    std::time_t lastModified_ = 0;
    std::string eTag_;
    int64_t size_ = 0;
    Owner owner_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    uint64_t hashCrc64ecma_ = 0;
};
}  // namespace VolcengineTos
