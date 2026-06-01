#pragma once

#include "model/async/base/BaseHttp.h"

#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/CallbackBase.h"
#include "model/async/object/base/UserMetaBase.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/CopySourceBase.h"
#include "model/async/object/base/WebsiteBase.h"

namespace VolcengineTos {
class CopyObjectAsyncInput : public BaseHttp,
                             public ObjectBaseInput,
                             public PermissionBase,
                             public StorageBase,
                             public UserMetaBase,
                             public CallbackBase,
                             public WebsiteBase,
                             public CopySourceBase,
                             public ChecksumBase {
    friend class TosAsyncClient;

public:
    CopyObjectAsyncInput() = default;
    ~CopyObjectAsyncInput() override = default;
    CopyObjectAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    std::string valid() override {
        std::string error_string = ObjectBaseInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }
        return CopySourceBase::valid();
    }

    void input2Headers() override {
        BaseHttp::input2Headers();
        ObjectBaseInput::input2Headers();
        PermissionBase::input2Headers();
        StorageBase::input2Headers();
        UserMetaBase::input2Headers();
        CallbackBase::input2Headers();
        WebsiteBase::input2Headers();
        CopySourceBase::input2Headers();
        ChecksumBase::input2Headers();
    }

    void json2Output(json& j) override {
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
