#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"

namespace VolcengineTos {
class PutBucketNotificationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }

private:
    RequestInfo requestInfo_;
};
}  // namespace VolcengineTos