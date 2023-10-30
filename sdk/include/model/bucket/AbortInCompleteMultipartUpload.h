#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class AbortInCompleteMultipartUpload {
public:
    explicit AbortInCompleteMultipartUpload(int daysAfterInitiation) : daysAfterInitiation_(daysAfterInitiation) {
    }
    AbortInCompleteMultipartUpload() = default;
    virtual ~AbortInCompleteMultipartUpload() = default;
    int getDaysAfterInitiation() const {
        return daysAfterInitiation_;
    }
    void setDaysAfterInitiation(int daysAfterInitiation) {
        daysAfterInitiation_ = daysAfterInitiation;
    }

private:
    int daysAfterInitiation_ = 0;
};
}  // namespace VolcengineTos
