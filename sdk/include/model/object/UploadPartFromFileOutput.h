#pragma once

#include "model/RequestInfo.h"
#include "UploadPartV2Output.h"
namespace VolcengineTos {
class UploadPartFromFileOutput {
public:
    const UploadPartV2Output& getUploadPartV2Output() const {
        return uploadPartV2Output_;
    }
    void setUploadPartV2Output(const UploadPartV2Output& uploadpartv2output) {
        uploadPartV2Output_ = uploadpartv2output;
    }

private:
    UploadPartV2Output uploadPartV2Output_;
};
}  // namespace VolcengineTos
