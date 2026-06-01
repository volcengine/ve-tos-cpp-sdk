#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseInput.h"
#include "model/async/object/base/DataProcessBase.h"
#include "model/async/object/base/RangeBase.h"
#include "model/async/object/base/UserResponseBase.h"

#include <string>

namespace VolcengineTos {
class GetObjectAsyncInput : public BaseHttp,
                            public ObjectBaseInput,
                            public UserResponseBase,
                            public RangeBase,
                            public DataProcessBase,
                            public ChecksumBase {
    friend class TosAsyncClient;

public:
    GetObjectAsyncInput() = delete;
    ~GetObjectAsyncInput() override = default;
    GetObjectAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

protected:
    std::string valid() override {
        std::string error_string = ObjectBaseInput::valid();
        if (!error_string.empty()) {
            return error_string;
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
};
}  // namespace VolcengineTos
