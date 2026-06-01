#pragma once

#include "model/async/base/BaseOutput.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/SseBase.h"

namespace VolcengineTos {

class CopyObjectAsyncOutput : public BaseOutput, public ChecksumBase, public SseBase {
    friend class TosAsyncClient;

public:
    CopyObjectAsyncOutput() = default;
    ~CopyObjectAsyncOutput() override = default;

protected:
    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        ChecksumBase::json2Output(j);
    }

    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        ChecksumBase::headers2Output(response);
        SseBase::headers2Output(response);
    }

    void input2Queries() override {
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos