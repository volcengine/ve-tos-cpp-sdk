#include "model/object/ListObjectVersionsOutput.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

VolcengineTos::Owner parseOwner(const json & o) {
  VolcengineTos::Owner owner;
  if (o.contains("ID")) {
    owner.setId(o.at("ID").get<std::string>());
  }
  if (o.contains("DisplayName")) {
    owner.setDisplayName(o.at("DisplayName").get<std::string>());
  }
  return owner;
}

VolcengineTos::ListedDeleteMarkerEntry parseListedDeleteMarkerEntry(const json& marker){
  VolcengineTos::ListedDeleteMarkerEntry entry;
  if (marker.contains("Key")) entry.setKey(marker.at("Key").get<std::string>());
  if (marker.contains("IsLatest")) entry.setIsLatest(marker.at("IsLatest").get<bool>());
  if (marker.contains("LastModified")) entry.setLastModified(marker.at("LastModified").get<std::string>());
  if (marker.contains("Owner")) {
    entry.setOwner(parseOwner(marker.at("Owner")));
  }
  if (marker.contains("VersionId")) entry.setVersionId(marker.at("VersionId").get<std::string>());
  return entry;
}

VolcengineTos::ListedObjectVersion parseListedObjectVersion(const json& version) {
  VolcengineTos::ListedObjectVersion listedObjectVersion;
  if (version.contains("Key")) listedObjectVersion.setKey(version.at("Key").get<std::string>());
  if (version.contains("IsLatest")) listedObjectVersion.setIsLatest(version.at("IsLatest").get<bool>());
  if (version.contains("LastModified")) listedObjectVersion.setLastModified(version.at("LastModified").get<std::string>());
  if (version.contains("ETag")) listedObjectVersion.setVersionId(version.at("ETag").get<std::string>());
  if (version.contains("Size")) listedObjectVersion.setSize(version.at("Size").get<int64_t>());
  if (version.contains("Owner")){
    listedObjectVersion.setOwner(parseOwner(version.at("Owner")));
  }
  if (version.contains("StorageClass")) listedObjectVersion.setStorageClass(version.at("StorageClass").get<std::string>());
  if (version.contains("Type")) listedObjectVersion.setType(version.at("Type").get<std::string>());
  if (version.contains("VersionId")) listedObjectVersion.setVersionId(version.at("VersionId").get<std::string>());
  return listedObjectVersion;
}

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
      if (pre.contains("Prefix")) {
        lcp.setPrefix(pre.at("Prefix").get<std::string>());
      }
      commonPrefixes_.emplace_back(lcp);
    }
  }
  if (j.contains("Versions")) {
    json versions = j.at("Versions");
    for (auto &v : versions) {
      versions_.emplace_back(parseListedObjectVersion(v));
    }
  }
  if (j.contains("DeleteMarkers")) {
    json dms = j.at("DeleteMarkers");
    for (auto & dm : dms) {
      deleteMarkers_.emplace_back(parseListedDeleteMarkerEntry(dm));
    }
  }
}