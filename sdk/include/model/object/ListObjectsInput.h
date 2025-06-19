#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class [[gnu::deprecated]] ListObjectsInput : public GenericInput {
public:
    ListObjectsInput() = default;
    ~ListObjectsInput() = default;
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
    bool reverse_ = false;
    std::string encodingType_;
};
}  // namespace VolcengineTos
