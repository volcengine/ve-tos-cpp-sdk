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
    void setStorageClass(const StorageClassType& storageClass) {
        storageClass_ = storageClass;
    }

private:
    RequestInfo requestInfo_;
    std::string region_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
