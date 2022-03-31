#include "model/object/ListMultipartUploadsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

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
      UploadInfo ui;
      ui.fromJsonString(up);
      upload_.emplace_back(ui);
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