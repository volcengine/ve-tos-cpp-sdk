#pragma once

#include "GetObjectAsyncOutput.h"

namespace VolcengineTos {

class HeadObjectAsyncOutput final : public GetObjectAsyncOutput {
    friend class TosAsyncClient;

public:
    HeadObjectAsyncOutput() = default;
    ~HeadObjectAsyncOutput() override = default;
};

}  // namespace VolcengineTos