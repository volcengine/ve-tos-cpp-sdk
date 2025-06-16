#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListObjectsV2Input : public GenericInput {
public:
    explicit ListObjectsV2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    ListObjectsV2Input() = default;
    ~ListObjectsV2Input() = default;

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
    const std::string& getMarker() const {
        return marker_;
    }
    void setMarker(const std::string& marker) {
        marker_ = marker;
    }
    int getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int maxKeys) {
        maxKeys_ = maxKeys;
    }
    bool isReverse() const {
        return reverse_;
    }
    void setReverse(bool reverse) {
        reverse_ = reverse;
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
    std::string marker_;
    int maxKeys_ = 0;
    [[gnu::deprecated]] bool reverse_ = false;
    std::string encodingType_;
};
}  // namespace VolcengineTos
