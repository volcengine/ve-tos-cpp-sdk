#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "model/acl/Owner.h"
#include "UploadedPartV2.h"
#include "Type.h"

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

    void fromJsonString(const std::string& input);

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
