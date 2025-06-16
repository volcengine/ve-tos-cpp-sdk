#pragma once

#include <string>

#include "model/GenericInput.h"

namespace VolcengineTos {
class AbortMultipartUploadInput : public GenericInput {
public:
    AbortMultipartUploadInput() = default;
    ~AbortMultipartUploadInput() = default;
    AbortMultipartUploadInput(std::string key, std::string uploadId)
            : key_(std::move(key)), uploadID_(std::move(uploadId)) {
    }
    AbortMultipartUploadInput(std::string bucket, std::string key, std::string uploadId)
            : bucket_(std::move(bucket)), key_(std::move(key)), uploadID_(std::move(uploadId)) {
    }
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
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
};
}  // namespace VolcengineTos
