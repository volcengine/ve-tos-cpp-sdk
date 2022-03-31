#include "model/acl/Grantee.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::Grantee::fromJsonString(const nlohmann::json &grantee) {
  if (grantee.contains("ID")) grantee.at("ID").get_to(id_);
  if (grantee.contains("DisplayName")) grantee.at("DisplayName").get_to(displayName_);
  if (grantee.contains("Type")) grantee.at("Type").get_to(type_);
  if (grantee.contains("Canned")) grantee.at("Canned").get_to(uri_);
}