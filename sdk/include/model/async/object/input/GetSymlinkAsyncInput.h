#pragma once

#include "model/async/base/BaseHttp.h"

#include <utility>

#include "model/async/base/ObjectBaseInput.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/SymlinkBase.h"
#include "model/async/object/base/UserMetaBase.h"

namespace VolcengineTos {
class GetSymlinkAsyncInput : public ObjectBaseInput,
                             public SymlinkBase,
                             public PermissionBase,
                             public StorageBase,
                             public UserMetaBase,
                             public BaseHttp {
    friend class TosAsyncClient;

public:
    GetSymlinkAsyncInput() = delete;
    ~GetSymlinkAsyncInput() override = default;
    GetSymlinkAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        SymlinkBase::input2Headers();
        PermissionBase::input2Headers();
        StorageBase::input2Headers();
        UserMetaBase::input2Headers();
        BaseHttp::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        addQueryWithEmptyValue("symlink");
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
