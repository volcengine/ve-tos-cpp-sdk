#pragma once

#include "model/async/base/BaseOutput.h"

namespace VolcengineTos {

class SetObjectMetaAsyncOutput final :  public BaseOutput {
    friend class TosAsyncClient;


public:
    SetObjectMetaAsyncOutput() = default;
    ~SetObjectMetaAsyncOutput() override = default;
};

}  // namespace VolcengineTos