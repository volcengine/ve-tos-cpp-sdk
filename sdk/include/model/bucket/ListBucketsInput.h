#pragma once

#include "model/GenericInput.h"

namespace VolcengineTos {
class ListBucketsInput : public GenericInput {
public:
    explicit ListBucketsInput(BucketType bucketType) : bucketType_(bucketType) {}
    ListBucketsInput() = default;
    ~ListBucketsInput() = default;
    void setBucketType(BucketType bucketType) {
        bucketType_ = bucketType;
    }
    BucketType getBucketType() const {
        return bucketType_;
    }
private:
    BucketType bucketType_;
};
}  // namespace VolcengineTos
