#pragma once

#include "model/RequestInfo.h"
#include "Type.h"
namespace VolcengineTos {
class HeadBucketV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getRegion() const {
        return region_;
    }
    void setRegion(const std::string& region) {
        region_ = region;
    }
    const StorageClassType& getStorageClass() const {
        return storageClass_;
    }
    const std::string& getStringFormatStorageClass() const {
        return StorageClassTypetoString[storageClass_];
    }
    void setStorageClass(const StorageClassType& storageClass) {
        storageClass_ = storageClass;
    }
    AzRedundancyType getAzRedundancy() const {
        return azRedundancy_;
    }
    void setAzRedundancy(AzRedundancyType azRedundancy) {
        azRedundancy_ = azRedundancy;
    }
    const std::string& getStringFormatAzRedundancy() const {
        return AzRedundancyTypetoString[azRedundancy_];
    }
    BucketType getBucketType() const {
        return bucketType_;
    }
    void setBucketType(BucketType bucketType) {
        bucketType_ = bucketType;
    }

private:
    RequestInfo requestInfo_;
    std::string region_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    AzRedundancyType azRedundancy_ = AzRedundancyType::NotSet;
    BucketType bucketType_;
};
}  // namespace VolcengineTos
