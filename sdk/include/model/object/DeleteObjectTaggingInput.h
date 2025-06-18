#pragma once

#include <string>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteObjectTaggingInput : public GenericInput {
public:
    DeleteObjectTaggingInput(std::string bucket, std::string key, std::string versionId)
            : bucket_(std::move(bucket)), key_(std::move(key)), versionID_(std::move(versionId)) {
    }
    DeleteObjectTaggingInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    DeleteObjectTaggingInput() = default;
    virtual ~DeleteObjectTaggingInput() = default;
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
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
};
}  // namespace VolcengineTos
