#include "model/object/UploadPartCopyOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::inner::InnerUploadPartCopyOutput::fromJsonString(const std::string& input) {
    auto j = json::parse(input);
    j.at("ETag").get_to(etag_);
    j.at("LastModified").get_to(lastModified_);
}
