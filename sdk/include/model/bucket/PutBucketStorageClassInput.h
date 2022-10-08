#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class PutBucketStorageClassInput {
public:
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageClass) {
        storageClass_ = storageClass;
    }

private:
    std::string bucket_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
