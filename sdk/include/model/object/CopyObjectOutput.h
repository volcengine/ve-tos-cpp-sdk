#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class CopyObjectOutput{
public:
  void fromJsonString(const std::string& copy);
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::string &getVersionId() const { return versionID_; }
  void setVersionId(const std::string &versionId) { versionID_ = versionId; }
  const std::string &getSourceVersionId() const { return sourceVersionID_; }
  void setSourceVersionId(const std::string &sourceVersionId) { sourceVersionID_ = sourceVersionId; }
  const std::string &getEtag() const { return etag_; }
  void setEtag(const std::string &etag) { etag_ = etag; }
  const std::string &getLastModified() const { return lastModified_; }
  void setLastModified(const std::string &lastModified) { lastModified_ = lastModified; }

private:
  RequestInfo requestInfo_;
  std::string versionID_;
  std::string sourceVersionID_;
  std::string etag_;
  std::string lastModified_;
};
} // namespace VolcengineTos