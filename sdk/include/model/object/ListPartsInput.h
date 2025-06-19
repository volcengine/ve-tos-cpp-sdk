#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListPartsInput : public GenericInput {
public:
    ListPartsInput(std::string bucket, std::string key, std::string uploadid)
            : bucket_(std::move(bucket)), key_(std::move(key)), uploadID_(std::move(uploadid)) {
    }
    ListPartsInput() = default;
    ~ListPartsInput() = default;

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
    int getPartNumberMarker() const {
        return partNumberMarker_;
    }
    void setPartNumberMarker(int partnumbermarker) {
        partNumberMarker_ = partnumbermarker;
    }
    int getMaxParts() const {
        return maxParts_;
    }
    void setMaxParts(int maxparts) {
        maxParts_ = maxparts;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    int partNumberMarker_ = 0;
    int maxParts_ = 0;
};
}  // namespace VolcengineTos
