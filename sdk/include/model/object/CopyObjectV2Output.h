#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CopyObjectV2Output {
public:
    void fromJsonString(const std::string& copy);
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(time_t lastmodified) {
        lastModified_ = lastmodified;
    }
    const std::string& getCopySourceVersionId() const {
        return copySourceVersionID_;
    }
    void setCopySourceVersionId(const std::string& copysourceversionid) {
        copySourceVersionID_ = copysourceversionid;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }

private:
    RequestInfo requestInfo_;
    std::string eTag_;
    std::time_t lastModified_ = 0;
    std::string copySourceVersionID_;
    std::string versionID_;
};
}  // namespace VolcengineTos
