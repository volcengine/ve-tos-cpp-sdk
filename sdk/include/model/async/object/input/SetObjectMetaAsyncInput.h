#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/UserMetaBase.h"

#include <utility>

namespace VolcengineTos {
class SetObjectMetaAsyncInput final : public BaseHttp, public ObjectBaseInput, public UserMetaBase {
    friend class TosAsyncClient;

public:
    SetObjectMetaAsyncInput() = delete;
    ~SetObjectMetaAsyncInput() override = default;
    SetObjectMetaAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    void input2Headers() override {
        BaseHttp::input2Headers();
        ObjectBaseInput::input2Headers();
        UserMetaBase::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        addQueryWithEmptyValue("metadata");
    }

    void headers2Output(HttpResponse& response) override {};
};
}  // namespace VolcengineTos
