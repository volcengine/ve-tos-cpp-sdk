#include "model/object/UploadInfo.h"

void VolcengineTos::UploadInfo::fromJsonString(const nlohmann::json &object) {
  if (object.contains("Key")) object.at("Key").get_to(key_);
  if (object.contains("UploadId")) object.at("UploadId").get_to(uploadID_);
  if (object.contains("Owner")) {
    std::string id;
    std::string name;
    if (object.at("Owner").contains("ID")) object.at("Owner").at("ID").get_to(id);
    if (object.at("Owner").contains("DisplayName")) object.at("Owner").at("DisplayName").get_to(name);
    owner_.setId(id);
    owner_.setDisplayName(name);
  }
  if (object.contains("StorageClass")) object.at("StorageClass").get_to(storageClass_);
  if (object.contains("Initiated")) object.at("Initiated").get_to(initiated_);
}