#include "model/acl/GetObjectAclOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::GetObjectAclOutput::fromJsonString(const std::string &input) {
  auto j = json::parse(input);
  if (j.contains("VersionId")) j.at("VersionId").get_to(versionId_);
  if (j.contains("Owner")) {
    std::string id;
    if (j.at("Owner").contains("VersionId")) j.at("Owner").at("ID").get_to(id);
    std::string displayName;
    if (j.at("Owner").contains("DisplayName"))j.at("Owner").at("DisplayName").get_to(displayName);
    owner_.setId(id);
    owner_.setDisplayName(displayName);
  }
  json grants = j.at("Grants");
  for (auto & grant : grants) {
    Grant g;
    Grantee ge;
    if (grant.contains("Grantee")) ge.fromJsonString(grant.at("Grantee"));
    g.setGrantee(ge);
    std::string permission;
    if (grant.contains("Permission")) grant.at("Permission").get_to(permission);
    g.setPermission(permission);
    grant_.emplace_back(g);
  }
}