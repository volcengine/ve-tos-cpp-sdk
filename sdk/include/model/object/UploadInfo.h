#pragma once

#include <string>
#include "model/acl/Owner.h"
namespace VolcengineTos {
class UploadInfo {
public:
  const std::string &getKey() const { return key_; }
  const std::string &getUploadId() const { return uploadID_; }
  const Owner &getOwner() const { return owner_; }
  const std::string &getStorageClass() const { return storageClass_; }
  const std::string &getInitiated() const { return initiated_; }

  void setKey(const std::string &key) { key_ = key; }
  void setUploadId(const std::string &uploadId) { uploadID_ = uploadId; }
  void setOwner(const Owner &owner) { owner_ = owner; }
  void setStorageClass(const std::string &storageClass) { storageClass_ = storageClass; }
  void setInitiated(const std::string &initiated) { initiated_ = initiated; }

private:
  std::string key_;
  std::string uploadID_;
  Owner owner_;
  std::string storageClass_;
  std::string initiated_;
};
} // namespace VolcengineTos