#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutObjectV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& eTag) {
        eTag_ = eTag;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMD5_ = ssecKeyMd5;
    }
    uint64_t getHashCrc64ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64ecma(uint64_t hashCrc64ecma) {
        hashCrc64ecma_ = hashCrc64ecma;
    }
    const std::string& getCallbackResult() const {
        return callbackResult_;
    }
    void setCallbackResult(const std::string& callbackResult) {
        callbackResult_ = callbackResult;
    }

private:
    RequestInfo requestInfo_;
    std::string eTag_;
    std::string ssecAlgorithm_;
    std::string ssecKeyMD5_;
    std::string versionID_;
    uint64_t hashCrc64ecma_;
    std::string callbackResult_;
};
}  // namespace VolcengineTos
