#pragma once

#include "PutObjectAsyncInput.h"
#include "model/async/object/base/PartBase.h"

namespace VolcengineTos {
class UploadPartAsyncInput : public PutObjectAsyncInput, public PartBase {
    friend class TosAsyncClient;

public:
    UploadPartAsyncInput() = default;
    ~UploadPartAsyncInput() override = default;
    UploadPartAsyncInput(const std::string& bucket, const std::string& key, const int& partNumber,
                         const std::string& uploadId, const TransferEncoding transferEncoding)
            : PutObjectAsyncInput(bucket, key, transferEncoding), PartBase(partNumber, uploadId) {
    }

protected:
    void input2Queries() override {
        PutObjectAsyncInput::input2Queries();
        PartBase::input2Queries();
    }

    std::string valid() override {
        std::string error_string = PutObjectAsyncInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = PartBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        return "";
    }

    void json2Output(json& j) override {
    }
};
}  // namespace VolcengineTos
