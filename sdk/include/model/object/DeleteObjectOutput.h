#pragma once

#include "model/RequestInfo.h"

namespace VolcengineTos {
class DeleteObjectOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    bool isDeleteMarker() const {
        return deleteMarker_;
    }
    void setDeleteMarker(bool deleteMarker) {
        deleteMarker_ = deleteMarker;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }
    const std::string& getTrashPath() const {
        return trashPath_;
    }
    void setTrashPath(const std::string& trashPath) {
        trashPath_ = trashPath;
    }

private:
    RequestInfo requestInfo_;
    bool deleteMarker_;
    std::string versionID_;
    std::string trashPath_;
};
}  // namespace VolcengineTos
