#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class DeleteBucketCORSOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }

private:
    RequestInfo requestInfo_;
};
}  // namespace VolcengineTos
