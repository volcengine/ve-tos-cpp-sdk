#include "model/bucket/PutBucketRenameInput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;
std::string VolcengineTos::PutBucketRenameInput::toJsonString() const {
    nlohmann::json j;
    j["RenameEnable"] = renameEnable_;
    return j.dump();
}
