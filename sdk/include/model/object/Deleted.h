#pragma once

#include <string>
namespace VolcengineTos {
class Deleted {
public:
  const std::string &getKey() const { return key_; }
  void setKey(const std::string &key) { key_ = key; }
  const std::string &getVersionId() const { return versionID_; }
  void setVersionId(const std::string &versionId) { versionID_ = versionId; }
  bool isDeleteMarker() const { return deleteMarker_; }
  void setDeleteMarker(bool deleteMarker) { deleteMarker_ = deleteMarker; }
  const std::string &getDeleteMarkerVersionId() const { return deleteMarkerVersionID_; }
  void setDeleteMarkerVersionId(const std::string &deleteMarkerVersionId) { deleteMarkerVersionID_ = deleteMarkerVersionId; }

private:
  std::string key_;
  std::string versionID_;
  bool deleteMarker_;
  std::string deleteMarkerVersionID_;
};
} // namespace VolcengineTos