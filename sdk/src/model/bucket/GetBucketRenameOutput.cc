#include "model/bucket/GetBucketRenameOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::GetBucketRenameOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("RenameEnable")) {
        setRenameEnable(j.at("RenameEnable").get<bool>());
    }
}
