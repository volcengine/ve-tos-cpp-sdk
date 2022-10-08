#pragma once

#include <string>
#include <utility>
#include "UploadPartBasicInput.h"
namespace VolcengineTos {
class UploadPartFromFileInput {
public:
    UploadPartFromFileInput(std::string bucket, std::string key, std::string uploadId, int partNumber,
                            std::string filepath, int64_t offset, int64_t partsize)
            : uploadPartBasicInput_(std::move(bucket), std::move(key), std::move(uploadId), partNumber),
              filePath_(std::move(filepath)),
              offset_(offset),
              partSize_(partsize) {
    }

    const UploadPartBasicInput& getUploadPartBasicInput() const {
        return uploadPartBasicInput_;
    }
    void setUploadPartBasicInput(const UploadPartBasicInput& uploadpartbasicinput) {
        uploadPartBasicInput_ = uploadpartbasicinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
    int64_t getOffset() const {
        return offset_;
    }
    void setOffset(int64_t offset) {
        offset_ = offset;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
    }

private:
    UploadPartBasicInput uploadPartBasicInput_;
    std::string filePath_;
    int64_t offset_ = 0;
    int64_t partSize_ = 0;
};
}  // namespace VolcengineTos
