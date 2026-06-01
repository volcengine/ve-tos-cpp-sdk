#pragma once

#include "common/Common.h"
#include "model/async/base/BaseHttp.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/SymlinkBase.h"
#include "model/async/object/base/UserMetaBase.h"

#include <utility>

namespace VolcengineTos {
class PutSymlinkAsyncInput : public ObjectBaseInput,
                             public SymlinkBase,
                             public PermissionBase,
                             public StorageBase,
                             public UserMetaBase,
                             public BaseHttp {
    friend class TosAsyncClient;

public:
    PutSymlinkAsyncInput() = delete;
    ~PutSymlinkAsyncInput() override = default;
    PutSymlinkAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

    bool getForbidOverwrite() const {
        return forbidOverwrite_;
    }

    void setForbidOverwrite(bool forbidOverwrite) {
        forbidOverwrite_ = forbidOverwrite;
    }

    const std::string& getTagging() const {
        return tagging_;
    }

    void setTagging(const std::string& tagging) {
        tagging_ = tagging;
    }

protected:
    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        SymlinkBase::input2Headers();
        PermissionBase::input2Headers();
        StorageBase::input2Headers();
        UserMetaBase::input2Headers();
        BaseHttp::input2Headers();
        addHeader(HEADER_FORBID_OVERWRITE, forbidOverwrite_ ? "true" : "false");
        addHeader(HEADER_TAGGING, tagging_);
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        addQueryWithEmptyValue("symlink");
    }

    void headers2Output(HttpResponse& response) override {
    }

private:
    bool forbidOverwrite_{false};
    std::string tagging_;
};
}  // namespace VolcengineTos
