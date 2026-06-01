#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ObjectBaseInput.h"

namespace VolcengineTos {
class GetFileStatusAsyncInput final : public ObjectBaseInput, public BaseHttp {
    friend class TosAsyncClient;

public:
    GetFileStatusAsyncInput() = delete;
    ~GetFileStatusAsyncInput() override = default;
    GetFileStatusAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        BaseHttp::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        addQueryWithEmptyValue("stat");
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
