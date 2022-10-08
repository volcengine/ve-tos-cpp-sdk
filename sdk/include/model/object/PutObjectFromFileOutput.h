#pragma once

#include "PutObjectV2Output.h"

namespace VolcengineTos {
class PutObjectFromFileOutput {
public:
    const PutObjectV2Output& getPutObjectV2Output() const {
        return putObjectV2Output_;
    }
    void setPutObjectV2Output(const PutObjectV2Output& putobjectv2output) {
        putObjectV2Output_ = putobjectv2output;
    }

private:
    PutObjectV2Output putObjectV2Output_;
};
}  // namespace VolcengineTos
