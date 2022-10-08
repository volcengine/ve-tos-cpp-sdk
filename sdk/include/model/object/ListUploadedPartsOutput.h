#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "model/acl/Owner.h"
#include "UploadedPart.h"
namespace VolcengineTos {
class ListUploadedPartsOutput {
public:
    void fromJsonString(const std::string& input);
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
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
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    int getPartNumberMarker() const {
        return partNumberMarker_;
    }
    void setPartNumberMarker(int partNumberMarker) {
        partNumberMarker_ = partNumberMarker;
    }
    int getNextPartNumberMarker() const {
        return nextPartNumberMarker_;
    }
    void setNextPartNumberMarker(int nextPartNumberMarker) {
        nextPartNumberMarker_ = nextPartNumberMarker;
    }
    int getMaxParts() const {
        return maxParts_;
    }
    void setMaxParts(int maxParts) {
        maxParts_ = maxParts;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(bool isTruncated) {
        isTruncated_ = isTruncated;
    }
    const std::string& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const std::string& storageClass) {
        storageClass_ = storageClass;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<UploadedPart>& getUploadedParts() const {
        return uploadedParts_;
    }
    void setUploadedParts(const std::vector<UploadedPart>& uploadedParts) {
        uploadedParts_ = uploadedParts;
    }

private:
    RequestInfo requestInfo_;
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    int partNumberMarker_ = 0;
    int nextPartNumberMarker_ = 0;
    int maxParts_ = 0;
    bool isTruncated_ = false;
    std::string storageClass_;
    Owner owner_;
    std::vector<UploadedPart> uploadedParts_;
};
}  // namespace VolcengineTos
