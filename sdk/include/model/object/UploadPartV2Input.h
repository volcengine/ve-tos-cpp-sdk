#pragma once

#include <string>
#include <utility>
#include "UploadPartBasicInput.h"
namespace VolcengineTos {
class UploadPartV2Input {
public:
    UploadPartV2Input() = default;
    ~UploadPartV2Input() = default;
    UploadPartV2Input(std::string bucket, std::string key, std::string uploadId, int64_t contentLength, int partNumber,
                      std::shared_ptr<std::iostream> content)
            : uploadPartBasicInput_(std::move(bucket), std::move(key), std::move(uploadId), partNumber),
              content_(std::move(content)),
              contentLength_(contentLength) {
    }
    UploadPartV2Input(const UploadPartBasicInput& uploadpartbasicinput, std::shared_ptr<std::iostream> content,
                      int64_t contentlength)
            : uploadPartBasicInput_(uploadpartbasicinput), content_(std::move(content)), contentLength_(contentlength) {
    }
    const UploadPartBasicInput& getUploadPartBasicInput() const {
        return uploadPartBasicInput_;
    }
    void setUploadPartBasicInput(const UploadPartBasicInput& uploadpartbasicinput) {
        uploadPartBasicInput_ = uploadpartbasicinput;
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

    const std::string& getBucket() const {
        return uploadPartBasicInput_.getBucket();
    }
    void setBucket(const std::string& bucket) {
        uploadPartBasicInput_.setBucket(bucket);
    }
    const std::string& getKey() const {
        return uploadPartBasicInput_.getKey();
    }
    void setKey(const std::string& key) {
        uploadPartBasicInput_.setKey(key);
    }
    const std::string& getUploadId() const {
        return uploadPartBasicInput_.getUploadId();
    }
    void setUploadId(const std::string& uploadid) {
        uploadPartBasicInput_.setUploadId(uploadid);
    }
    int getPartNumber() const {
        return uploadPartBasicInput_.getPartNumber();
    }
    void setPartNumber(int partnumber) {
        uploadPartBasicInput_.setPartNumber(partnumber);
    }
    const std::string& getContentMd5() const {
        return uploadPartBasicInput_.getContentMd5();
    }
    void setContentMd5(const std::string& contentmd5) {
        uploadPartBasicInput_.setContentMd5(contentmd5);
    }
    const std::string& getSsecAlgorithm() const {
        return uploadPartBasicInput_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        uploadPartBasicInput_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKey() const {
        return uploadPartBasicInput_.getSsecKey();
    }
    void setSsecKey(const std::string& sseckey) {
        uploadPartBasicInput_.setSsecKey(sseckey);
    }
    const std::string& getSsecKeyMd5() const {
        return uploadPartBasicInput_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        uploadPartBasicInput_.setSsecKeyMd5(sseckeymd5);
    }
    const DataTransferListener& getDataTransferListener() const {
        return uploadPartBasicInput_.getDataTransferListener();
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        uploadPartBasicInput_.setDataTransferListener(datatransferlistener);
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return uploadPartBasicInput_.getRateLimiter();
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        uploadPartBasicInput_.setRateLimiter(ratelimiter);
    }

private:
    UploadPartBasicInput uploadPartBasicInput_;
    std::shared_ptr<std::iostream> content_;
    int64_t contentLength_ = 0;
};
}  // namespace VolcengineTos
