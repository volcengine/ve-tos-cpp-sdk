#pragma once
#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "utils/BaseUtils.h"

#include <ctime>
#include <string>

namespace VolcengineTos {
class CopyConditionBase : virtual public Headers {
public:
    CopyConditionBase() = default;
    ~CopyConditionBase() override = default;

    const std::string& getCopySourceIfMatch() const {
        return copySourceIfMatch_;
    }
    void setCopySourceIfMatch(const std::string& value) {
        copySourceIfMatch_ = value;
    }
    time_t getCopySourceIfModifiedSince() const {
        return copySourceIfModifiedSince_;
    }
    void setCopySourceIfModifiedSince(time_t value) {
        copySourceIfModifiedSince_ = value;
    }
    const std::string& getCopySourceIfNoneMatch() const {
        return copySourceIfNoneMatch_;
    }
    void setCopySourceIfNoneMatch(const std::string& value) {
        copySourceIfNoneMatch_ = value;
    }
    time_t getCopySourceIfUnmodifiedSince() const {
        return copySourceIfUnmodifiedSince_;
    }
    void setCopySourceIfUnmodifiedSince(time_t value) {
        copySourceIfUnmodifiedSince_ = value;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_COPY_SOURCE_IF_MATCH, copySourceIfMatch_);
        addHeader(HEADER_COPY_SOURCE_IF_MODIFIED_SINCE, TimeUtils::transTimeToGmtTime(copySourceIfModifiedSince_));
        addHeader(HEADER_COPY_SOURCE_IF_NONE_MATCH, copySourceIfNoneMatch_);
        addHeader(HEADER_COPY_SOURCE_IF_UNMODIFIED_SINCE, TimeUtils::transTimeToGmtTime(copySourceIfUnmodifiedSince_));
    }

private:
    std::string copySourceIfMatch_;
    std::time_t copySourceIfModifiedSince_ = 0;
    std::string copySourceIfNoneMatch_;
    std::time_t copySourceIfUnmodifiedSince_ = 0;
};
}  // namespace VolcengineTos
