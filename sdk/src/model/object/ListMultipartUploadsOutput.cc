#include "model/object/ListMultipartUploadsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

VolcengineTos::UploadInfo parseUploadInfo(const json &j) {
  VolcengineTos::UploadInfo info;
  std::string tmp;
  if (j.contains("Key")){
    j.at("Key").get_to(tmp);
    info.setKey(tmp);
  }
  if (j.contains("UploadId")) {
    j.at("UploadId").get_to(tmp);
    info.setUploadId(tmp);
  }
  if (j.contains("Owner")) {
    VolcengineTos::Owner owner;
    if (j.at("Owner").contains("ID")) {
      j.at("Owner").at("ID").get_to(tmp);
      owner.setId(tmp);
    }
    if (j.at("Owner").contains("DisplayName")) {
      j.at("Owner").at("DisplayName").get_to(tmp);
      owner.setDisplayName(tmp);
    }
    info.setOwner(owner);
  }
  if (j.contains("StorageClass")) {
    j.at("StorageClass").get_to(tmp);
    info.setStorageClass(tmp);
  }
  if (j.contains("Initiated")) {
    j.at("Initiated").get_to(tmp);
    info.setInitiated(tmp);
  }
  return info;
}

void VolcengineTos::ListMultipartUploadsOutput::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  if (j.contains("Bucket")) j.at("Bucket").get_to(bucket_);
  if (j.contains("KeyMarker")) j.at("KeyMarker").get_to(keyMarker_);
  if (j.contains("NextKeyMarker")) j.at("NextKeyMarker").get_to(nextKeyMarker_);
  if (j.contains("UploadIdMarker")) j.at("UploadIdMarker").get_to(uploadIDMarker_);
  if (j.contains("NextUploadIdMarker")) j.at("NextUploadIdMarker").get_to(nextUploadIdMarker_);
  if (j.contains("Delimiter")) j.at("Delimiter").get_to(delimiter_);
  if (j.contains("Prefix")) j.at("Prefix").get_to(prefix_);
  if (j.contains("MaxUploads")) j.at("MaxUploads").get_to(maxUploads_);
  if (j.contains("IsTruncated")) j.at("IsTruncated").get_to(isTruncated_);
  if (j.contains("Uploads")) {
    auto ups = j.at("Uploads");
    for (auto & up : ups) {
      upload_.emplace_back(parseUploadInfo(up));
    }
  }
  if (j.contains("CommonPrefixes")) {
    auto cps = j.at("CommonPrefixes");
    for (auto & cp : cps) {
      UploadCommonPrefix ucp;
      std::string prefix;
      if (cp.contains("Prefix")) cp.at("Prefix").get_to(prefix);
      ucp.setPrefix(prefix);
      commonPrefixes_.emplace_back(ucp);
    }
  }
}