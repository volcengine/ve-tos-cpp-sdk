#pragma once

#include "model/async/base/AreaBase.h"
#include "model/async/base/BaseHttp.h"
#include "model/async/base/BucketBaseInput.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/Queries.h"
#include "model/async/base/StorageBase.h"

#include <string>

namespace VolcengineTos {
class CreateBucketAsyncInput final : public BaseHttp,
                                     public BucketBaseInput,
                                     public PermissionBase,
                                     public StorageBase,
                                     public AreaBase,
                                     public Queries {
    friend class TosAsyncClient;

public:
    CreateBucketAsyncInput() = delete;
    ~CreateBucketAsyncInput() override = default;
    explicit CreateBucketAsyncInput(const std::string& bucket) : BucketBaseInput(bucket) {
    }

protected:
    void input2Headers() override {
        BucketBaseInput::input2Headers();
        PermissionBase::input2Headers();
        StorageBase::input2Headers();
        AreaBase::input2Headers();
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
