#include "../src/external/json/json.hpp"

#include "model/object/GetObjectTaggingOutput.h"
#include "model/bucket/Tag.h"

void VolcengineTos::GetObjectTaggingOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("TagSet")) {
        nlohmann::json tags = j.at("TagSet");
        if (tags.contains("Tags")) {
            auto tag = tags.at("Tags");
            std::vector<Tag> tags_;
            for (auto& t : tag) {
                Tag tag_;
                if (t.contains("Key")) {
                    tag_.setKey(t.at("Key").get<std::string>());
                }
                if (t.contains("Value")) {
                    tag_.setValue(t.at("Value").get<std::string>());
                }
                tags_.emplace_back(tag_);
            }
            tagSet_.setTags(tags_);
        }
    }
}
