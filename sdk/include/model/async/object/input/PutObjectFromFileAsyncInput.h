#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ChecksumBase.h"

#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/CallbackBase.h"
#include "model/async/object/base/UserMetaBase.h"
#include "model/async/base/PermissionBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/WebsiteBase.h"

namespace VolcengineTos {
class PutObjectFromFileAsyncInput : public BaseHttp,
                                    public ObjectBaseInput,
                                    public PermissionBase,
                                    public StorageBase,
                                    public UserMetaBase,
                                    public CallbackBase,
                                    public WebsiteBase,
                                    public ChecksumBase {
    friend class TosAsyncClient;

public:
    PutObjectFromFileAsyncInput() = default;
    ~PutObjectFromFileAsyncInput() override = default;
    PutObjectFromFileAsyncInput(const std::string& bucket, const std::string& key, const std::string& filePath)
            : ObjectBaseInput(bucket, key), filePath_(filePath) {
    }

    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
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

    std::string valid() override {
        std::string error_string = ObjectBaseInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        if (filePath_.empty()) {
            return "empty file path";
        }

        return "";
    }

    void headers2Output(HttpResponse& response) override {
    }

    void json2Output(json& j) override {
    }

private:
    std::string filePath_;
};
}  // namespace VolcengineTos
