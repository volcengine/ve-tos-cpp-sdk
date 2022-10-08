#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class SetObjectMetaOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        SetObjectMetaOutput::requestInfo_ = requestInfo;
    }

private:
    RequestInfo requestInfo_;
};
}  // namespace VolcengineTos
