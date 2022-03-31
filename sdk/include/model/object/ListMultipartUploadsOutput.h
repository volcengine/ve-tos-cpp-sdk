#pragma once

#include <string>
#include "model/RequestInfo.h"
#include "UploadInfo.h"
#include "UploadCommonPrefix.h"

namespace VolcengineTos {
class ListMultipartUploadsOutput {
public:
  void fromJsonString(const std::string & input);
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::string &getBucket() const { return bucket_; }
  void setBucket(const std::string &bucket) { bucket_ = bucket; }
  const std::string &getKeyMarker() const { return keyMarker_; }
  void setKeyMarker(const std::string &keyMarker) { keyMarker_ = keyMarker; }
  const std::string &getNextKeyMarker() const { return nextKeyMarker_; }
  void setNextKeyMarker(const std::string &nextKeyMarker) { nextKeyMarker_ = nextKeyMarker; }
  const std::string &getUploadIdMarker() const { return uploadIDMarker_; }
  void setUploadIdMarker(const std::string &uploadIdMarker) { uploadIDMarker_ = uploadIdMarker; }
  const std::string &getNextUploadIdMarker() const { return nextUploadIdMarker_; }
  void setNextUploadIdMarker(const std::string &nextUploadIdMarker) { nextUploadIdMarker_ = nextUploadIdMarker; }
  const std::string &getDelimiter() const { return delimiter_; }
  void setDelimiter(const std::string &delimiter) { delimiter_ = delimiter; }
  const std::string &getPrefix() const { return prefix_; }
  void setPrefix(const std::string &prefix) { prefix_ = prefix; }
  int getMaxUploads() const { return maxUploads_; }
  void setMaxUploads(int maxUploads) { maxUploads_ = maxUploads; }
  bool isTruncated() const { return isTruncated_; }
  void setIsTruncated(bool isTruncated) { isTruncated_ = isTruncated; }
  const std::vector<UploadInfo> &getUpload() const { return upload_; }
  void setUpload(const std::vector<UploadInfo> &upload) { upload_ = upload; }
  const std::vector<UploadCommonPrefix> &getCommonPrefixes() const { return commonPrefixes_; }
  void setCommonPrefixes(const std::vector<UploadCommonPrefix> &commonPrefixes) { commonPrefixes_ = commonPrefixes; }

private:
  RequestInfo requestInfo_;
  std::string bucket_;
  std::string keyMarker_;
  std::string nextKeyMarker_;
  std::string uploadIDMarker_;
  std::string nextUploadIdMarker_;
  std::string delimiter_;
  std::string prefix_;
  int maxUploads_;
  bool isTruncated_;
  std::vector<UploadInfo> upload_;
  std::vector<UploadCommonPrefix> commonPrefixes_;
};
} // namespace VolcengineTos