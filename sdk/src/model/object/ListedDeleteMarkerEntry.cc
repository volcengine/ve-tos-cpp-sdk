#include "model/object/ListedDeleteMarkerEntry.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListedDeleteMarkerEntry::fromJsonString(const nlohmann::json &marker) {
  if (marker.contains("Key")) marker.at("Key").get_to(key_);
  if (marker.contains("IsLatest")) marker.at("IsLatest").get_to(isLatest_);
  if (marker.contains("LastModified")) marker.at("LastModified").get_to(lastModified_);
  if (marker.contains("Owner")){
    std::string id;
    std::string name;
    if (marker.at("Owner").contains("ID")) marker.at("Owner").at("ID").get_to(id);
    if (marker.at("Owner").contains("DisplayName")) marker.at("Owner").at("DisplayName").get_to(name);
    owner_.setId(id);
    owner_.setDisplayName(name);
  }
  if (marker.contains("VersionId")) marker.at("VersionId").get_to(versionID_);
}