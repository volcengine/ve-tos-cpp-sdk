#pragma once

#include "model/RequestInfo.h"
#include "ObjectMeta.h"

namespace VolcengineTos {
class [[gnu::deprecated]] GetObjectOutput {
public:
    void setObjectMetaFromResponse(TosResponse& response);

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
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream> content) {
        content_ = content;
    }
    const ObjectMeta& getObjectMeta() const {
        return objectMeta_;
    }
    void setObjectMeta(const ObjectMeta& objectMeta) {
        objectMeta_ = objectMeta;
    }

private:
    RequestInfo requestInfo_;
    std::string contentRange_;
    std::shared_ptr<std::iostream> content_;
    ObjectMeta objectMeta_;
};
}  // namespace VolcengineTos
