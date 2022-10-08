#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class UploadPartV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partnumber) {
        partNumber_ = partnumber;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        ssecAlgorithm_ = ssecalgorithm;
    }
    const std::string& getSsecMd5() const {
        return ssecMD5_;
    }
    void setSsecMd5(const std::string& ssecmd5) {
        ssecMD5_ = ssecmd5;
    }
    uint64_t getHashCrc64ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }

private:
    RequestInfo requestInfo_;
    int partNumber_;
    std::string eTag_;
    std::string ssecAlgorithm_;
    std::string ssecMD5_;
    uint64_t hashCrc64ecma_;
};
}  // namespace VolcengineTos
