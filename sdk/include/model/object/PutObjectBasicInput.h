#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class PutObjectBasicInput {
public:
    PutObjectBasicInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    PutObjectBasicInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutObjectBasicInput() = default;
    ~PutObjectBasicInput() = default;

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
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentlength) {
        contentLength_ = contentlength;
    }
    const std::string& getContentMd5() const {
        return contentMD5_;
    }
    void setContentMd5(const std::string& contentmd5) {
        contentMD5_ = contentmd5;
    }
    const std::string& getContentSha256() const {
        return contentSHA256_;
    }
    void setContentSha256(const std::string& contentsha256) {
        contentSHA256_ = contentsha256;
    }
    const std::string& getCacheControl() const {
        return CacheControl;
    }
    void setCacheControl(const std::string& cachecontrol) {
        CacheControl = cachecontrol;
    }
    const std::string& getContentDisposition() const {
        return ContentDisposition;
    }
    void setContentDisposition(const std::string& contentdisposition) {
        ContentDisposition = contentdisposition;
    }
    const std::string& getContentEncoding() const {
        return ContentEncoding;
    }
    void setContentEncoding(const std::string& contentencoding) {
        ContentEncoding = contentencoding;
    }
    const std::string& getContentLanguage() const {
        return ContentLanguage;
    }
    void setContentLanguage(const std::string& contentlanguage) {
        ContentLanguage = contentlanguage;
    }
    const std::string& getContentType() const {
        return ContentType;
    }
    void setContentType(const std::string& contenttype) {
        ContentType = contenttype;
    }
    time_t getExpires() const {
        return Expires;
    }
    void setExpires(time_t expires) {
        Expires = expires;
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
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        ssecAlgorithm_ = ssecalgorithm;
    }
    const std::string& getSsecKey() const {
        return ssecKey;
    }
    void setSsecKey(const std::string& sseckey) {
        ssecKey = sseckey;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        ssecKeyMD5_ = sseckeymd5;
    }
    const std::string& getServerSideEncryption() const {
        return serverSideEncryption_;
    }
    void setServerSideEncryption(const std::string& serversideencryption) {
        serverSideEncryption_ = serversideencryption;
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
    const std::string& getCallBack() const {
        return callBack_;
    }
    void setCallBack(const std::string& callBack) {
        callBack_ = callBack;
    }
    const std::string& getCallBackVar() const {
        return callBackVar_;
    }
    void setCallBackVar(const std::string& callBackVar) {
        callBackVar_ = callBackVar;
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

    int64_t contentLength_ = 0;
    std::string contentMD5_;
    std::string contentSHA256_;
    std::string CacheControl;
    std::string ContentDisposition;  // TOS SDK 会对 Value 包含的中文汉字进行 URL 编码
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentType;
    std::time_t Expires = 0;

    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey;
    std::string ssecKeyMD5_;

    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256

    std::map<std::string, std::string> meta_;  // TOS SDK 会对 Key/Value 包含的中文汉字进行 URL 编码
    std::string websiteRedirectLocation_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速
    int64_t trafficLimit_ = 0;
    std::string callBack_;
    std::string callBackVar_;

    std::string notificationCustomParameters_;
};
}  // namespace VolcengineTos
