#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class AbortInCompleteMultipartUpload {
public:
    explicit AbortInCompleteMultipartUpload(int daysAfterInitiation) : daysAfterInitiation(daysAfterInitiation) {
    }
    AbortInCompleteMultipartUpload() = default;
    virtual ~AbortInCompleteMultipartUpload() = default;
    int getDaysAfterInitiation() const {
        return daysAfterInitiation;
    }
    void setDaysAfterInitiation(int daysAfterInitiation) {
        AbortInCompleteMultipartUpload::daysAfterInitiation = daysAfterInitiation;
    }

private:
    int daysAfterInitiation = 0;
};
}  // namespace VolcengineTos
