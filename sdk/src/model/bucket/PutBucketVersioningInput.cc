#include "../src/external/json/json.hpp"

#include "model/bucket/PutBucketVersioningInput.h"

std::string VolcengineTos::PutBucketVersioningInput::toJsonString() const {
    nlohmann::json j;
    if (status_ != VersioningStatusType::NotSet) {
        j["Status"] = VersioningStatusTypetoString[status_];
    }
    return j.dump();
}
