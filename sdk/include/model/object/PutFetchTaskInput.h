#pragma once
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class PutFetchTaskInput : public GenericInput {
public:
    PutFetchTaskInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    PutFetchTaskInput(std::string bucket, std::string key, ACLType acl, StorageClassType storageClass,
                      std::map<std::string, std::string> meta, std::string url)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              acl_(acl),
              storageClass_(storageClass),
              meta_(std::move(meta)),
              url_(std::move(url)) {
    }
    PutFetchTaskInput() = default;
    virtual ~PutFetchTaskInput() = default;
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
    ACLType getAcl() const {
        return acl_;
    }
    void setAcl(ACLType acl) {
        acl_ = acl;
    }
    const std::string& getGrantFullControl() const {
        return grantFullControl_;
    }
    void setGrantFullControl(const std::string& grantFullControl) {
        grantFullControl_ = grantFullControl;
    }
    const std::string& getGrantRead() const {
        return grantRead_;
    }
    void setGrantRead(const std::string& grantRead) {
        grantRead_ = grantRead;
    }
    const std::string& getGrantReadAcp() const {
        return grantReadAcp_;
    }
    void setGrantReadAcp(const std::string& grantReadAcp) {
        grantReadAcp_ = grantReadAcp;
    }
    const std::string& getGrantWriteAcp() const {
        return grantWriteAcp_;
    }
    void setGrantWriteAcp(const std::string& grantWriteAcp) {
        grantWriteAcp_ = grantWriteAcp;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageClass) {
        storageClass_ = storageClass;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& ssecKey) {
        ssecKey_ = ssecKey;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMd5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMd5_ = ssecKeyMd5;
    }
    const std::map<std::string, std::string>& getMeta() const {
        return meta_;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }
    const std::string& getUrl() const {
        return url_;
    }
    void setUrl(const std::string& url) {
        url_ = url;
    }
    bool isIgnoreSameKey() const {
        return ignoreSameKey_;
    }
    void setIgnoreSameKey(bool ignoreSameKey) {
        ignoreSameKey_ = ignoreSameKey;
    }
    const std::string& getHexMd5() const {
        return hexMD5_;
    }
    void setHexMd5(const std::string& hexMd5) {
        hexMD5_ = hexMd5;
    }

    std::string toJsonString() const;

private:
    std::string bucket_;
    std::string key_;
    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;
    StorageClassType storageClass_ = StorageClassType::NotSet;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey_;
    std::string ssecKeyMd5_;
    std::map<std::string, std::string> meta_;  // TOS SDK 会对 Key/Value 包含的中文汉字进行 URL 编码

    std::string url_;
    bool ignoreSameKey_;
    std::string hexMD5_;
};
}  // namespace VolcengineTos
