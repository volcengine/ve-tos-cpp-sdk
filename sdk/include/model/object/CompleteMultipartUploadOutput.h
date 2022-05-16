#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CompleteMultipartUploadOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::string &getVersionId() const { return versionID_; }
  void setVersionId(const std::string &versionId) { versionID_ = versionId; }
  const std::string &getCrc64() const { return crc64_; }
  void setCrc64(const std::string &crc64) { crc64_ = crc64; }

private:
  RequestInfo requestInfo_;
  std::string versionID_;
  std::string crc64_;
};
} // namespace VolcengineTos