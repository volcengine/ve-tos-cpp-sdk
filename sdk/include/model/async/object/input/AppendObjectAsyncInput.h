#pragma once

#include "model/async/base/BaseHttp.h"

#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/CallbackBase.h"
#include "model/async/object/base/UserMetaBase.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/OffsetBase.h"
#include "model/async/object/base/WebsiteBase.h"

namespace VolcengineTos {
class AppendObjectAsyncInput : public BaseHttp,
                               public ObjectBaseInput,
                               public PermissionBase,
                               public StorageBase,
                               public UserMetaBase,
                               public CallbackBase,
                               public WebsiteBase,
                               public OffsetBase,
                               public ChecksumBase {
    friend class TosAsyncClient;

public:
    AppendObjectAsyncInput() = default;
    ~AppendObjectAsyncInput() override = default;
    AppendObjectAsyncInput(const std::string& bucket, const std::string& key, const uint64_t offset,
                           TransferEncoding encoding)
            : ObjectBaseInput(bucket, key), OffsetBase(offset), BaseHttp(0, encoding) {
    }

protected:
    void input2Headers() override {
        BaseHttp::input2Headers();
        ObjectBaseInput::input2Headers();
        PermissionBase::input2Headers();
        StorageBase::input2Headers();
        UserMetaBase::input2Headers();
        CallbackBase::input2Headers();
        WebsiteBase::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        OffsetBase::input2Queries();
        addQueryWithEmptyValue("append");
    }

    void json2Output(json& j) override {
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
