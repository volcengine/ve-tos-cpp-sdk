#include "model/object/CompleteMultipartUploadInput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::inner::InnerCompleteMultipartUploadInput::toJsonString() {
  json j;
  json jParts = json::array();
  for (auto & part : parts_) {
    json jPart;
    jPart["PartNumber"] = part.getPartNumber();
    jPart["ETag"] = part.getEtag();
    jParts.emplace_back(jPart);
  }
  j["Parts"] = jParts;
  return j.dump();
}