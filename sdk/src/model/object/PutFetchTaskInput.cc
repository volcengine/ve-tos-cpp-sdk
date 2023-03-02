#include "../src/external/json/json.hpp"

#include "model/object/PutFetchTaskInput.h"

std::string VolcengineTos::PutFetchTaskInput::toJsonString() const {
    nlohmann::json j;
    j["URL"] = url_;
    j["Object"] = key_;
    if (ignoreSameKey_) {
        j["IgnoreSameKey"] = ignoreSameKey_;
    }
    if (!hexMD5_.empty())
        j["ContentMD5"] = hexMD5_;
    return j.dump();
}
