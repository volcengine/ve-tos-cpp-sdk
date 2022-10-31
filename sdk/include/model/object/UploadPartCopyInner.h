#pragma once

#include "model/RequestInfo.h"

#include "TosError.h"
#include "../src/external/json/json.hpp"
#include "utils/BaseUtils.h"
#include "UploadPartCopyV2Output.h"
namespace VolcengineTos {
class UploadPartCopyInner {
public:
    const UploadPartCopyV2Output& getOutput() const {
        return output_;
    }
    void setOutput(const UploadPartCopyV2Output& output) {
        output_ = output;
    }
    const TosError& getTosError() const {
        return tosError_;
    }
    void setTosError(const TosError& tosError) {
        tosError_ = tosError;
    }
    void fromJsonString(const std::string& intput) {
        auto j = nlohmann::json::parse(intput);

        if (j.contains("ETag")) {
            output_.setETag(j.at("ETag").get<std::string>());
        }
        if (j.contains("LastModified")) {
            auto lastModified_ = TimeUtils::transLastModifiedStringToTime(j.at("LastModified").get<std::string>());
            output_.setLastModified(lastModified_);
        }

        if (j.contains("Code"))
            tosError_.setCode(j.at("Code").get<std::string>());
        if (j.contains("Message"))
            tosError_.setMessage(j.at("Message").get<std::string>());
        if (j.contains("RequestId"))
            tosError_.setRequestId(j.at("RequestId").get<std::string>());
        if (j.contains("HostId"))
            tosError_.setHostId(j.at("HostId").get<std::string>());
    }

private:
    UploadPartCopyV2Output output_;
    TosError tosError_;
};
}  // namespace VolcengineTos
