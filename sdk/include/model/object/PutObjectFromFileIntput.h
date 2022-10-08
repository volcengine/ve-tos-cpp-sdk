#pragma once

#include "PutObjectBasicInput.h"

namespace VolcengineTos {
class PutObjectFromFileInput {
public:
    PutObjectFromFileInput(std::string bucket, std::string key, std::string filepath)
            : putObjectBasicInput_(std::move(bucket), std::move(key)), filePath_(std::move(filepath)) {
    }
    PutObjectFromFileInput() = default;
    ~PutObjectFromFileInput() = default;

    const PutObjectBasicInput& getPutObjectBasicInput() const {
        return putObjectBasicInput_;
    }
    void setPutObjectBasicInput(const PutObjectBasicInput& putobjectbasicinput) {
        putObjectBasicInput_ = putobjectbasicinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }

private:
    PutObjectBasicInput putObjectBasicInput_;
    std::string filePath_;
};
}  // namespace VolcengineTos
