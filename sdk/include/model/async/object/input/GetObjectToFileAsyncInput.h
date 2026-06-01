#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ChecksumBase.h"

#include <string>
#include <utility>
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/DataProcessBase.h"
#include "model/async/object/base/RangeBase.h"
#include "model/async/object/base/UserResponseBase.h"

namespace VolcengineTos {
class GetObjectToFileAsyncInput : public BaseHttp,
                                  public ObjectBaseInput,
                                  public UserResponseBase,
                                  public RangeBase,
                                  public DataProcessBase,
                                  public ChecksumBase {
    friend class TosAsyncClient;

public:
    GetObjectToFileAsyncInput() = delete;
    ~GetObjectToFileAsyncInput() override = default;
    GetObjectToFileAsyncInput(const std::string& bucket, const std::string& key, std::string filePath)
            : ObjectBaseInput(bucket, key), filePath_(std::move(filePath)) {
    }

    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }

protected:
    std::string valid() override {
        std::string error_string = ObjectBaseInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        if (filePath_.empty()) {
            return "empty file path";
        }

        return RangeBase::valid();
    }

    void input2Headers() override {
        ObjectBaseInput::input2Headers();
        BaseHttp::input2Headers();
        RangeBase::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();
        UserResponseBase::input2Queries();
        DataProcessBase::input2Queries();
    }

    void json2Output(json& j) override {
    }

    void headers2Output(HttpResponse& response) override {
    }

private:
    std::string filePath_;
};
}  // namespace VolcengineTos
