#pragma once

#include "model/RequestInfo.h"

#include "TosError.h"
#include "utils/BaseUtils.h"
#include "UploadPartCopyV2Output.h"
namespace VolcengineTos {
class UploadPartCopyInner {
public:
    const UploadPartCopyV2Output& getOutput() const {
        return output_;
    }
    void setOutput(const UploadPartCopyV2Output& output) {
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
    UploadPartCopyV2Output output_;
    TosError tosError_;
};
}  // namespace VolcengineTos
