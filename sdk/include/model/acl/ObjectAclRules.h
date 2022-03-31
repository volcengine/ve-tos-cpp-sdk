#pragma once

#include <vector>
#include "Owner.h"
#include "Grant.h"
namespace VolcengineTos {
class ObjectAclRules {
public:
  std::string toJsonString();
  const Owner &getOwner() const { return owner_; }
  void setOwner(const Owner &owner) { owner_ = owner; }
  const std::vector<Grant> &getGrants() const { return grants_; }
  void setGrants(const std::vector<Grant> &grants) { grants_ = grants; }

private:
  Owner owner_;
  std::vector<Grant> grants_;
};
} // namespace VolcengineTos