#pragma once

#include <string>
namespace VolcengineTos {
class AbortMultipartUploadInput {
public:
  AbortMultipartUploadInput() = default;
  ~AbortMultipartUploadInput() = default;
  AbortMultipartUploadInput(std::string key, std::string uploadId)
      : key_(std::move(key)), uploadID_(std::move(uploadId)) {}

  const std::string &getKey() const { return key_; }
  void setKey(const std::string &key) { key_ = key; }
  const std::string &getUploadId() const { return uploadID_; }
  void setUploadId(const std::string &uploadId) { uploadID_ = uploadId; }

private:
  std::string key_;
  std::string uploadID_;
};
} // namespace VolcengineTos