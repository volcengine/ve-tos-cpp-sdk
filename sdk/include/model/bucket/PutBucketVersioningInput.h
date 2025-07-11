#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "model/GenericInput.h"


namespace VolcengineTos {
class PutBucketVersioningInput : public GenericInput {
public:
    explicit PutBucketVersioningInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketVersioningInput(std::string bucket, VersioningStatusType status)
            : bucket_(std::move(bucket)), status_(status) {
    }
    PutBucketVersioningInput() = default;
    virtual ~PutBucketVersioningInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    VersioningStatusType getStatus() const {
        return status_;
    }
    void setStatus(VersioningStatusType status) {
        status_ = status;
    }

    std::string toJsonString() const;

private:
    std::string bucket_;
    VersioningStatusType status_ = VersioningStatusType::NotSet;
};
}  // namespace VolcengineTos
