#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListMultipartUploadsV2Input : public GenericInput {
public:
    explicit ListMultipartUploadsV2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    ListMultipartUploadsV2Input() = default;
    ~ListMultipartUploadsV2Input() = default;

    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }
    const std::string& getKeyMarker() const {
        return keyMarker_;
    }
    void setKeyMarker(const std::string& keyMarker) {
        keyMarker_ = keyMarker;
    }
    const std::string& getUploadIdMarker() const {
        return uploadIDMarker_;
    }
    void setUploadIdMarker(const std::string& uploadIdMarker) {
        uploadIDMarker_ = uploadIdMarker;
    }
    int getMaxUploads() const {
        return maxUploads_;
    }
    void setMaxUploads(int maxUploads) {
        maxUploads_ = maxUploads;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingtype) {
        encodingType_ = encodingtype;
    }

private:
    std::string bucket_;
    std::string prefix_;
    std::string delimiter_;
    std::string keyMarker_;
    std::string uploadIDMarker_;
    int maxUploads_ = 0;
    std::string encodingType_;
};
}  // namespace VolcengineTos
