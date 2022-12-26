#pragma once
#include <string>
#include <utility>
#include "TagSet.h"
namespace VolcengineTos {
class PutObjectTaggingInput {
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
    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json tagSet;
        if (!tagSet_.getTags().empty()) {
            nlohmann::json tagArray = nlohmann::json::array();
            for (auto& t : tagSet_.getTags()) {
                nlohmann::json tag;
                if (!t.getKey().empty()) {
                    tag["Key"] = t.getKey();
                }
                if (!t.getValue().empty()) {
                    tag["Value"] = t.getValue();
                }
                tagArray.push_back(tag);
            }
            if (!tagArray.empty()) {
                j["Tags"] = tagArray;
                tagSet["TagSet"] = j;
            }
        }
        return tagSet.dump();
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    TagSet tagSet_;
};
}  // namespace VolcengineTos
