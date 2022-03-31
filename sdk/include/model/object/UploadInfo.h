#pragma once

#include <string>
#include "model/acl/Owner.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class UploadInfo {
public:
  void fromJsonString(const nlohmann::json & object);
  const std::string &getKey() const { return key_; }
  const std::string &getUploadId() const { return uploadID_; }
  const Owner &getOwner() const { return owner_; }
  const std::string &getStorageClass() const { return storageClass_; }
  const std::string &getInitiated() const { return initiated_; }

private:
  std::string key_;
  std::string uploadID_;
  Owner owner_;
  std::string storageClass_;
  std::string initiated_;
};
} // namespace VolcengineTos