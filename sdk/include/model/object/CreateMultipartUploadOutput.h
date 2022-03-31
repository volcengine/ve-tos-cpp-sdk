#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CreateMultipartUploadOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::string &getBucket() const { return bucket_; }
  void setBucket(const std::string &bucket) { bucket_ = bucket; }
  const std::string &getKey() const { return key_; }
  void setKey(const std::string &key) { key_ = key; }
  const std::string &getUploadId() const { return uploadID_; }
  void setUploadId(const std::string &uploadId) { uploadID_ = uploadId; }
  const std::string &getSseCustomerAlgorithm() const { return sseCustomerAlgorithm_; }
  void setSseCustomerAlgorithm(const std::string &sseCustomerAlgorithm) { sseCustomerAlgorithm_ = sseCustomerAlgorithm; }
  const std::string &getSseCustomerMd5() const { return sseCustomerMD5_; }
  void setSseCustomerMd5(const std::string &sseCustomerMd5) { sseCustomerMD5_ = sseCustomerMd5; }

private:
  RequestInfo requestInfo_;
  std::string bucket_;
  std::string key_;
  std::string uploadID_;
  std::string sseCustomerAlgorithm_;
  std::string sseCustomerMD5_;
};
} // namespace VolcengineTos