#pragma once

#include <string>
#include <map>
#include "TosResponse.h"
#include "Type.h"
namespace VolcengineTos {
class RestoreObjectOutput {
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
