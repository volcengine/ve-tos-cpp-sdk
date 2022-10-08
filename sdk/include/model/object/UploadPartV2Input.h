#pragma once

#include <string>
#include <utility>
#include "UploadPartBasicInput.h"
namespace VolcengineTos {
class UploadPartV2Input {
public:
    UploadPartV2Input() = default;
    ~UploadPartV2Input() = default;
    UploadPartV2Input(std::string bucket, std::string key, std::string uploadId, int64_t contentLength, int partNumber,
                      std::shared_ptr<std::iostream> content)
            : uploadPartBasicInput_(std::move(bucket), std::move(key), std::move(uploadId), partNumber),
              contentLength_(contentLength),
              content_(std::move(content)) {
    }
    UploadPartV2Input(const UploadPartBasicInput& uploadpartbasicinput, std::shared_ptr<std::iostream>  content,
                      int64_t contentlength)
            : uploadPartBasicInput_(uploadpartbasicinput), content_(std::move(content)), contentLength_(contentlength) {
    }
    const UploadPartBasicInput& getUploadPartBasicInput() const {
        return uploadPartBasicInput_;
    }
    void setUploadPartBasicInput(const UploadPartBasicInput& uploadpartbasicinput) {
        uploadPartBasicInput_ = uploadpartbasicinput;
    }
    const std::shared_ptr<std::iostream>& getContent() const {
        return content_;
    }
    void setContent(const std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentlength) {
        contentLength_ = contentlength;
    }

private:
    UploadPartBasicInput uploadPartBasicInput_;
    std::shared_ptr<std::iostream> content_;
    int64_t contentLength_ = 0;
};
}  // namespace VolcengineTos
