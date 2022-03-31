#include "model/object/MultipartUpload.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::inner::MultipartUpload::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  j.at("Bucket").get_to(bucket_);
  j.at("Key").get_to(key_);
  j.at("UploadId").get_to(uploadID_);
}