#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/OffsetBase.h"

namespace VolcengineTos {
class ModifyObjectAsyncInput final : public ObjectBaseInput, public BaseHttp, public OffsetBase, public ChecksumBase {
    friend class TosAsyncClient;

public:
    ModifyObjectAsyncInput() = delete;
    ~ModifyObjectAsyncInput() override = default;
    ModifyObjectAsyncInput(const std::string& bucket, const std::string& key, const uint64_t offset,
                           const TransferEncoding encoding)
            : ObjectBaseInput(bucket, key), OffsetBase(offset), BaseHttp(0, encoding) {
    }

protected:
    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        BaseHttp::input2Headers();
        ChecksumBase::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        OffsetBase::input2Queries();
        addQueryWithEmptyValue("modify");
    }

    void json2Output(json& j) override {
    }

    void headers2Output(HttpResponse& response) override {
    }

private:
};
}  // namespace VolcengineTos
