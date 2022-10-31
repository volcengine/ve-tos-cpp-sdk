#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class GetObjectV2Input {
public:
    GetObjectV2Input() = default;
    ~GetObjectV2Input() = default;
    GetObjectV2Input(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
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
    const std::string& getResponseCacheControl() const {
        return responseCacheControl_;
    }
    void setResponseCacheControl(const std::string& responsecachecontrol) {
        responseCacheControl_ = responsecachecontrol;
    }
    const std::string& getResponseContentDisposition() const {
        return responseContentDisposition_;
    }
    void setResponseContentDisposition(const std::string& responsecontentdisposition) {
        responseContentDisposition_ = responsecontentdisposition;
    }
    const std::string& getResponseContentEncoding() const {
        return responseContentEncoding_;
    }
    void setResponseContentEncoding(const std::string& responsecontentencoding) {
        responseContentEncoding_ = responsecontentencoding;
    }
    const std::string& getResponseContentLanguage() const {
        return responseContentLanguage_;
    }
    void setResponseContentLanguage(const std::string& responsecontentlanguage) {
        responseContentLanguage_ = responsecontentlanguage;
    }
    const std::string& getResponseContentType() const {
        return responseContentType_;
    }
    void setResponseContentType(const std::string& responsecontenttype) {
        responseContentType_ = responsecontenttype;
    }
    time_t getResponseExpires() const {
        return responseExpires_;
    }
    void setResponseExpires(time_t responseexpires) {
        responseExpires_ = responseexpires;
    }
    int64_t getRangeStart() const {
        return rangeStart_;
    }
    void setRangeStart(int64_t rangestart) {
        rangeStart_ = rangestart;
    }
    int64_t getRangeEnd() const {
        return rangeEnd_;
    }
    void setRangeEnd(int64_t rangeend) {
        rangeEnd_ = rangeend;
    }
    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        dataTransferListener_ = datatransferlistener;
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        rateLimiter_ = ratelimiter;
    }

    const std::string& getRange() const {
        return range_;
    }
    void setRange(const std::string& range) {
        range_ = range;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;

    std::string ifMatch_;
    std::time_t ifModifiedSince_ = 0;
    std::string ifNoneMatch_;
    std::time_t ifUnmodifiedSince_ = 0;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey_;
    std::string ssecKeyMd5_;

    std::string responseCacheControl_;
    std::string responseContentDisposition_;
    std::string responseContentEncoding_;
    std::string responseContentLanguage_;
    std::string responseContentType_;
    std::time_t responseExpires_ = 0;

    int64_t rangeStart_ = 0;
    int64_t rangeEnd_ = 0;
    std::string range_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速，详见 4.4.5 章节
};
}  // namespace VolcengineTos
