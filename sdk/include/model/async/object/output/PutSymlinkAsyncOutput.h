#pragma once

#include "model/async/base/BaseOutput.h"
#include "model/async/base/VersionIdBase.h"

namespace VolcengineTos {

class PutSymlinkAsyncOutput : public BaseOutput, public VersionIdBase {
    friend class TosAsyncClient;

public:
    PutSymlinkAsyncOutput() = default;
    ~PutSymlinkAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        VersionIdBase::headers2Output(response);
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos