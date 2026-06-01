#pragma once

#include "model/async/bucket/base/ListerBase.h"
#include "model/async/base/BaseHttp.h"
#include "model/async/base/EncodingType.h"
#include "model/async/base/BucketBaseInput.h"

#include <string>

namespace VolcengineTos {

class ListObjectsAsyncInput final : public BaseHttp, public BucketBaseInput, public ListerBase, public EncodingType {
    friend class TosAsyncClient;

public:
    ListObjectsAsyncInput() = delete;
    ~ListObjectsAsyncInput() override = default;
    explicit ListObjectsAsyncInput(const std::string& bucket) : BucketBaseInput(bucket) {
    }

protected:
    void input2Headers() override {
        BucketBaseInput::input2Headers();
        BaseHttp::input2Headers();
    }
    void input2Queries() override {
        BucketBaseInput::input2Queries();
        ListerBase::input2Queries();
        EncodingType::input2Queries();
    }

    void json2Output(json& j) override {
    }
    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
