#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/bucket/base/VersionsLister.h"
#include "model/async/base/BucketBaseInput.h"
#include "model/async/base/EncodingType.h"

#include <string>

namespace VolcengineTos {

class ListObjectVersionsAsyncInput final : public BaseHttp,
                                           public BucketBaseInput,
                                           public VersionsListerBase,
                                           public EncodingType {
    friend class TosAsyncClient;

public:
    ListObjectVersionsAsyncInput() = delete;
    ~ListObjectVersionsAsyncInput() override = default;
    explicit ListObjectVersionsAsyncInput(const std::string& bucket) : BucketBaseInput(bucket) {
    }

protected:
    void input2Headers() override {
        BucketBaseInput::input2Headers();
        BaseHttp::input2Headers();
    }

    void input2Queries() override {
        BucketBaseInput::input2Queries();
        VersionsListerBase::input2Queries();
        EncodingType::input2Queries();
    }

    void json2Output(json& j) override {
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
