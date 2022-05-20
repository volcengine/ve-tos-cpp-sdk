
#include <fstream>
#include "model/object/UploadFileCheckpoint.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::UploadFileCheckpoint::dump() {
  json j;
  j["Bucket"] = bucket_;
  j["Key"] = key_;
  j["PartSize"] = partSize_;
  j["UploadID"] = uploadID_;
  j["SSECustomerAlgorithm"] = sseAlgorithm_;
  j["SSECustomerMD5"] = sseKeyMd5_;
  j["UploadFileInfo"] = fileInfo_.dump();
  json jParts = json::array();
  for (auto & part : uploadFilePartInfoList_) {
    jParts.emplace_back(part.dump());
  }
  j["Parts"] = jParts;
  std::ofstream ofs(checkpointFilePath_, std::ios::out | std::ios::trunc);
  ofs << j.dump();
  ofs.close();
}

void VolcengineTos::UploadFileCheckpoint::load() {
  if (checkpointFilePath_.empty()) {
    return;
  }
  std::ifstream ifs(checkpointFilePath_, std::ios::in);
  std::stringstream ss;
  ss << ifs.rdbuf();
  ifs.close();
  auto j = json::parse(ss.str());
  if (j.contains("Bucket")) j.at("Bucket").get_to(bucket_);
  if (j.contains("Key")) j.at("Key").get_to(key_);
  if (j.contains("PartSize")) j.at("PartSize").get_to(partSize_);
  if (j.contains("UploadID")) j.at("UploadID").get_to(uploadID_);
  if (j.contains("SSECustomerAlgorithm")) j.at("SSECustomerAlgorithm").get_to(sseAlgorithm_);
  if (j.contains("SSECustomerMD5")) j.at("SSECustomerMD5").get_to(sseKeyMd5_);
  if (j.contains("UploadFileInfo")) { fileInfo_.load(j.at("UploadFileInfo")); }
  if (j.contains("Parts")) {
    json parts = j.at("Parts");
    for (auto &part : parts) {
      UploadFilePartInfo ufp;
      ufp.load(part);
      uploadFilePartInfoList_.emplace_back(ufp);
    }
  }
}

bool VolcengineTos::UploadFileCheckpoint::isValid(
    int64_t uploadFileSize, int64_t uploadFileLastModifiedTime,
    const std::string &bucket, const std::string &objectKey,
    const std::string &uploadFilePath) {
  if (uploadID_.empty() || bucket_ != bucket || key_ != objectKey || fileInfo_.getFilePath() != uploadFilePath) {
    return false;
  }
  return fileInfo_.getFileSize() == uploadFileSize && fileInfo_.getLastModified() == uploadFileLastModifiedTime;
}
