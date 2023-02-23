#pragma once

#include <string>
#include "model/acl/Owner.h"
#include "Type.h"
#include "../src/external/json/json.hpp"
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

    static ListedObjectV2 parseListedObjectV2(const nlohmann::json& object) {
        VolcengineTos::ListedObjectV2 lo;
        if (object.contains("Key")) {
            lo.setKey(object.at("Key").get<std::string>());
        }
        if (object.contains("LastModified")) {
            lo.setLastModified(VolcengineTos::TimeUtils::transLastModifiedStringToTime(
                    object.at("LastModified").get<std::string>()));
        }

        if (object.contains("ETag"))
            lo.setETag(object.at("ETag").get<std::string>());
        if (object.contains("Size"))
            lo.setSize(object.at("Size").get<int64_t>());
        if (object.contains("Owner")) {
            VolcengineTos::Owner owner;
            if (object.at("Owner").contains("ID")) {
                owner.setId(object.at("Owner").at("ID").get<std::string>());
            }
            if (object.at("Owner").contains("DisplayName")) {
                owner.setDisplayName(object.at("Owner").at("DisplayName").get<std::string>());
            }
            lo.setOwner(owner);
        }
        if (object.contains("StorageClass"))
            lo.setStorageClass(VolcengineTos::StringtoStorageClassType[object.at("StorageClass").get<std::string>()]);
        if (object.contains("HashCrc64ecma")) {
            auto hashCrc_ = object.at("HashCrc64ecma").get<std::string>();
            if (!hashCrc_.empty()) {
                lo.setHashCrc64Ecma(stoull(hashCrc_));
            }
        }

        return lo;
    }
    const std::string& getStringFormatStorageClass() const {
        return StorageClassTypetoString[storageClass_];
    }
    const std::string& getStringFormatLastModified() const {
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
