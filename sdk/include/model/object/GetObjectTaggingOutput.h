#pragma once

#include "model/RequestInfo.h"

namespace VolcengineTos {
class GetObjectTaggingOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
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

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("TagSet")) {
            nlohmann::json tags = j.at("TagSet");
            if (tags.contains("Tags")) {
                auto tag = tags.at("Tags");
                std::vector<Tag> tags_;
                for (auto& t : tag) {
                    Tag tag_;
                    if (t.contains("Key")) {
                        tag_.setKey(t.at("Key").get<std::string>());
                    }
                    if (t.contains("Value")) {
                        tag_.setValue(t.at("Value").get<std::string>());
                    }
                    tags_.emplace_back(tag_);
                }
                tagSet_.setTags(tags_);
            }
        }
    }

private:
    RequestInfo requestInfo_;
    std::string versionID_;
    TagSet tagSet_;
};
}  // namespace VolcengineTos
