#pragma once

#include <string>
namespace VolcengineTos {
class ListUploadedPartsInput {
public:
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
    int getMaxParts() const {
        return maxParts_;
    }
    void setMaxParts(int maxParts) {
        maxParts_ = maxParts;
    }
    int getPartNumberMarker() const {
        return partNumberMarker_;
    }
    void setPartNumberMarker(int partNumberMarker) {
        partNumberMarker_ = partNumberMarker;
    }

private:
    std::string key_;
    std::string uploadID_;
    int maxParts_;
    int partNumberMarker_;
};
}  // namespace VolcengineTos
