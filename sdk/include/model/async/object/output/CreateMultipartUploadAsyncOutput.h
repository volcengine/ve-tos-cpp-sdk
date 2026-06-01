#pragma once

#include "model/async/base/BucketNameBase.h"
#include "model/async/base/KeyBase.h"
#include "model/async/object/base/UploadIdBase.h"
#include "model/async/base/BaseOutput.h"
#include "model/async/base/EncodingType.h"

namespace VolcengineTos {

class CreateMultipartUploadAsyncOutput final : public BaseOutput,
                                               public BucketNameBase,
                                               public KeyBase,
                                               public UploadIdBase,
                                               public EncodingType {
    friend class TosAsyncClient;

public:
    CreateMultipartUploadAsyncOutput() = default;
    ~CreateMultipartUploadAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        BucketNameBase::headers2Output(response);
        KeyBase::headers2Output(response);
        UploadIdBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        BucketNameBase::json2Output(j);
        KeyBase::json2Output(j);
        UploadIdBase::json2Output(j);
        EncodingType::json2Output(j);
    }

    void input2Queries() override {
    }
    std::string valid() override {
        return "";
    }
};

}  // namespace VolcengineTos