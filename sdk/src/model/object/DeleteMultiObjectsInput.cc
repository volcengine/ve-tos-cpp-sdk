#include "model/object/DeleteMultiObjectsInput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::DeleteMultiObjectsInput::toJsonString() {
  json j;
  j["Quiet"] = quiet_;
  json objectToBeDeleteArray = json::array();
  for (auto & o : objectTobeDeleteds_) {
    json object;
    if (!o.getKey().empty()) object["Key"] = o.getKey();
    if (!o.getVersionId().empty()) object["VersionId"] = o.getVersionId();
    objectToBeDeleteArray.push_back(object);
  }
  if (!objectToBeDeleteArray.empty()) j["Objects"] = objectToBeDeleteArray;
  return j.dump();
}
