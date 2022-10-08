#pragma once

#include "model/RequestInfo.h"
#include "ObjectMeta.h"
#include "GetObjectBasicOutput.h"

namespace VolcengineTos {
class GetObjectToFileOutput {
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

private:
    GetObjectBasicOutput getObjectBasicOutput_;
};
}  // namespace VolcengineTos
