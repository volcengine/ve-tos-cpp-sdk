#pragma once

#include <string>
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class ListedBucket {
public:
  const std::string &getCreationDate() const { return creationDate_; }
  const std::string &getName() const { return name_; }
  const std::string &getLocation() const { return location_; }
  const std::string &getExtranetEndpoint() const { return extranetEndpoint_; }
  const std::string &getIntranetEndpoint() const { return intranetEndpoint_; }

  void fromJsonString(const nlohmann::json & buckets);

private:
  std::string creationDate_;
  std::string name_;
  std::string location_;
  std::string extranetEndpoint_;
  std::string intranetEndpoint_;
};
} // namespace VolcengineTos