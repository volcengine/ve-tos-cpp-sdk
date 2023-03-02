#pragma once

#include "model/RequestInfo.h"
#include "TagSet.h"
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

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string versionID_;
    TagSet tagSet_;
};
}  // namespace VolcengineTos
