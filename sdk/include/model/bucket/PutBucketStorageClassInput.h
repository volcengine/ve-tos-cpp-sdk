#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/GenericInput.h"


namespace VolcengineTos {
class PutBucketStorageClassInput : public GenericInput {
public:
    explicit PutBucketStorageClassInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketStorageClassInput(std::string bucket, StorageClassType storageClass)
            : bucket_(std::move(bucket)), storageClass_(storageClass) {
    }
    PutBucketStorageClassInput() = default;
    virtual ~PutBucketStorageClassInput() = default;
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
