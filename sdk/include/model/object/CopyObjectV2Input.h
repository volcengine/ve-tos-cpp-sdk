#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class CopyObjectV2Input {
public:
    CopyObjectV2Input(std::string bucket, std::string key, std::string srcbucket, std::string srckey)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              srcBucket_(std::move(srcbucket)),
              srcKey_(std::move(srckey)) {
    }
    CopyObjectV2Input() = default;
    ~CopyObjectV2Input() = default;
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
    const std::string& getSrcBucket() const {
        return srcBucket_;
    }
    void setSrcBucket(const std::string& srcBucket) {
        srcBucket_ = srcBucket;
    }
    const std::string& getSrcKey() const {
        return srcKey_;
    }
    void setSrcKey(const std::string& srcKey) {
        srcKey_ = srcKey;
    }
    const StorageClassType& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const StorageClassType& storageClass) {
        storageClass_ = storageClass;
    }
    const std::string& getSrcVersionId() const {
        return srcVersionID_;
    }
    void setSrcVersionId(const std::string& srcversionid) {
        srcVersionID_ = srcversionid;
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
    const std::string& getCopySourceIfMatch() const {
        return copySourceIfMatch_;
    }
    void setCopySourceIfMatch(const std::string& copysourceifmatch) {
        copySourceIfMatch_ = copysourceifmatch;
    }
    time_t getCopySourceIfModifiedSince() const {
        return copySourceIfModifiedSince_;
    }
    void setCopySourceIfModifiedSince(time_t copysourceifmodifiedsince) {
        copySourceIfModifiedSince_ = copysourceifmodifiedsince;
    }
    const std::string& getCopySourceIfNoneMatch() const {
        return copySourceIfNoneMatch_;
    }
    void setCopySourceIfNoneMatch(const std::string& copysourceifnonematch) {
        copySourceIfNoneMatch_ = copysourceifnonematch;
    }
    time_t getCopySourceIfUnmodifiedSince() const {
        return copySourceIfUnmodifiedSince_;
    }
    void setCopySourceIfUnmodifiedSince(time_t copysourceifunmodifiedsince) {
        copySourceIfUnmodifiedSince_ = copysourceifunmodifiedsince;
    }
    const std::string& getCopySourceSsecAlgorithm() const {
        return copySourceSSECAlgorithm_;
    }
    void setCopySourceSsecAlgorithm(const std::string& copysourcessecalgorithm) {
        copySourceSSECAlgorithm_ = copysourcessecalgorithm;
    }
    const std::string& getCopySourceSsecKey() const {
        return copySourceSSECKey_;
    }
    void setCopySourceSsecKey(const std::string& copysourcesseckey) {
        copySourceSSECKey_ = copysourcesseckey;
    }
    const std::string& getCopySourceSsecKeyMd5() const {
        return copySourceSSECKeyMd5_;
    }
    void setCopySourceSsecKeyMd5(const std::string& copysourcesseckeymd5) {
        copySourceSSECKeyMd5_ = copysourcesseckeymd5;
    }
    const std::string& getServerSideEncryption() const {
        return serverSideEncryption_;
    }
    void setServerSideEncryption(const std::string& serversideencryption) {
        serverSideEncryption_ = serversideencryption;
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
    MetadataDirectiveType getMetadataDirective() const {
        return metadataDirective_;
    }
    void setMetadataDirective(MetadataDirectiveType metadatadirective) {
        metadataDirective_ = metadatadirective;
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

    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMD5_ = ssecKeyMd5;
    }
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& ssecKey) {
        ssecKey_ = ssecKey;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string srcBucket_;
    std::string srcKey_;
    std::string srcVersionID_;
    std::string cacheControl_;
    std::string contentDisposition_;  // TOS SDK 会对 Value 包含的中文汉字进行 URL 编码
    std::string contentEncoding_;
    std::string contentLanguage_;
    std::string contentType_;
    std::time_t expires_ = 0;

    std::string copySourceIfMatch_;
    std::time_t copySourceIfModifiedSince_ = 0;
    std::string copySourceIfNoneMatch_;
    std::time_t copySourceIfUnmodifiedSince_ = 0;

    std::string copySourceSSECAlgorithm_;  // 可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string copySourceSSECKey_;
    std::string copySourceSSECKeyMd5_;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKeyMD5_;
    std::string ssecKey_;

    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256

    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;

    MetadataDirectiveType metadataDirective_ = COPY;
    std::map<std::string, std::string> meta_;
    std::string websiteRedirectLocation_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
