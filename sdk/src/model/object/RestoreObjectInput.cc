#include "model/object/RestoreObjectInput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;
std::string VolcengineTos::RestoreObjectInput::toJsonString() const {
    nlohmann::json j;
    if (restoreJobParameters_.getTier() != TierType::NotSet) {
        j["RestoreJobParameters"]["Tier"] = TierTypetoString[restoreJobParameters_.getTier()];
    }
    if (days_ != 0) {
        j["Days"] = days_;
    }
    return j.dump();
}
