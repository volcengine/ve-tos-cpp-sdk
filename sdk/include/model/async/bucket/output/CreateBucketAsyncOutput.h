#pragma once
#include "model/async/base/BaseOutput.h"
#include "model/async/base/BucketBase.h"

namespace VolcengineTos {

class CreateBucketAsyncOutput final : public BaseOutput, public BucketBase {
    friend class TosAsyncClient;

public:
    CreateBucketAsyncOutput() = default;
    ~CreateBucketAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        BucketBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        BucketBase::json2Output(j);
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos