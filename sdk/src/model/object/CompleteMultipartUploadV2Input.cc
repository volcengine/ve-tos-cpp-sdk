#include "../src/external/json/json.hpp"
#include "model/object/CompleteMultipartUploadV2Input.h"

std::string VolcengineTos::CompleteMultipartUploadV2Input::toJsonString() const {
    nlohmann::json j;
    nlohmann::json partsArray = nlohmann::json::array();
    for (auto& p : parts_) {
        nlohmann::json parts;
        auto partNumber_ = std::to_string(p.getPartNumber());
        if (!partNumber_.empty())
            parts["PartNumber"] = p.getPartNumber();
        if (!p.getETag().empty())
            parts["ETag"] = p.getETag();
        partsArray.push_back(parts);
    }
    if (!partsArray.empty())
        j["Parts"] = partsArray;
    return j.dump();
}
