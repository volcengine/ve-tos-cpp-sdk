#pragma once

#include "model/RequestInfo.h"
#include "ObjectMeta.h"
namespace VolcengineTos {
class [[gnu::deprecated]] HeadObjectOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getContentRange() const {
        return contentRange_;
    }
    void setContentRange(const std::string& contentRange) {
        contentRange_ = contentRange;
    }
    const ObjectMeta& getObjectMeta() const {
        return objectMeta_;
    }
    void setObjectMeta(TosResponse& response) {
        objectMeta_.fromResponse(response);
    }

private:
    RequestInfo requestInfo_;
    std::string contentRange_;
    ObjectMeta objectMeta_;
};
}  // namespace VolcengineTos
