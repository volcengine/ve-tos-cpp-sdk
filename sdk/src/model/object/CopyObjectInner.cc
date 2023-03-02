#include "model/object/CopyObjectInner.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::CopyObjectInner::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);

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
