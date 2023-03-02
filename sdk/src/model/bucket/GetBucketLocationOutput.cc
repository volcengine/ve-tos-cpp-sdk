#include "model/bucket/GetBucketLocationOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::GetBucketLocationOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("Region")) {
        setRegion(j.at("Region").get<std::string>());
    }
    if (j.contains("ExtranetEndpoint")) {
        setExtranetEndpoint(j.at("ExtranetEndpoint").get<std::string>());
    }
    if (j.contains("IntranetEndpoint")) {
        setIntranetEndpoint(j.at("IntranetEndpoint").get<std::string>());
    }
}
