#pragma once

#include <map>
#include "common/Common.h"
#include "RequestBuilder.h"
namespace VolcengineTos {
class RequestOptionBuilder {
public:
  RequestOptionBuilder() = default;
  ~RequestOptionBuilder() = default;
  const std::map<std::string, std::string> &getHeaders() const { return headers_; }
  const std::map<std::string, std::string> &getQueries() const { return queries_; }
  int64_t getContentLength() const { return contentLength_; }
  bool isAutoRecognizeContentType() const { return autoRecognizeContentType_; }
  const HttpRange &getRange() const { return range_; }

  void withContentLength(int64_t length){
    contentLength_ = length;
  }
  void withContentType(const std::string& contentType){
    headers_[http::HEADER_CONTENT_TYPE] = contentType;
  }
  void withAutoRecognizeContentType(bool autoRecognizeContentType){
    autoRecognizeContentType_ = autoRecognizeContentType;
  }
  void withCacheControl(const std::string &cacheControl) {
    headers_[http::HEADER_CACHE_CONTROL] = cacheControl;
  }
  void withContentDisposition(const std::string &contentDisposition) {
    headers_[http::HEADER_CONTENT_DISPOSITION] = contentDisposition;
  }
  void withContentEncoding(const std::string &contentEncoding) {
    headers_[http::HEADER_CONTENT_ENCODING] = contentEncoding;
  }
  void withContentLanguage(const std::string &contentLanguage) {
    headers_[http::HEADER_CONTENT_LANGUAGE] = contentLanguage;
  }
  void withContentMD5(const std::string &contentMD5) {
    headers_[http::HEADER_CONTENT_MD5] = contentMD5;
  }
  void withContentSHA256(const std::string &contentSHA256) {
    headers_[VolcengineTos::HEADER_CONTENT_SHA256] = contentSHA256;
  }
  void withExpires(const std::time_t &expires) {
    headers_[http::HEADER_EXPIRES] = std::to_string(expires);
  }
  void withServerSideEncryptionCustomer(const std::string &ssecAlgorithm, const std::string &ssecKey, const std::string &ssecKeyMD5) {
    headers_[VolcengineTos::HEADER_SSE_CUSTOMER_ALGORITHM] = ssecAlgorithm;
    headers_[VolcengineTos::HEADER_SSE_CUSTOMER_KEY] = ssecKey;
    headers_[VolcengineTos::HEADER_SSE_CUSTOMER_KEY_MD5] = ssecKeyMD5;
  }
  void withServerSideEncryption(const std::string &serverSideEncryption) {
    headers_[VolcengineTos::HEADER_SSE] = serverSideEncryption;
  }
  void withStorageClass(const std::string &storageClass) {
    headers_[VolcengineTos::HEADER_STORAGE_CLASS] = storageClass;
  }
  void withIfModifiedSince(const std::time_t &since) {
    headers_[http::HEADER_IF_MODIFIED_SINCE] = std::to_string(since);
  }
  void withIfUnmodifiedSince(const std::time_t &since) {
    headers_[http::HEADER_IF_UNMODIFIED_SINCE] = std::to_string(since);
  }
  void withIfMatch(const std::string &ifMatch) {
    headers_[http::HEADER_IF_MATCH] = ifMatch;
  }
  void withIfNoneMatch(const std::string &ifNoneMatch) {
    headers_[http::HEADER_IF_NONE_MATCH] = ifNoneMatch;
  }
  void withCopySourceIfMatch(const std::string &ifMatch) {
    headers_[VolcengineTos::HEADER_COPY_SOURCE_IF_MATCH] = ifMatch;
  }
  void withCopySourceIfNoneMatch(const std::string &ifNoneMatch) {
    headers_[VolcengineTos::HEADER_COPY_SOURCE_IF_NONE_MATCH] = ifNoneMatch;
  }
  void withCopySourceIfModifiedSince(const std::string &ifModifiedSince) {
    headers_[VolcengineTos::HEADER_COPY_SOURCE_IF_MODIFIED_SINCE] = ifModifiedSince;
  }
  void withCopySourceIfUnmodifiedSince(const std::string &ifUnmodifiedSince) {
    headers_[VolcengineTos::HEADER_COPY_SOURCE_IF_UNMODIFIED_SINCE] = ifUnmodifiedSince;
  }
  void withMeta(const std::string &key, const std::string &value) {
    headers_[VolcengineTos::HEADER_META_PREFIX+key] = value;
  }

  void withRange(int64_t start, int64_t end) {
    range_.setStart(start);
    range_.setEnd(end);
    headers_[http::HEADER_RANGE] = range_.toString();
  }
  void withVersionID(const std::string &versionID) {
    headers_["versionId"] = versionID;
  }
  void withMetadataDirective(const std::string &directive) {
    headers_[VolcengineTos::HEADER_METADATA_DIRECTIVE] = directive;
  }
  void withACL(const std::string &acl) {
    headers_[VolcengineTos::HEADER_ACL] = acl;
  }
  void withACLGrantFullControl(const std::string &grantFullControl) {
    headers_[VolcengineTos::HEADER_GRANT_FULL_CONTROL] = grantFullControl;
  }
  void withACLGrantRead(const std::string &grantRead) {
    headers_[VolcengineTos::HEADER_GRANT_READ] = grantRead;
  }
  void withACLGrantReadAcp(const std::string &grantReadAcp) {
    headers_[VolcengineTos::HEADER_GRANT_READ_ACP] = grantReadAcp;
  }
  void withACLGrantWrite(const std::string &grantWrite) {
    headers_[VolcengineTos::HEADER_GRANT_WRITE] = grantWrite;
  }
  void withACLGrantWriteAcp(const std::string &grantWriteAcp) {
    headers_[VolcengineTos::HEADER_GRANT_WRITE_ACP] = grantWriteAcp;
  }
  void withWebsiteRedirectLocation(const std::string &redirectLocation) {
    headers_[VolcengineTos::HEADER_WEBSITE_REDIRECT_LOCATION] = redirectLocation;
  }

  void withHeader(const std::string & key, const std::string & value) {
    headers_[key] = value;
  }
  void withQuery(const std::string & key, const std::string & value) {
    queries_[key] = value;
  }
private:
  std::map<std::string, std::string> headers_;
  std::map<std::string, std::string> queries_;
  int64_t contentLength_{};
  bool autoRecognizeContentType_{};
  HttpRange range_;
};
} // namespace VolcengineTos