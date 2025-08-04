#pragma once

#include <string>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class AppendObjectV2Input : public GenericInput {
public:
    AppendObjectV2Input(std::string bucket, std::string key, std::shared_ptr<std::iostream> content, int64_t offset)
            : bucket_(std::move(bucket)), key_(std::move(key)), offset_(offset), content_(std::move(content)) {
    }
    AppendObjectV2Input() = default;
    ~AppendObjectV2Input() = default;

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
    int64_t getOffset() const {
        return offset_;
    }
    void setOffset(int64_t offset) {
        offset_ = offset;
    }
    const std::shared_ptr<std::iostream>& getContent() const {
        return content_;
    }
    void setContent(const std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentlength) {
        contentLength_ = contentlength;
    }
    const std::string& getContentType() const {
        return contentType_;
    }
    void setContentType(const std::string& contenttype) {
        contentType_ = contenttype;
    }
    const std::string& getCacheControl() const {
        return cacheControl_;
    }
    void setCacheControl(const std::string& cachecontrol) {
        cacheControl_ = cachecontrol;
    }
    const std::string& getContentDisposition() const {
        return contentDisposition_;
    }
    void setContentDisposition(const std::string& contentdisposition) {
        contentDisposition_ = contentdisposition;
    }
    const std::string& getContentEncoding() const {
        return contentEncoding_;
    }
    void setContentEncoding(const std::string& contentencoding) {
        contentEncoding_ = contentencoding;
    }
    const std::string& getContentLanguage() const {
        return contentLanguage_;
    }
    void setContentLanguage(const std::string& contentlanguage) {
        contentLanguage_ = contentlanguage;
    }
    time_t getExpires() const {
        return expires_;
    }
    void setExpires(time_t expires) {
        expires_ = expires;
    }
    ACLType getAcl() const {
        return acl_;
    }
    void setAcl(ACLType acl) {
        acl_ = acl;
    }
    const std::string& getGrantFullControl() const {
        return grantFullControl_;
    }
    void setGrantFullControl(const std::string& grantfullcontrol) {
        grantFullControl_ = grantfullcontrol;
    }
    const std::string& getGrantRead() const {
        return grantRead_;
    }
    void setGrantRead(const std::string& grantread) {
        grantRead_ = grantread;
    }
    const std::string& getGrantReadAcp() const {
        return grantReadAcp_;
    }
    void setGrantReadAcp(const std::string& grantreadacp) {
        grantReadAcp_ = grantreadacp;
    }
    const std::string& getGrantWriteAcp() const {
        return grantWriteAcp_;
    }
    void setGrantWriteAcp(const std::string& grantwriteacp) {
        grantWriteAcp_ = grantwriteacp;
    }
    const std::map<std::string, std::string>& getMeta() const {
        return meta_;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }
    const std::string& getWebsiteRedirectLocation() const {
        return websiteRedirectLocation_;
    }
    void setWebsiteRedirectLocation(const std::string& websiteredirectlocation) {
        websiteRedirectLocation_ = websiteredirectlocation;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageclass) {
        storageClass_ = storageclass;
    }
    uint64_t getPreHashCrc64Ecma() const {
        return preHashCrc64ecma_;
    }
    void setPreHashCrc64Ecma(uint64_t prehashcrc64ecma) {
        preHashCrc64ecma_ = prehashcrc64ecma;
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
    int64_t getTrafficLimit() const {
        return trafficLimit_;
    }
    void setTrafficLimit(int64_t trafficLimit) {
        trafficLimit_ = trafficLimit;
    }
    const std::string& getNotificationCustomParameters() const {
        return notificationCustomParameters_;
    }
    void setNotificationCustomParameters(const std::string& notificationCustomParameters) {
        notificationCustomParameters_ = notificationCustomParameters;
    }

private:
    std::string bucket_;
    std::string key_;
    int64_t offset_ = 0;
    std::shared_ptr<std::iostream> content_;

    int64_t contentLength_ = 0;
    std::string contentType_;
    std::string cacheControl_;
    std::string contentDisposition_;
    std::string contentEncoding_;
    std::string contentLanguage_;
    std::time_t expires_ = 0;
    ACLType acl_ = ACLType::NotSet;

    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;

    std::map<std::string, std::string> meta_;  // TOS SDK 会对 Key/Value 包含的中文汉字进行 URL 编码
    std::string websiteRedirectLocation_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速
    uint64_t preHashCrc64ecma_ = 0;  // 与初始化参数 EnableCRC 配套使用，代表上一次调用 AppendObjectOutput 返回的
                                     // HashCrc64ecma，第一次请求时为 0
    int64_t trafficLimit_ = 0;

    std::string notificationCustomParameters_;
};
}  // namespace VolcengineTos
