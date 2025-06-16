#pragma once
#include <string>
#include <utility>
#include "TagSet.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class PutObjectTaggingInput : public GenericInput {
public:
    PutObjectTaggingInput() = default;
    virtual ~PutObjectTaggingInput() = default;
    PutObjectTaggingInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    PutObjectTaggingInput(std::string bucket, std::string key, const TagSet& tagSet)
            : bucket_(std::move(bucket)), key_(std::move(key)), tagSet_(tagSet) {
    }
    PutObjectTaggingInput(std::string bucket, std::string key, std::string versionId, const TagSet& tagSet)
            : bucket_(std::move(bucket)), key_(std::move(key)), versionID_(std::move(versionId)), tagSet_(tagSet) {
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
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }
    const TagSet& getTagSet() const {
        return tagSet_;
    }
    void setTagSet(const TagSet& tagSet) {
        tagSet_ = tagSet;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    TagSet tagSet_;
};
}  // namespace VolcengineTos
