#pragma once

#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "utils/BaseUtils.h"

#include <ctime>
#include <string>

namespace VolcengineTos {
class ConditionBase : virtual public Headers {
public:
    ConditionBase() = default;
    ~ConditionBase() override = default;

    const std::string& getIfMatch() const {
        return ifMatch_;
    }
    void setIfMatch(const std::string& value) {
        ifMatch_ = value;
    }
    time_t getIfModifiedSince() const {
        return ifModifiedSince_;
    }
    void setIfModifiedSince(const time_t value) {
        ifModifiedSince_ = value;
    }
    const std::string& getIfNoneMatch() const {
        return ifNoneMatch_;
    }
    void setIfNoneMatch(const std::string& value) {
        ifNoneMatch_ = value;
    }
    time_t getIfUnmodifiedSince() const {
        return ifUnmodifiedSince_;
    }
    void setIfUnmodifiedSince(const time_t value) {
        ifUnmodifiedSince_ = value;
    }

protected:
    void input2Headers() override {
        addHeader(http::HEADER_IF_MATCH, ifMatch_);

        addHeader(http::HEADER_IF_MODIFIED_SINCE, TimeUtils::transTimeToGmtTime(ifModifiedSince_));

        addHeader(http::HEADER_IF_NONE_MATCH, ifNoneMatch_);

        addHeader(http::HEADER_IF_UNMODIFIED_SINCE, TimeUtils::transTimeToGmtTime(ifUnmodifiedSince_));
    }

private:
    std::string ifMatch_;
    std::time_t ifModifiedSince_ = 0;
    std::string ifNoneMatch_;
    std::time_t ifUnmodifiedSince_ = 0;
};
}  // namespace VolcengineTos
