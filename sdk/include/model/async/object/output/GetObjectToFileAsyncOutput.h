#pragma once

#include "model/async/base/ChecksumBase.h"
#include "model/async/base/ObjectBaseOutput.h"
#include "model/async/base/SseBase.h"
#include "model/async/base/StorageBase.h"
#include "model/async/object/base/UserMetaBase.h"
#include "model/async/object/base/WebsiteBase.h"

namespace VolcengineTos {

class GetObjectToFileAsyncOutput : public ObjectBaseOutput,
                             public SseBase,
                             public WebsiteBase,
                             public ChecksumBase,
                             public UserMetaBase,
                             public StorageBase {
    friend class TosAsyncClient;

public:
    GetObjectToFileAsyncOutput() = default;
    ~GetObjectToFileAsyncOutput() override = default;

protected:
    void input2Headers() override {
    }

    void input2Queries() override {
    }

    std::string input2json() override {
        return "";
    }

    std::string valid() override {
        return "";
    }

    void json2Output(json& j) override {
        ObjectBaseOutput::json2Output(j);
    }

    void headers2Output(HttpResponse& response) override {
        ObjectBaseOutput::headers2Output(response);
        SseBase::headers2Output(response);
        WebsiteBase::headers2Output(response);
        ChecksumBase::headers2Output(response);
        StorageBase::headers2Output(response);
        UserMetaBase::headers2Output(response);
    }
};

}  // namespace VolcengineTos