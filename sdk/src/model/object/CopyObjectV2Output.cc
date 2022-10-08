#include "model/object/CopyObjectV2Output.h"
#include "../src/external/json/json.hpp"
#include "utils/BaseUtils.h"
using namespace nlohmann;

void VolcengineTos::CopyObjectV2Output::fromJsonString(const std::string& copy) {
    auto j = json::parse(copy);

    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
    if (j.contains("LastModified")) {
        lastModified_ = TimeUtils::transLastModifiedStringToTime(j.at("LastModified").get<std::string>());
    }
}
