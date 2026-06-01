#pragma once
#include "model/async/base/BaseOutput.h"
#include "model/async/base/BucketBase.h"
#include "model/async/base/AreaBase.h"

namespace VolcengineTos {

class HeadBucketAsyncOutput final : public BaseOutput, public BucketBase, public AreaBase {
    friend class TosAsyncClient;

public:
    HeadBucketAsyncOutput() = default;
    ~HeadBucketAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        BucketBase::headers2Output(response);
        AreaBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        BucketBase::json2Output(j);
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos