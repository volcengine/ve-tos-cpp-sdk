#pragma once
#include "HeadObjectV2Output.h"
#include "CompleteMultipartUploadOutput.h"
namespace VolcengineTos {
class DownloadFileOutput {
public:
    const HeadObjectV2Output& getHeadObjectV2Output() const {
        return headObjectV2Output_;
    }
    void setHeadObjectV2Output(const HeadObjectV2Output& headobjectv2output) {
        headObjectV2Output_ = headobjectv2output;
    }

private:
    HeadObjectV2Output headObjectV2Output_;
};
}  // namespace VolcengineTos
