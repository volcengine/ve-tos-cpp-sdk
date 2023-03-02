#pragma once

#include "model/RequestInfo.h"
#include "CopyObjectV2Output.h"
#include "TosError.h"
#include "utils/BaseUtils.h"
namespace VolcengineTos {
class CopyObjectInner {
public:
    const CopyObjectV2Output& getOutput() const {
        return output_;
    }
    void setOutput(const CopyObjectV2Output& output) {
        output_ = output;
    }
    const TosError& getTosError() const {
        return tosError_;
    }
    void setTosError(const TosError& tosError) {
        tosError_ = tosError;
    }
    void fromJsonString(const std::string& input);

private:
    CopyObjectV2Output output_;
    TosError tosError_;
};
}  // namespace VolcengineTos
