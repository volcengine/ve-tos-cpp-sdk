#pragma once

#include "model/async/base/ChecksumBase.h"
#include "model/async/object/base/PartBase.h"
#include "model/async/base/SseBase.h"
#include "model/async/base/BaseOutput.h"

namespace VolcengineTos {

class UploadPartAsyncOutput : public PartBase, public BaseOutput, public SseBase, public ChecksumBase {
    friend class TosAsyncClient;

public:
    UploadPartAsyncOutput() = default;
    ~UploadPartAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        PartBase::headers2Output(response);
        SseBase::headers2Output(response);
        ChecksumBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        PartBase::json2Output(j);
        ChecksumBase::json2Output(j);
    }

    void input2Queries() override {
    }
    void input2Headers() override {
    }
    std::string valid() override {
        return "";
    }
};

}  // namespace VolcengineTos