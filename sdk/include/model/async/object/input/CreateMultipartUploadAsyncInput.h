#pragma once

#include "PutObjectAsyncInput.h"

#include <utility>
#include "model/async/base/EncodingType.h"

namespace VolcengineTos {
class CreateMultipartUploadAsyncInput final : public PutObjectAsyncInput, public EncodingType {
    friend class TosAsyncClient;

public:
    CreateMultipartUploadAsyncInput() = delete;
    ~CreateMultipartUploadAsyncInput() override = default;
    CreateMultipartUploadAsyncInput(const std::string& bucket, const std::string& key)
            : PutObjectAsyncInput(bucket, key, TransferEncoding::ContentLength) {
    }

protected:
    void input2Queries() override {
        PutObjectAsyncInput::input2Queries();
        EncodingType::input2Queries();
        addQueryWithEmptyValue("uploads");
    }

    void json2Output(json& j) override {
    }
};
}  // namespace VolcengineTos
