#include "../src/external/json/json.hpp"
#include "model/object/FetchObjectOutput.h"

void VolcengineTos::FetchObjectOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("VersionId"))
        j.at("VersionId").get_to(versionID_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
}
