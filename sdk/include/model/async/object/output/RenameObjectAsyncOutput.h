#pragma once

#include "model/async/base/BaseOutput.h"

namespace VolcengineTos {

class RenameObjectAsyncOutput : public BaseOutput {
    friend class TosAsyncClient;

public:
    RenameObjectAsyncOutput() = default;
    ~RenameObjectAsyncOutput() override = default;

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
    }

    void input2Headers() override {
    }
};

}  // namespace VolcengineTos