#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CreateMultipartUploadOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::string& getSseCustomerAlgorithm() const {
        return sseCustomerAlgorithm_;
    }
    void setSseCustomerAlgorithm(const std::string& sseCustomerAlgorithm) {
        sseCustomerAlgorithm_ = sseCustomerAlgorithm;
        ssecAlgorithm_ = sseCustomerAlgorithm;
    }
    const std::string& getSseCustomerMd5() const {
        return sseCustomerMD5_;
    }
    void setSseCustomerMd5(const std::string& sseCustomerMd5) {
        sseCustomerMD5_ = sseCustomerMd5;
        ssecMD5_ = sseCustomerMd5;
    }
    const std::string& getSseCustomerKey() const {
        return sseCustomerKey_;
    }
    void setSseCustomerKey(const std::string& sseCustomerKey) {
        sseCustomerKey_ = sseCustomerKey;
        ssecKey_ = sseCustomerKey;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecMd5() const {
        return ssecMD5_;
    }
    void setSsecMd5(const std::string& ssecMd5) {
        ssecMD5_ = ssecMd5;
    }
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& ssecKey) {
        ssecKey_ = ssecKey;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingtype) {
        encodingType_ = encodingtype;
    }

private:
    RequestInfo requestInfo_;
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    [[gnu::deprecated]] std::string sseCustomerAlgorithm_;
    [[gnu::deprecated]] std::string sseCustomerMD5_;
    [[gnu::deprecated]] std::string sseCustomerKey_;

    std::string ssecAlgorithm_;
    std::string ssecMD5_;
    std::string ssecKey_;
    std::string encodingType_;
};
}  // namespace VolcengineTos
