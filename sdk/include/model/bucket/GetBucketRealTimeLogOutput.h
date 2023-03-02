#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "RealTimeLogConfiguration.h"

namespace VolcengineTos {
class GetBucketRealTimeLogOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const RealTimeLogConfiguration& getConfiguration() const {
        return configuration_;
    }
    void setConfiguration(const RealTimeLogConfiguration& configuration) {
        configuration_ = configuration;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    RealTimeLogConfiguration configuration_;
};
}  // namespace VolcengineTos
