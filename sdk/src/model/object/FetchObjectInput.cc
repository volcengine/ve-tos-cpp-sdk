#include "../src/external/json/json.hpp"

#include "model/object/FetchObjectInput.h"

std::string VolcengineTos::FetchObjectInput::toJsonString() const {
    nlohmann::json j;
    j["URL"] = url_;
    if (ignoreSameKey_)
        j["IgnoreSameKey"] = ignoreSameKey_;
    if (!hexMD5_.empty())
        j["ContentMD5"] = hexMD5_;
    return j.dump();
}
