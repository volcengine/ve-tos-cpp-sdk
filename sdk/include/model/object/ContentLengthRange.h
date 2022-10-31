#pragma once

#include <string>
namespace VolcengineTos {
class ContentLengthRange {
public:
    ContentLengthRange(int64_t rangeStart, int64_t rangeEnd) : rangeStart_(rangeStart), rangeEnd_(rangeEnd) {
    }
    ContentLengthRange() = default;
    virtual ~ContentLengthRange() = default;
    int64_t getRangeStart() const {
        return rangeStart_;
    }
    void setRangeStart(int64_t rangeStart) {
        rangeStart_ = rangeStart;
    }
    int64_t getRangeEnd() const {
        return rangeEnd_;
    }
    void setRangeEnd(int64_t rangeEnd) {
        rangeEnd_ = rangeEnd;
    }

private:
    int64_t rangeStart_ = 0;
    int64_t rangeEnd_ = 0;
};
}  // namespace VolcengineTos
