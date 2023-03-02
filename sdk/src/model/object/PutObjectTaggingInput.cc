#include "../src/external/json/json.hpp"

#include "model/object/PutObjectTaggingInput.h"

std::string VolcengineTos::PutObjectTaggingInput::toJsonString() const {
    nlohmann::json j;
    nlohmann::json tagSet;
    if (!tagSet_.getTags().empty()) {
        nlohmann::json tagArray = nlohmann::json::array();
        for (auto& t : tagSet_.getTags()) {
            nlohmann::json tag;
            if (!t.getKey().empty()) {
                tag["Key"] = t.getKey();
            }
            if (!t.getValue().empty()) {
                tag["Value"] = t.getValue();
            }
            tagArray.push_back(tag);
        }
        if (!tagArray.empty()) {
            j["Tags"] = tagArray;
            tagSet["TagSet"] = j;
        }
    }
    return tagSet.dump();
}
