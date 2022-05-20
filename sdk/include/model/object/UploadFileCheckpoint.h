#pragma once

#include <string>
#include <vector>
#include "UploadFileInfo.h"
#include "UploadFilePartInfo.h"
namespace VolcengineTos {
class UploadFileCheckpoint {
public:
  void dump();
  void load();
  const std::string &getBucket() const { return bucket_; }
  void setBucket(const std::string &bucket) { bucket_ = bucket; }
  const std::string &getKey() const { return key_; }
  void setKey(const std::string &key) { key_ = key; }
  int64_t getPartSize() const { return partSize_; }
  void setPartSize(int64_t partSize) { partSize_ = partSize; }
  const std::string &getUploadId() const { return uploadID_; }
  void setUploadId(const std::string &uploadId) { uploadID_ = uploadId; }
  const std::string &getSseAlgorithm() const { return sseAlgorithm_; }
  void setSseAlgorithm(const std::string &sseAlgorithm) {
    sseAlgorithm_ = sseAlgorithm;
  }
  const std::string &getSseKeyMd5() const { return sseKeyMd5_; }
  void setSseKeyMd5(const std::string &sseKeyMd5) { sseKeyMd5_ = sseKeyMd5; }
  const UploadFileInfo &getFileInfo() const { return fileInfo_; }
  void setFileInfo(const UploadFileInfo &fileInfo) { fileInfo_ = fileInfo; }
  const std::string &getCheckpointFilePath() const {
    return checkpointFilePath_;
  }
  void setCheckpointFilePath(const std::string &checkpointFilePath) {
    UploadFileCheckpoint::checkpointFilePath_ = checkpointFilePath;
  }
  const std::vector<UploadFilePartInfo> &getUploadFilePartInfoList() const {
    return uploadFilePartInfoList_;
  }
  void setUploadFilePartInfoList(
      const std::vector<UploadFilePartInfo> &uploadFilePartInfoList) {
    uploadFilePartInfoList_ = uploadFilePartInfoList;
  }
  void setUploadFilePartInfoByIdx(const UploadFilePartInfo &uploadFilePartInfo, int idx) {
    if (idx < 0 || idx >= uploadFilePartInfoList_.size()) return;
    uploadFilePartInfoList_[idx] = uploadFilePartInfo;
  }

  bool isValid(int64_t uploadFileSize, int64_t uploadFileLastModifiedTime,
                  const std::string &bucket, const std::string &objectKey, const std::string &uploadFilePath);

private:
  std::string bucket_;
  std::string key_;
  int64_t partSize_ = 0;
  std::string uploadID_;
  std::string sseAlgorithm_;
  std::string sseKeyMd5_;
  UploadFileInfo fileInfo_;
  std::string checkpointFilePath_;
  std::vector<UploadFilePartInfo> uploadFilePartInfoList_;
};
} // namespace VolcengineTos