#include "model/object/ListObjectsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListObjectsOutput::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  if (j.contains("Name")) j.at("Name").get_to(name_);
  if (j.contains("Prefix")) j.at("Prefix").get_to(prefix_);
  if (j.contains("Marker")) j.at("Marker").get_to(marker_);
  if (j.contains("MaxKeys")) j.at("MaxKeys").get_to(maxKeys_);
  if (j.contains("NextMarker")) j.at("NextMarker").get_to(nextMarker_);
  if (j.contains("Delimiter")) j.at("Delimiter").get_to(delimiter_);
  if (j.contains("IsTruncated")) j.at("IsTruncated").get_to(isTruncated_);
  if (j.contains("EncodingType")) j.at("EncodingType").get_to(encodingType_);
  if (j.contains("CommonPrefixes")){
    json pres = j.at("CommonPrefixes");
    for (auto & pre : pres) {
      ListedCommonPrefix lcp;
      std::string prefix;
      if (pre.contains("Prefix")) pre.at("Prefix").get_to(prefix);
      lcp.setPrefix(prefix);
      commonPrefixes_.emplace_back(lcp);
    }
  }
  if (j.contains("Contents")) {
    json contents = j.at("Contents");
    for (auto &ct : contents) {
      ListedObject lo;
      lo.fromJsonString(ct);
      contents_.emplace_back(lo);
    }
  }
}