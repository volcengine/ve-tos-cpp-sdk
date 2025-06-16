#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListObjectVersionsV2Input : public GenericInput {
public:
    ListObjectVersionsV2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    ListObjectVersionsV2Input() = default;
    ~ListObjectVersionsV2Input() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
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
    const std::string& getVersionIdMarker() const {
        return versionIDMarker_;
    }
    void setVersionIdMarker(const std::string& versionIdMarker) {
        versionIDMarker_ = versionIdMarker;
    }
    int getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int maxKeys) {
        maxKeys_ = maxKeys;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }

private:
    std::string bucket_;
    std::string prefix_;
    std::string delimiter_;
    std::string keyMarker_;
    std::string versionIDMarker_;
    int maxKeys_ = 0;
    std::string encodingType_;
};
}  // namespace VolcengineTos
