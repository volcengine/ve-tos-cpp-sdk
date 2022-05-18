#pragma once

#include "CompleteMultipartUploadOutput.h"
namespace VolcengineTos {
class UploadFileOutput {
public:
  const CompleteMultipartUploadOutput &getOutput() const { return output_; }
  void setOutput(const CompleteMultipartUploadOutput &output) {
    output_ = output;
  }
  const std::string &getBucket() const { return bucket_; }
  void setBucket(const std::string &bucket) { bucket_ = bucket; }
  const std::string &getObjectKey() const { return objectKey_; }
  void setObjectKey(const std::string &objectKey) { objectKey_ = objectKey; }
  const std::string &getUploadId() const { return uploadId_; }
  void setUploadId(const std::string &uploadId) { uploadId_ = uploadId; }

private:
  CompleteMultipartUploadOutput output_;
  std::string bucket_;
  std::string objectKey_;
  std::string uploadId_;
};
} // namespace VolcengineTos