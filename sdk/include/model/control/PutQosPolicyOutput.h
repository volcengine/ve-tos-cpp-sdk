
#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutQosPolicyOutput {
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
