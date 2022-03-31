#include "model/acl/ObjectAclRules.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::ObjectAclRules::toJsonString() {
  json j;
  if (!owner_.getId().empty()) j["Owner"]["ID"] = owner_.getId();
  if (!owner_.getDisplayName().empty()) j["Owner"]["DisplayName"] = owner_.getDisplayName();
  json grantArray = json::array();
  for (auto & g : grants_) {
    json grant;
    if (!g.getGrantee().getId().empty()) grant["Grantee"]["ID"] = g.getGrantee().getId();
    if (!g.getGrantee().getDisplayName().empty()) grant["Grantee"]["DisplayName"] = g.getGrantee().getDisplayName();
    if (!g.getGrantee().getType().empty()) grant["Grantee"]["Type"] = g.getGrantee().getType();
    if (!g.getGrantee().getUri().empty()) grant["Grantee"]["Canned"] = g.getGrantee().getUri();
    if (!g.getPermission().empty()) grant["Permission"] = g.getPermission();
    grantArray.push_back(grant);
  }
  if (!grantArray.empty()) j["Grants"] = grantArray;
  return j.dump();
}