#include "model/object/ListUploadedPartsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListUploadedPartsOutput::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  j.at("Bucket").get_to(bucket_);
  j.at("Key").get_to(key_);
  j.at("UploadId").get_to(uploadID_);
  j.at("PartNumberMarker").get_to(partNumberMarker_);
  j.at("NextPartNumberMarker").get_to(nextPartNumberMarker_);
  j.at("MaxParts").get_to(maxParts_);
  j.at("IsTruncated").get_to(isTruncated_);
  j.at("StorageClass").get_to(storageClass_);
  std::string id;
  std::string name;
  j.at("Owner").at("ID").get_to(id);
  if (j.at("Owner").contains("DisplayName"))j.at("Owner").at("DisplayName").get_to(name);
  owner_.setId(id);
  owner_.setDisplayName(name);
  auto ups = j.at("Parts");
  for (auto & up : ups) {
    UploadedPart part;
    part.fromJsonString(up);
    uploadedParts_.emplace_back(part);
  }
}