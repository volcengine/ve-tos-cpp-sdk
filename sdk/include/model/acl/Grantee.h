#pragma once

#include <string>
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class Grantee{
public:
  void fromJsonString(const nlohmann::json & grantee);
  const std::string &getId() const { return id_; }
  void setId(const std::string &id) { id_ = id; }
  const std::string &getDisplayName() const { return displayName_; }
  void setDisplayName(const std::string &displayName) { displayName_ = displayName; }
  const std::string &getType() const { return type_; }
  void setType(const std::string &type) { type_ = type; }
  const std::string &getUri() const { return uri_; }
  void setUri(const std::string &uri) { uri_ = uri; }

private:
  std::string id_;
  std::string displayName_;
  std::string type_;
  std::string uri_;
};
} // namespace VolcengineTos