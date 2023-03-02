#include "model/object/CompleteMultipartUploadV2Output.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

void VolcengineTos::CompleteMultipartUploadV2Output::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("Location"))
        j.at("Location").get_to(location_);
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("Key"))
        j.at("Key").get_to(key_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
}
