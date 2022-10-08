#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class CreateMultipartUploadInput {
public:
    CreateMultipartUploadInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    CreateMultipartUploadInput() = default;
    ~CreateMultipartUploadInput() = default;
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
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingtype) {
        encodingType_ = encodingtype;
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
    const std::string& getContentType() const {
        return contentType_;
    }
    void setContentType(const std::string& contenttype) {
        contentType_ = contenttype;
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
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& sseckey) {
        ssecKey_ = sseckey;
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

private:
    std::string bucket_;
    std::string key_;

    std::string encodingType_;
    std::string cacheControl_;
    std::string contentDisposition_;  // TOS SDK 会对 Value 包含的中文汉字进行 URL 编码
    std::string contentEncoding_;
    std::string contentLanguage_;
    std::string contentType_;
    std::time_t expires_ = 0;
    ACLType acl_ = ACLType::NotSet;

    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKeyMD5_;
    std::string ssecKey_;

    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256

    std::map<std::string, std::string> meta_;  // TOS SDK 会对 Key/Value 包含的中文汉字进行 URL 编码
    std::string websiteRedirectLocation_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
