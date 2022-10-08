#pragma once

#include <utility>

#include "model/RequestInfo.h"
#include "PutObjectBasicInput.h"
namespace VolcengineTos {
class PutObjectV2Input {
public:
    PutObjectV2Input() = default;
    ~PutObjectV2Input() = default;
    PutObjectV2Input(std::string bucket, std::string key, std::shared_ptr<std::iostream> content)
            : putObjectBasicInput_(std::move(bucket), std::move(key)), content_(std::move(content)) {
    }
    PutObjectV2Input(const PutObjectBasicInput& putobjectbasicinput, std::shared_ptr<std::iostream>  content)
            : putObjectBasicInput_(putobjectbasicinput), content_(std::move(content)) {
    }
    PutObjectV2Input(std::string bucket, std::string key) : putObjectBasicInput_(std::move(bucket), std::move(key)) {
    }
    const PutObjectBasicInput& getPutObjectBasicInput() const {
        return putObjectBasicInput_;
    }
    void setPutObjectBasicInput(const PutObjectBasicInput& putobjectbasicinput) {
        putObjectBasicInput_ = putobjectbasicinput;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream> content) {
        content_ = std::move(content);
    }

private:
    PutObjectBasicInput putObjectBasicInput_;
    std::shared_ptr<std::iostream> content_;  // io.Reader
};
}  // namespace VolcengineTos
