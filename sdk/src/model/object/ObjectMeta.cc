#include "model/object/ObjectMeta.h"
#include <cstring>
std::map<std::string, std::string> userMetadata(const std::map<std::string, std::string>& headers){
  std::map<std::string, std::string> meta;
  for (const auto & header : headers) {
    if (VolcengineTos::StringUtils::startsWithIgnoreCase(header.first, VolcengineTos::HEADER_META_PREFIX)){
      auto kk = header.first.substr(std::strlen(VolcengineTos::HEADER_META_PREFIX), header.first.size());
      meta[kk] = header.second;
    }
  }
  return meta;
}
void VolcengineTos::ObjectMeta::fromResponse(TosResponse& res) {
  contentLength_ = res.getContentLength();
  contentType_ = res.findHeader(http::HEADER_CONTENT_TYPE);
  contentMD5_ = res.findHeader(http::HEADER_CONTENT_MD5);
  contentLanguage_ = res.findHeader(http::HEADER_CONTENT_LANGUAGE);
  contentEncoding_ = res.findHeader(http::HEADER_CONTENT_ENCODING);
  contentDisposition_ = res.findHeader(http::HEADER_CONTENT_DISPOSITION);
  lastModified_ = res.findHeader(http::HEADER_LAST_MODIFIED);
  cacheControl_ = res.findHeader(http::HEADER_CACHE_CONTROL);
  expires_ = res.findHeader(http::HEADER_EXPIRES);
  etags_ = res.findHeader(http::HEADER_ETAG);
  versionID_ = res.findHeader(HEADER_VERSIONID);
  deleteMarker_ = res.findHeader(HEADER_DELETE_MARKER) == "true";
  objectType_ = res.findHeader(HEADER_OBJECT_TYPE);
  storageClass_ = res.findHeader(HEADER_STORAGE_CLASS);
  restore_ = res.findHeader(HEADER_RESTORE);
  metadata_ = userMetadata(res.getHeaders());
  mirrorTag_ = res.findHeader(HEADER_MIRROR_TAG);
  sseCustomerAlgorithm_ = res.findHeader(HEADER_SSE_CUSTOMER_ALGORITHM);
  sseCustomerKeyMD5_ = res.findHeader(HEADER_SSE_CUSTOMER_KEY_MD5);
  csType_ = res.findHeader(HEADER_CS_TYPE);
}
bool VolcengineTos::ObjectMeta::operator==(
    const VolcengineTos::ObjectMeta &rhs) const {
  return contentLength_ == rhs.contentLength_ &&
         contentType_ == rhs.contentType_ && contentMD5_ == rhs.contentMD5_ &&
         contentLanguage_ == rhs.contentLanguage_ &&
         contentEncoding_ == rhs.contentEncoding_ &&
         contentDisposition_ == rhs.contentDisposition_ &&
         lastModified_ == rhs.lastModified_ &&
         cacheControl_ == rhs.cacheControl_ && expires_ == rhs.expires_ &&
         etags_ == rhs.etags_ && versionID_ == rhs.versionID_ &&
         deleteMarker_ == rhs.deleteMarker_ && objectType_ == rhs.objectType_ &&
         storageClass_ == rhs.storageClass_ && restore_ == rhs.restore_ &&
         metadata_ == rhs.metadata_ && mirrorTag_ == rhs.mirrorTag_ &&
         sseCustomerAlgorithm_ == rhs.sseCustomerAlgorithm_ &&
         sseCustomerKeyMD5_ == rhs.sseCustomerKeyMD5_ && csType_ == rhs.csType_;
}
bool VolcengineTos::ObjectMeta::operator!=(
    const VolcengineTos::ObjectMeta &rhs) const {
  return !(rhs == *this);
}
