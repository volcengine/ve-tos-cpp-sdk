#pragma once

#include "model/RequestInfo.h"
#include "GetObjectBasicOutput.h"

namespace VolcengineTos {
class FetchObjectOutput {
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
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& eTag) {
        eTag_ = eTag;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMd5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMd5_ = ssecKeyMd5;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string versionID_;
    std::string eTag_;
    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKeyMd5_;
};
}  // namespace VolcengineTos
