#pragma once

#include "model/RequestInfo.h"
#include "GetObjectBasicOutput.h"

namespace VolcengineTos {
class GetObjectV2Output {
public:
    void setObjectMetaFromResponse(TosResponse& response) {
        getObjectBasicOutput_.fromResponse(response);
    }
    const GetObjectBasicOutput& getGetObjectBasicOutput() const {
        return getObjectBasicOutput_;
    }
    void setGetObjectBasicOutput(const GetObjectBasicOutput& getobjectbasicoutput) {
        getObjectBasicOutput_ = getobjectbasicoutput;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream> content) {
        content_ = content;
    }

private:
    GetObjectBasicOutput getObjectBasicOutput_;
    std::shared_ptr<std::iostream> content_;
};
}  // namespace VolcengineTos
