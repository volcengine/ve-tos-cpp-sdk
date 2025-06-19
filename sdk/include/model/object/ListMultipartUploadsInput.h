#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class [[gnu::deprecated]] ListMultipartUploadsInput : public GenericInput {
public:
    ListMultipartUploadsInput() = default;
    ~ListMultipartUploadsInput() = default;
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

private:
    std::string prefix_;
    std::string delimiter_;
    std::string keyMarker_;
    std::string uploadIDMarker_;
    int maxUploads_ = 0;
};
}  // namespace VolcengineTos
