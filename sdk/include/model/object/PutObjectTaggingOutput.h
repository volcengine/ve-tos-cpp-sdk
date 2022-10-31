#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutObjectTaggingOutput {
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

private:
    RequestInfo requestInfo_;
    std::string versionID_;
};
}  // namespace VolcengineTos
