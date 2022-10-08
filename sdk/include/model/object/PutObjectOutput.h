#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class [[gnu::deprecated]] PutObjectOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getEtag() const {
        return etag_;
    }
    void setEtag(const std::string& etag) {
        etag_ = etag;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }
    const std::string& getSseCustomerAlgorithm() const {
        return sseCustomerAlgorithm_;
    }
    void setSseCustomerAlgorithm(const std::string& sseCustomerAlgorithm) {
        sseCustomerAlgorithm_ = sseCustomerAlgorithm;
    }
    const std::string& getSseCustomerKeyMd5() const {
        return sseCustomerKeyMD5_;
    }
    void setSseCustomerKeyMd5(const std::string& sseCustomerKeyMd5) {
        sseCustomerKeyMD5_ = sseCustomerKeyMd5;
    }
    const std::string& getSseCustomerKey() const {
        return sseCustomerKey_;
    }
    void setSseCustomerKey(const std::string& sseCustomerKey) {
        sseCustomerKey_ = sseCustomerKey;
    }
    const std::string& getCrc64() const {
        return crc64_;
    }
    void setCrc64(const std::string& crc64) {
        crc64_ = crc64;
    }

private:
    RequestInfo requestInfo_;
    std::string etag_;
    std::string versionID_;
    std::string sseCustomerAlgorithm_;
    std::string sseCustomerKeyMD5_;
    std::string sseCustomerKey_;
    std::string crc64_;
};
}  // namespace VolcengineTos
