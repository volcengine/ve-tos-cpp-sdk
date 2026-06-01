#pragma once
#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "utils/BaseUtils.h"

#include <string>

namespace VolcengineTos {
class SymlinkBase : virtual public Headers {
public:
    SymlinkBase() = default;
    ~SymlinkBase() override = default;

    const std::string& getSymlinkTargetKey() const {
        return symlinkTargetKey_;
    }
    void setSymlinkTargetKey(const std::string& value) {
        symlinkTargetKey_ = value;
    }

    const std::string& getSymlinkTargetBucket() const {
        return symlinkTargetBucket_;
    }
    void setSymlinkTargetBucket(const std::string& value) {
        symlinkTargetBucket_ = value;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_SYMLINK_TARGET, symlinkTargetKey_);
        addHeader(HEADER_SYMLINK_BUCKET, symlinkTargetBucket_);
    }

    void headers2Output(HttpResponse& response) override {
        symlinkTargetKey_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_SYMLINK_TARGET);
        symlinkTargetBucket_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_SYMLINK_BUCKET);
    }

private:
    std::string symlinkTargetKey_;
    std::string symlinkTargetBucket_;
};
}  // namespace VolcengineTos
