#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/RenameBase.h"

namespace VolcengineTos {
class RenameObjectAsyncInput : public ObjectBaseInput, public RenameBase, public BaseHttp {
    friend class TosAsyncClient;

public:
    RenameObjectAsyncInput() = delete;
    ~RenameObjectAsyncInput() override = default;
    RenameObjectAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        RenameBase::input2Headers();
        BaseHttp::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        RenameBase::input2Queries();
        addQueryWithEmptyValue("rename");
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
