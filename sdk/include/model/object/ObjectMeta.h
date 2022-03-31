#pragma once

#include <string>
#include <map>
#include "TosResponse.h"
namespace VolcengineTos {
class ObjectMeta {
public:
  void fromResponse(TosResponse& res);
  bool operator==(const ObjectMeta &rhs) const;
  bool operator!=(const ObjectMeta &rhs) const;

  int64_t getContentLength() const { return contentLength_; }
  void setContentLength(int64_t contentLength) { contentLength_ = contentLength; }
  const std::string &getContentType() const { return contentType_; }
  void setContentType(const std::string &contentType) {
    contentType_ = contentType;
  }
  const std::string &getContentMd5() const { return contentMD5_; }
  void setContentMd5(const std::string &contentMd5) {
    contentMD5_ = contentMd5;
  }
  const std::string &getContentLanguage() const { return contentLanguage_; }
  void setContentLanguage(const std::string &contentLanguage) {
    contentLanguage_ = contentLanguage;
  }
  const std::string &getContentEncoding() const { return contentEncoding_; }
  void setContentEncoding(const std::string &contentEncoding) {
    contentEncoding_ = contentEncoding;
  }
  const std::string &getContentDisposition() const {
    return contentDisposition_;
  }
  void setContentDisposition(const std::string &contentDisposition) {
    contentDisposition_ = contentDisposition;
  }
  const std::string &getLastModified() const { return lastModified_; }
  void setLastModified(const std::string &lastModified) {
    lastModified_ = lastModified;
  }
  const std::string &getCacheControl() const { return cacheControl_; }
  void setCacheControl(const std::string &cacheControl) {
    cacheControl_ = cacheControl;
  }
  const std::string &getExpires() const { return expires_; }
  void setExpires(const std::string &expires) { expires_ = expires; }
  const std::string &getEtags() const { return etags_; }
  void setEtags(const std::string &etags) { etags_ = etags; }
  const std::string &getVersionId() const { return versionID_; }
  void setVersionId(const std::string &versionId) { versionID_ = versionId; }
  bool isDeleteMarker() const { return deleteMarker_; }
  void setDeleteMarker(bool deleteMarker) { deleteMarker_ = deleteMarker; }
  const std::string &getObjectType() const { return objectType_; }
  void setObjectType(const std::string &objectType) {
    objectType_ = objectType;
  }
  const std::string &getStorageClass() const { return storageClass_; }
  void setStorageClass(const std::string &storageClass) {
    storageClass_ = storageClass;
  }
  const std::string &getRestore() const { return restore_; }
  void setRestore(const std::string &restore) { restore_ = restore; }
  const std::map<std::string, std::string> &getMetadata() const {
    return metadata_;
  }
  void setMetadata(const std::map<std::string, std::string> &metadata) {
    metadata_ = metadata;
  }
  const std::string &getMirrorTag() const { return mirrorTag_; }
  void setMirrorTag(const std::string &mirrorTag) { mirrorTag_ = mirrorTag; }
  const std::string &getSseCustomerAlgorithm() const {
    return sseCustomerAlgorithm_;
  }
  void setSseCustomerAlgorithm(const std::string &sseCustomerAlgorithm) {
    sseCustomerAlgorithm_ = sseCustomerAlgorithm;
  }
  const std::string &getSseCustomerKeyMd5() const { return sseCustomerKeyMD5_; }
  void setSseCustomerKeyMd5(const std::string &sseCustomerKeyMd5) {
    sseCustomerKeyMD5_ = sseCustomerKeyMd5;
  }
  const std::string &getCsType() const { return csType_; }
  void setCsType(const std::string &csType) { csType_ = csType; }

private:
  long contentLength_;
  std::string contentType_;
  std::string contentMD5_;
  std::string contentLanguage_;
  std::string contentEncoding_;
  std::string contentDisposition_;
  std::string lastModified_;
  std::string cacheControl_;
  std::string expires_;
  std::string etags_;
  std::string versionID_;
  bool deleteMarker_;
  // "" or "appendable"
  std::string objectType_;
  std::string storageClass_;
  std::string restore_;
  std::map<std::string, std::string> metadata_;
  std::string mirrorTag_;
  std::string sseCustomerAlgorithm_;
  std::string sseCustomerKeyMD5_;
  std::string csType_;
};
} // namespace VolcengineTos