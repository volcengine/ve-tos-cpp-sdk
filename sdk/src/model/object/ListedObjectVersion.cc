#include "model/object/ListedObjectVersion.h"
using namespace nlohmann;

void VolcengineTos::ListedObjectVersion::fromJsonString(const nlohmann::json &version) {
  if (version.contains("Key")) version.at("Key").get_to(key_);
  if (version.contains("IsLatest")) version.at("IsLatest").get_to(isLatest_);
  if (version.contains("LastModified")) version.at("LastModified").get_to(lastModified_);
  if (version.contains("ETag")) version.at("ETag").get_to(etag_);
  if (version.contains("Size")) version.at("Size").get_to(size_);
  if (version.contains("Owner")){
    std::string id;
    std::string name;
    if (version.at("Owner").contains("ID")) version.at("Owner").at("ID").get_to(id);
    if (version.at("Owner").contains("DisplayName")) version.at("Owner").at("DisplayName").get_to(name);
    owner_.setId(id);
    owner_.setDisplayName(name);
  }
  if (version.contains("StorageClass")) version.at("StorageClass").get_to(storageClass_);
  if (version.contains("Type")) version.at("Type").get_to(type_);
  if (version.contains("VersionId")) version.at("VersionId").get_to(versionID_);
}