#pragma once

#include <string>
namespace VolcengineTos {
namespace inner {
class MultipartUpload {
public:
  void fromJsonString(const std::string & input);
  const std::string &getBucket() const { return bucket_; }
  const std::string &getKey() const { return key_; }
  const std::string &getUploadId() const { return uploadID_; }

private:
  std::string bucket_;
  std::string key_;
  std::string uploadID_;
};
} // namespace inner
} // namespace VolcengineTos