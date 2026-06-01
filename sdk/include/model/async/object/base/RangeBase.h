#pragma once

#include "RequestBuilder.h"
#include "model/async/base/Headers.h"

#include <cstdint>
#include <string>

namespace VolcengineTos {
class RangeBase : virtual public Headers {
public:
    RangeBase() = default;
    ~RangeBase() override = default;

    int64_t getRangeStart() const {
        return rangeStart_;
    }
    void setRangeStart(const int64_t value) {
        rangeStart_ = value;
    }
    int64_t getRangeEnd() const {
        return rangeEnd_;
    }
    void setRangeEnd(const int64_t value) {
        rangeEnd_ = value;
    }

    const std::string& getRange() const {
        return range_;
    }
    void setRange(const std::string& value) {
        range_ = value;
    }

protected:
    std::string valid() override {
        return isValidRange(rangeStart_, rangeEnd_);
    }

    void input2Headers() override {
        if (!range_.empty()) {
            addHeader(http::HEADER_RANGE, range_);
        } else if (rangeStart_ != 0 || rangeEnd_ != 0) {
            HttpRange range_;
            range_.setStart(rangeStart_);
            range_.setEnd(rangeEnd_);
            addHeader(http::HEADER_RANGE, range_.toString());
        }
    }

    static std::string isValidRange(int64_t start, int64_t end) {
        if (start == 0 && end == 0) {
            return "";
        }
        if (start < 0 || end < 0) {
            return "invalid range format";
        }

        if (end != 0 && end < start) {
            return "invalid range format";
        }
        return "";
    }

private:
    int64_t rangeStart_ = 0;
    int64_t rangeEnd_ = 0;
    std::string range_;
};

}  // namespace VolcengineTos
