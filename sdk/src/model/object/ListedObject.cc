#include "model/object/ListedObject.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListedObject::fromJsonString(const json &object) {
  if (object.contains("Key")) object.at("Key").get_to(key_);
  if (object.contains("LastModified")) object.at("LastModified").get_to(lastModified_);
  if (object.contains("ETag")) object.at("ETag").get_to(etag_);
  if (object.contains("Size")) object.at("Size").get_to(size_);
  if (object.contains("Owner")){
    std::string id;
    std::string name;
    if (object.at("Owner").contains("ID")) object.at("Owner").at("ID").get_to(id);
    if (object.at("Owner").contains("DisplayName")) object.at("Owner").at("DisplayName").get_to(name);
    owner_.setId(id);
    owner_.setDisplayName(name);
  }
  if (object.contains("StorageClass")) object.at("StorageClass").get_to(storageClass_);
  if (object.contains("Type")) object.at("Type").get_to(type_);
}
