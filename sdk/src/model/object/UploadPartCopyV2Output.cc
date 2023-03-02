#include "model/object/UploadPartCopyV2Output.h"
#include "../src/external/json/json.hpp"
#include "utils/BaseUtils.h"

void VolcengineTos::UploadPartCopyV2Output::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
    if (j.contains("LastModified")) {
        std::time_t lastModified = TimeUtils::transLastModifiedStringToTime(j.at("LastModified").get<std::string>());
        lastModified_ = lastModified;
    }
}
