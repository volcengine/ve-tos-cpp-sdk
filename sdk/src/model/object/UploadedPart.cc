#include "model/object/UploadedPart.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::UploadedPart::fromJsonString(const nlohmann::json &object) {
  object.at("PartNumber").get_to(partNumber_);
  object.at("LastModified").get_to(lastModified_);
  object.at("ETag").get_to(etag_);
  object.at("Size").get_to(size_);
}