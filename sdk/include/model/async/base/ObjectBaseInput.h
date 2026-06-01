#pragma once

#include "BucketBaseInput.h"
#include "ConditionBase.h"
#include "KeyBase.h"
#include "LimiterBase.h"
#include "NotifyBase.h"
#include "SseBase.h"
#include "VersionIdBase.h"

#include <string>

namespace VolcengineTos {

class ObjectBaseInput : public BucketBaseInput,
                        public KeyBase,
                        public VersionIdBase,
                        public ConditionBase,
                        public SseBase,
                        public LimiterBase,
                        public NotifyBase {
    friend class TosAsyncClient;
public:
    ObjectBaseInput() = default;
    ~ObjectBaseInput() override = default;
    ObjectBaseInput(const std::string& bucket, const std::string& key) : BucketBaseInput(bucket), KeyBase(key) {
    }

    bool getIgnoreBody() const {
        return ignore_body_;
    }

    void setIgnoreBody(const bool ignoreContent) {
        ignore_body_ = ignoreContent;
    }

protected:
    void headers2Output(HttpResponse& response) override {
    }

    std::string valid() override {
        std::string error_string = SseBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = VolcengineTos::BucketBaseInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = KeyBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        return "";
    }

    void input2Headers() override {
        ConditionBase::input2Headers();
        LimiterBase::input2Headers();
        NotifyBase::input2Headers();
    }

    void input2Queries() override {
        VersionIdBase::input2Queries();
    }

    void json2Output(json& j) override {
    }

private:
    bool ignore_body_ = false;  // 不读取body，直接有头就返回
};

}  // namespace VolcengineTos
