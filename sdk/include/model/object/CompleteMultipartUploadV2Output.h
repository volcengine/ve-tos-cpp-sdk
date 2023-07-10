#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CompleteMultipartUploadV2Output {
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
        return versionID_;
    }
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }
    uint64_t getHashCrc64ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }
    const std::string& getCallbackResult() const {
        return callbackResult_;
    }
    void setCallbackResult(const std::string& callbackResult) {
        callbackResult_ = callbackResult;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string bucket_;
    std::string key_;
    std::string eTag_;
    std::string location_;
    std::string versionID_;
    uint64_t hashCrc64ecma_ = 0;
    std::string callbackResult_;
};
}  // namespace VolcengineTos
