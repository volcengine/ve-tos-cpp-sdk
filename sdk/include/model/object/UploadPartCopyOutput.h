#pragma once

#include <utility>

#include "model/RequestInfo.h"
namespace VolcengineTos {
namespace inner {
class InnerUploadPartCopyOutput {
public:
    void fromJsonString(const std::string& input);
    const std::string& getEtag() const {
        return etag_;
    }
    const std::string& getLastModified() const {
        return lastModified_;
    }

private:
    std::string etag_;
    std::string lastModified_;
};
}  // namespace inner
class [[gnu::deprecated]] UploadPartCopyOutput {
public:
    UploadPartCopyOutput() = default;
    ~UploadPartCopyOutput() = default;
    UploadPartCopyOutput(RequestInfo requestInfo, std::string versionId, std::string sourceVersionId, int partNumber,
                         std::string etag, std::string lastModified)
            : requestInfo_(std::move(requestInfo)),
              versionID_(std::move(versionId)),
              sourceVersionID_(std::move(sourceVersionId)),
              partNumber_(partNumber),
              etag_(std::move(etag)),
              lastModified_(std::move(lastModified)) {
    }

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
    const std::string& getSourceVersionId() const {
        return sourceVersionID_;
    }
    void setSourceVersionId(const std::string& sourceVersionId) {
        sourceVersionID_ = sourceVersionId;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partNumber) {
        partNumber_ = partNumber;
    }
    const std::string& getEtag() const {
        return etag_;
    }
    void setEtag(const std::string& etag) {
        etag_ = etag;
    }
    const std::string& getLastModified() const {
        return lastModified_;
    }
    void setLastModified(const std::string& lastModified) {
        lastModified_ = lastModified;
    }
    const std::string& getCrc64() const {
        return crc64_;
    }
    void setCrc64(const std::string& crc64) {
        crc64_ = crc64;
    }

private:
    RequestInfo requestInfo_;
    std::string versionID_;
    std::string sourceVersionID_;
    int partNumber_ = 0;
    std::string etag_;
    std::string lastModified_;
    std::string crc64_;
};
}  // namespace VolcengineTos
