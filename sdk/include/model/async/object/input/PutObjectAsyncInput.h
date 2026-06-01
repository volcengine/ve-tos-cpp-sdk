#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/CallbackBase.h"
#include "model/async/object/base/UserMetaBase.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/WebsiteBase.h"

#include <utility>

namespace VolcengineTos {
class PutObjectAsyncInput : public BaseHttp,
                            public ObjectBaseInput,
                            public PermissionBase,
                            public StorageBase,
                            public UserMetaBase,
                            public CallbackBase,
                            public WebsiteBase,
                            public ChecksumBase {
    friend class TosAsyncClient;

public:
    PutObjectAsyncInput() = default;
    ~PutObjectAsyncInput() override = default;
    PutObjectAsyncInput(const std::string& bucket, const std::string& key, const TransferEncoding transferEncoding)
            : BaseHttp(0, transferEncoding), ObjectBaseInput(bucket, key) {
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
        ChecksumBase::input2Headers();
    }

    void headers2Output(HttpResponse& response) override {
    }

    void json2Output(json& j) override {
    }
};
}  // namespace VolcengineTos
