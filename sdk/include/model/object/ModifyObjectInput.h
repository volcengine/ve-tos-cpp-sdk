#pragma once

#include <utility>

#include "model/RequestInfo.h"
#include "PutObjectBasicInput.h"
namespace VolcengineTos {
class ModifyObjectInput {
public:
    ModifyObjectInput() = default;
    ~ModifyObjectInput() = default;
    ModifyObjectInput(std::string bucket, std::string key): bucket_(std::move(bucket)), key_(std::move(key)) {}
    std::string getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    std::string getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    long getOffset() const {
        return offset_;
    }
    void setOffset(long offset) {
        offset_ = offset;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(const std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }
    long getContentLength() const {
        return contentLength_;
    }
    void setContentLength(long contentLength) {
        contentLength_ = contentLength;
    }
    DataTransferListener getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& dataTransferListener) {
        dataTransferListener_ = dataTransferListener;
    }
    std::shared_ptr<RateLimiter> getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& rateLimiter) {
        rateLimiter_ = rateLimiter;
    }
    long getTrafficLimit() const {
        return trafficLimit_;
    }
    void setTrafficLimit(long trafficLimit) {
        trafficLimit_ = trafficLimit;
    }

private:
    std::string bucket_;
    std::string key_;
    long offset_;
    std::shared_ptr<std::iostream> content_;
    long contentLength_ = -1;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速
    long trafficLimit_;
};
}  // namespace VolcengineTos
