#pragma once

#include <utility>

#include "model/RequestInfo.h"
#include "PutObjectBasicInput.h"
namespace VolcengineTos {
class PutObjectV2Input {
public:
    PutObjectV2Input() = default;
    ~PutObjectV2Input() = default;
    PutObjectV2Input(std::string bucket, std::string key, std::shared_ptr<std::iostream> content)
            : putObjectBasicInput_(std::move(bucket), std::move(key)), content_(std::move(content)) {
    }
    PutObjectV2Input(const PutObjectBasicInput& putobjectbasicinput, std::shared_ptr<std::iostream>  content)
            : putObjectBasicInput_(putobjectbasicinput), content_(std::move(content)) {
    }
    PutObjectV2Input(std::string bucket, std::string key) : putObjectBasicInput_(std::move(bucket), std::move(key)) {
    }
    const PutObjectBasicInput& getPutObjectBasicInput() const {
        return putObjectBasicInput_;
    }
    void setPutObjectBasicInput(const PutObjectBasicInput& putobjectbasicinput) {
        putObjectBasicInput_ = putobjectbasicinput;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream> content) {
        content_ = std::move(content);
    }

    const std::string& getBucket() const {
        return putObjectBasicInput_.getBucket();
    }
    void setBucket(const std::string& bucket) {
        putObjectBasicInput_.setBucket(bucket);
    }
    const std::string& getKey() const {
        return putObjectBasicInput_.getKey();
    }
    void setKey(const std::string& key) {
        putObjectBasicInput_.setKey(key);
    }
    int64_t getContentLength() const {
        return putObjectBasicInput_.getContentLength();
    }
    void setContentLength(int64_t contentlength) {
        putObjectBasicInput_.setContentLength(contentlength);
    }
    const std::string& getContentMd5() const {
        return putObjectBasicInput_.getContentMd5();
    }
    void setContentMd5(const std::string& contentmd5) {
        putObjectBasicInput_.setContentMd5(contentmd5);
    }
    const std::string& getContentSha256() const {
        return putObjectBasicInput_.getContentSha256();
    }
    void setContentSha256(const std::string& contentsha256) {
        putObjectBasicInput_.setContentSha256(contentsha256);
    }
    const std::string& getCacheControl() const {
        return putObjectBasicInput_.getCacheControl();
    }
    void setCacheControl(const std::string& cachecontrol) {
        putObjectBasicInput_.setCacheControl(cachecontrol);
    }
    const std::string& getContentDisposition() const {
        return putObjectBasicInput_.getContentDisposition();
    }
    void setContentDisposition(const std::string& contentdisposition) {
        putObjectBasicInput_.setContentDisposition(contentdisposition);
    }
    const std::string& getContentEncoding() const {
        return putObjectBasicInput_.getContentEncoding();
    }
    void setContentEncoding(const std::string& contentencoding) {
        putObjectBasicInput_.setContentEncoding(contentencoding);
    }
    const std::string& getContentLanguage() const {
        return putObjectBasicInput_.getContentLanguage();
    }
    void setContentLanguage(const std::string& contentlanguage) {
        putObjectBasicInput_.setContentLanguage(contentlanguage);
    }
    const std::string& getContentType() const {
        return putObjectBasicInput_.getContentType();
    }
    void setContentType(const std::string& contenttype) {
        putObjectBasicInput_.setContentType(contenttype);
    }
    time_t getExpires() const {
        return putObjectBasicInput_.getExpires();
    }
    void setExpires(time_t expires) {
        putObjectBasicInput_.setExpires(expires);
    }
    ACLType getAcl() const {
        return putObjectBasicInput_.getAcl();
    }
    void setAcl(ACLType acl) {
        putObjectBasicInput_.setAcl(acl);
    }
    const std::string& getGrantFullControl() const {
        return putObjectBasicInput_.getGrantFullControl();
    }
    void setGrantFullControl(const std::string& grantfullcontrol) {
        putObjectBasicInput_.setGrantFullControl(grantfullcontrol);
    }
    const std::string& getGrantRead() const {
        return putObjectBasicInput_.getGrantRead();
    }
    void setGrantRead(const std::string& grantread) {
        putObjectBasicInput_.setGrantRead(grantread);
    }
    const std::string& getGrantReadAcp() const {
        return putObjectBasicInput_.getGrantReadAcp();
    }
    void setGrantReadAcp(const std::string& grantreadacp) {
        putObjectBasicInput_.setGrantReadAcp(grantreadacp);
    }
    const std::string& getGrantWriteAcp() const {
        return putObjectBasicInput_.getGrantWriteAcp();
    }
    void setGrantWriteAcp(const std::string& grantwriteacp) {
        putObjectBasicInput_.setGrantWriteAcp(grantwriteacp);
    }
    const std::string& getSsecAlgorithm() const {
        return putObjectBasicInput_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        putObjectBasicInput_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKey() const {
        return putObjectBasicInput_.getSsecKey();
    }
    void setSsecKey(const std::string& sseckey) {
        putObjectBasicInput_.setSsecKey(sseckey);
    }
    const std::string& getSsecKeyMd5() const {
        return putObjectBasicInput_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        putObjectBasicInput_.setSsecKeyMd5(sseckeymd5);
    }
    const std::string& getServerSideEncryption() const {
        return putObjectBasicInput_.getServerSideEncryption();
    }
    void setServerSideEncryption(const std::string& serversideencryption) {
        putObjectBasicInput_.setServerSideEncryption(serversideencryption);
    }
    const std::map<std::string, std::string>& getMeta() const {
        return putObjectBasicInput_.getMeta();
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        putObjectBasicInput_.setMeta(meta);
    }
    const std::string& getWebsiteRedirectLocation() const {
        return putObjectBasicInput_.getWebsiteRedirectLocation();
    }
    void setWebsiteRedirectLocation(const std::string& websiteredirectlocation) {
        putObjectBasicInput_.setWebsiteRedirectLocation(websiteredirectlocation);
    }
    StorageClassType getStorageClass() const {
        return putObjectBasicInput_.getStorageClass();
    }
    void setStorageClass(StorageClassType storageclass) {
        putObjectBasicInput_.setStorageClass(storageclass);
    }
    const DataTransferListener& getDataTransferListener() const {
        return putObjectBasicInput_.getDataTransferListener();
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        putObjectBasicInput_.setDataTransferListener(datatransferlistener);
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return putObjectBasicInput_.getRateLimiter();
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        putObjectBasicInput_.setRateLimiter(ratelimiter);
    }

private:
    PutObjectBasicInput putObjectBasicInput_;
    std::shared_ptr<std::iostream> content_;  // io.Reader
};
}  // namespace VolcengineTos
