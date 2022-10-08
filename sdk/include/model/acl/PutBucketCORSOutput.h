#pragma once

#include <string>
#include <vector>
#include "../src/external/json/json.hpp"
#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutBucketCORSOutPut {
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
