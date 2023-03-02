#include "../src/external/json/json.hpp"

#include "model/bucket/GetBucketVersioningOutput.h"

void VolcengineTos::GetBucketVersioningOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("Status")) {
        status_ = StringtoVersioningStatusType[j.at("Status").get<std::string>()];
    }
}
