#pragma once

#include "model/async/base/BucketNameBase.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/KeyBase.h"
#include "model/async/base/VersionIdBase.h"
#include "model/async/object/base/UploadIdBase.h"
#include "model/async/base/BaseOutput.h"

namespace VolcengineTos {

class CompleteMultipartUploadAsyncOutput final : public BaseOutput,
                                                 public BucketNameBase,
                                                 public KeyBase,
                                                 public VersionIdBase,
                                                 public ChecksumBase,
                                                 public UploadIdBase {
    friend class TosAsyncClient;

public:
    CompleteMultipartUploadAsyncOutput() = default;
    ~CompleteMultipartUploadAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        BucketNameBase::headers2Output(response);
        ChecksumBase::headers2Output(response);
        KeyBase::headers2Output(response);
        VersionIdBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        BucketNameBase::json2Output(j);
        ChecksumBase::json2Output(j);
        KeyBase::json2Output(j);
        UploadIdBase::json2Output(j);
    }

    void input2Queries() override {
    }
    std::string valid() override {
        return "";
    }

private:
    std::string callbackResult_;  // 当前机制不好处理
};

}  // namespace VolcengineTos