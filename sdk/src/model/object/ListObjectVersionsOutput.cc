#include "model/object/ListObjectVersionsOutput.h"
using namespace nlohmann;

void VolcengineTos::ListObjectVersionsOutput::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  if (j.contains("Name")) j.at("Name").get_to(name_);
  if (j.contains("Prefix")) j.at("Prefix").get_to(prefix_);
  if (j.contains("KeyMarker")) j.at("KeyMarker").get_to(keyMarker_);
  if (j.contains("VersionIdMarker")) j.at("VersionIdMarker").get_to(versionIDMarker_);
  if (j.contains("Delimiter")) j.at("Delimiter").get_to(delimiter_);
  if (j.contains("EncodingType")) j.at("EncodingType").get_to(encodingType_);
  if (j.contains("MaxKeys")) j.at("MaxKeys").get_to(maxKeys_);
  if (j.contains("NextKeyMarker")) j.at("NextKeyMarker").get_to(nextKeyMarker_);
  if (j.contains("NextVersionIdMarker")) j.at("NextVersionIdMarker").get_to(nextVersionIDMarker_);
  if (j.contains("IsTruncated")) j.at("IsTruncated").get_to(isTruncated_);
  if (j.contains("CommonPrefixes")) {
    json pres = j.at("CommonPrefixes");
    for (auto & pre : pres) {
      ListedCommonPrefix lcp;
      std::string prefix;
      if (pre.contains("Prefix")) pre.at("Prefix").get_to(prefix);
      lcp.setPrefix(prefix);
      commonPrefixes_.emplace_back(lcp);
    }
  }
  if (j.contains("Versions")) {
    json versions = j.at("Versions");
    for (auto &v : versions) {
      ListedObjectVersion lov;
      lov.fromJsonString(v);
      versions_.emplace_back(lov);
    }
  }
  if (j.contains("DeleteMarkers")) {
    json dms = j.at("DeleteMarkers");
    for (auto & dm : dms) {
      ListedDeleteMarkerEntry ldme;
      ldme.fromJsonString(dm);
      deleteMarkers_.emplace_back(ldme);
    }
  }
}