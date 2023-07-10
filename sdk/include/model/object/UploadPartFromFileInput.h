#pragma once

#include <string>
#include <utility>
#include "UploadPartBasicInput.h"
namespace VolcengineTos {
class UploadPartFromFileInput {
public:
    UploadPartFromFileInput(std::string bucket, std::string key, std::string uploadId, int partNumber,
                            std::string filepath, int64_t offset, int64_t partsize)
            : uploadPartBasicInput_(std::move(bucket), std::move(key), std::move(uploadId), partNumber),
              filePath_(std::move(filepath)),
              offset_(offset),
              partSize_(partsize) {
    }

    const UploadPartBasicInput& getUploadPartBasicInput() const {
        return uploadPartBasicInput_;
    }
    void setUploadPartBasicInput(const UploadPartBasicInput& uploadpartbasicinput) {
        uploadPartBasicInput_ = uploadpartbasicinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
    int64_t getOffset() const {
        return offset_;
    }
    void setOffset(int64_t offset) {
        offset_ = offset;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
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
    int64_t getTrafficLimit() const {
        return uploadPartBasicInput_.getTrafficLimit();
    }
    void setTrafficLimit(int64_t trafficLimit) {
        uploadPartBasicInput_.setTrafficLimit(trafficLimit);
    }

private:
    UploadPartBasicInput uploadPartBasicInput_;
    std::string filePath_;
    int64_t offset_ = 0;
    int64_t partSize_ = 0;
};
}  // namespace VolcengineTos
