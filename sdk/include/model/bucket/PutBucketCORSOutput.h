#pragma once

#include <string>
#include <vector>
#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutBucketCORSOutput {
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
