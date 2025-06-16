#pragma once

#include <string>
#include <utility>
#include "model/GenericInput.h"

namespace VolcengineTos {
class [[gnu::deprecated]] UploadPartInput : public GenericInput {
public:
    UploadPartInput() = default;
    ~UploadPartInput() = default;
    UploadPartInput(std::string key, std::string uploadId, int64_t partSize, int partNumber,
                    std::shared_ptr<std::iostream> content)
            : key_(std::move(key)),
              uploadID_(std::move(uploadId)),
              partSize_(partSize),
              partNumber_(partNumber),
              content_(std::move(content)) {
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
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partSize) {
        partSize_ = partSize;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partNumber) {
        partNumber_ = partNumber;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }

private:
    std::string key_;
    std::string uploadID_;
    int64_t partSize_{};
    int partNumber_{};
    std::shared_ptr<std::iostream> content_;
};
}  // namespace VolcengineTos
