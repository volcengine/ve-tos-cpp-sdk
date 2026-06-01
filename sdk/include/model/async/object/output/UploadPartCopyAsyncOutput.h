#pragma once

#include "UploadPartAsyncOutput.h"
#include "model/async/object/base/CopySourceBase.h"

namespace VolcengineTos {

class UploadPartCopyAsyncOutput final : public UploadPartAsyncOutput, public CopySourceBase {
    friend class TosAsyncClient;

public:
    UploadPartCopyAsyncOutput() = default;
    ~UploadPartCopyAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        UploadPartAsyncOutput::headers2Output(response);
        CopySourceBase::headers2Output(response);
    }

    void input2Headers() override {
    }

    std::string valid() override {
        return "";
    }
};

}  // namespace VolcengineTos