#pragma once

#include <string>
#include <utility>
#include "Type.h"
namespace VolcengineTos {
class HeadObjectV2Input {
public:
    HeadObjectV2Input(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    HeadObjectV2Input(std::string bucket, std::string key, std::string versionId)
            : bucket_(std::move(bucket)), key_(std::move(key)), versionID_(std::move(versionId)) {
    }
    HeadObjectV2Input() = default;
    ~HeadObjectV2Input() = default;
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
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }
    const std::string& getIfMatch() const {
        return ifMatch_;
    }
    void setIfMatch(const std::string& ifmatch) {
        ifMatch_ = ifmatch;
    }
    time_t getIfModifiedSince() const {
        return ifModifiedSince_;
    }
    void setIfModifiedSince(time_t ifmodifiedsince) {
        ifModifiedSince_ = ifmodifiedsince;
    }
    const std::string& getIfNoneMatch() const {
        return ifNoneMatch_;
    }
    void setIfNoneMatch(const std::string& ifnonematch) {
        ifNoneMatch_ = ifnonematch;
    }
    time_t getIfUnmodifiedSince() const {
        return ifUnmodifiedSince_;
    }
    void setIfUnmodifiedSince(time_t ifunmodifiedsince) {
        ifUnmodifiedSince_ = ifunmodifiedsince;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        ssecAlgorithm_ = ssecalgorithm;
    }
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& sseckey) {
        ssecKey_ = sseckey;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMd5_;
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        ssecKeyMd5_ = sseckeymd5;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    std::string ifMatch_;
    std::time_t ifModifiedSince_ = 0;
    std::string ifNoneMatch_;
    std::time_t ifUnmodifiedSince_ = 0;
    std::string ssecAlgorithm_;
    std::string ssecKey_;
    std::string ssecKeyMd5_;
};
}  // namespace VolcengineTos
