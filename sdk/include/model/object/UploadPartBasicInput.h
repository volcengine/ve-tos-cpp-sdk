#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class UploadPartBasicInput {
public:
    UploadPartBasicInput(std::string bucket, std::string key, std::string uploadId, int partNumber)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              uploadID_(std::move(uploadId)),
              partNumber_(partNumber) {
    }
    UploadPartBasicInput() = default;
    ~UploadPartBasicInput() = default;
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
        return uploadID_;
    }
    void setUploadId(const std::string& uploadid) {
        uploadID_ = uploadid;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partnumber) {
        partNumber_ = partnumber;
    }
    const std::string& getContentMd5() const {
        return contentMD5;
    }
    void setContentMd5(const std::string& contentmd5) {
        contentMD5 = contentmd5;
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

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    int partNumber_ = 0;

    std::string contentMD5;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey;
    std::string ssecKeyMD5_;

    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256

    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速，详见 4.4.5 章节
};
}  // namespace VolcengineTos
