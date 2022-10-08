#pragma once

#include "CompleteMultipartUploadOutput.h"
namespace VolcengineTos {
class UploadFileV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
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
        return uploadId_;
    }
    void setUploadId(const std::string& uploadid) {
        uploadId_ = uploadid;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    const std::string& getLocation() const {
        return location_;
    }
    void setLocation(const std::string& location) {
        location_ = location;
    }
    const std::string& getVersionId() const {
        return versionId_;
    }
    void setVersionId(const std::string& versionid) {
        versionId_ = versionid;
    }
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        ssecAlgorithm_ = ssecalgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        ssecKeyMD5_ = sseckeymd5;
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
    std::string uploadId_;
    std::string eTag_;
    std::string location_;
    std::string versionId_;
    uint64_t hashCrc64ecma_ = 0;
    std::string ssecAlgorithm_;
    std::string ssecKeyMD5_;
    std::string encodingType_;
};
}  // namespace VolcengineTos
