#pragma once

#include <string>
#include "model/acl/Owner.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class ListedObject {
public:
  void fromJsonString(const nlohmann::json & object);
  const std::string &getKey() const { return key_; }
  void setKey(const std::string &key) { key_ = key; }
  const std::string &getLastModified() const { return lastModified_; }
  void setLastModified(const std::string &lastModified) { lastModified_ = lastModified; }
  const std::string &getEtag() const { return etag_; }
  void setEtag(const std::string &etag) { etag_ = etag; }
  int64_t getSize() const { return size_; }
  void setSize(int64_t size) { size_ = size; }
  const Owner &getOwner() const { return owner_; }
  void setOwner(const Owner &owner) { owner_ = owner; }
  const std::string &getStorageClass() const { return storageClass_; }
  void setStorageClass(const std::string &storageClass) { storageClass_ = storageClass; }
  const std::string &getType() const { return type_; }
  void setType(const std::string &type) { type_ = type; }

private:
  std::string key_;
  std::string lastModified_;
  std::string etag_;
  int64_t size_;
  Owner owner_;
  std::string storageClass_;
  std::string type_;
};
} // namespace VolcengineTos