#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "model/acl/Owner.h"
#include "UploadedPartV2.h"
#include "Type.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class ListPartsOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
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
    int getPartNumberMarker() const {
        return partNumberMarker_;
    }
    void setPartNumberMarker(int partnumbermarker) {
        partNumberMarker_ = partnumbermarker;
    }
    int getMaxParts() const {
        return maxParts_;
    }
    void setMaxParts(int maxparts) {
        maxParts_ = maxparts;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(bool istruncated) {
        isTruncated_ = istruncated;
    }

    int getNextPartNumberMarker() const {
        return nextPartNumberMarker_;
    }
    void setNextPartNumberMarker(int nextpartnumbermarker) {
        nextPartNumberMarker_ = nextpartnumbermarker;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageclass) {
        storageClass_ = storageclass;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<UploadedPartV2>& getParts() const {
        return parts_;
    }
    void setParts(const std::vector<UploadedPartV2>& parts) {
        parts_ = parts;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Bucket"))
            j.at("Bucket").get_to(bucket_);
        if (j.contains("Key"))
            j.at("Key").get_to(key_);
        if (j.contains("UploadId"))
            j.at("UploadId").get_to(uploadID_);
        if (j.contains("MaxParts"))
            j.at("MaxParts").get_to(maxParts_);
        if (j.contains("PartNumberMarker"))
            j.at("PartNumberMarker").get_to(partNumberMarker_);
        if (j.contains("IsTruncated"))
            j.at("IsTruncated").get_to(isTruncated_);
        // todo: encodingType API中没有相关参数
        if (j.contains("NextPartNumberMarker"))
            j.at("NextPartNumberMarker").get_to(nextPartNumberMarker_);
        if (j.contains("StorageClass"))
            setStorageClass(StringtoStorageClassType[j.at("StorageClass").get<std::string>()]);

        if (j.contains("Owner")) {
            if (j.at("Owner").contains("ID")) {
                owner_.setId(j.at("Owner").at("ID").get<std::string>());
            }
            if (j.at("Owner").contains("DisplayName")) {
                owner_.setDisplayName(j.at("Owner").at("DisplayName").get<std::string>());
            }
        }

        nlohmann::json parts = j.at("Parts");
        for (auto& part : parts) {
            UploadedPartV2 up;
            if (part.contains("PartNumber"))
                up.setPartNumber(part.at("PartNumber").get<int>());
            if (part.contains("ETag"))
                up.setETag(part.at("ETag").get<std::string>());
            if (part.contains("Size"))
                up.setSize(part.at("Size").get<int>());
            if (part.contains("LastModified")) {
                std::time_t lastModified =
                        TimeUtils::transLastModifiedStringToTime(part.at("LastModified").get<std::string>());
                up.setLastModified(lastModified);
            }
            parts_.emplace_back(up);
        }
    }

private:
    RequestInfo requestInfo_;
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    int partNumberMarker_ = 0;
    int maxParts_ = 0;
    bool isTruncated_ = false;

    int nextPartNumberMarker_ = 0;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    Owner owner_;
    std::vector<UploadedPartV2> parts_;
};
}  // namespace VolcengineTos
