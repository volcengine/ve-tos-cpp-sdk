#pragma once

#include "common/Common.h"
#include "RangeBase.h"

#include <string>

namespace VolcengineTos {
class CopySourceRangeBase : public RangeBase {
public:
    CopySourceRangeBase() = default;
    ~CopySourceRangeBase() override = default;

    int64_t getCopySourceRangeStart() const {
        return getRangeStart();
    }
    void setCopySourceRangeStart(const int64_t value) {
        return setRangeStart(value);
    }
    int64_t getCopySourceRangeEnd() const {
        return getRangeEnd();
    }
    void setCopySourceRangeEnd(const int64_t value) {
        return setRangeEnd(value);
    }
    const std::string& getCopySourceRange() const {
        return getRange();
    }
    void setCopySourceRange(const std::string& value) {
        setRange(value);
    }

protected:
    void input2Headers() override {
        if (!getCopySourceRange().empty()) {
            addHeader(HEADER_COPY_SOURCE_RANGE, getCopySourceRange());
        } else if (getCopySourceRangeStart() != 0 || getCopySourceRangeEnd() != 0) {
            HttpRange range_;
            range_.setStart(getCopySourceRangeStart());
            range_.setEnd(getCopySourceRangeEnd());
            addHeader(HEADER_COPY_SOURCE_RANGE, range_.toString());
        }
    }
};
}  // namespace VolcengineTos
