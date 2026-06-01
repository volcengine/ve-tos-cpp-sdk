#pragma once

#include "model/async/base/BaseOutput.h"
#include "model/async/base/VersionIdBase.h"
#include "model/async/object/base/SymlinkBase.h"

namespace VolcengineTos {

class GetSymlinkAsyncOutput : public BaseOutput, public VersionIdBase, public SymlinkBase {
    friend class TosAsyncClient;

public:
    GetSymlinkAsyncOutput() = default;
    ~GetSymlinkAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        VersionIdBase::headers2Output(response);
        SymlinkBase::headers2Output(response);
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos