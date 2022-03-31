#include <utility>
#include "auth/SignV4.h"
#include "model/object/MultipartUpload.h"
#include "transport/http/HttpClient.h"
#include "TosClientImpl.h"
#include "TosClient.h"
#include "transport/DefaultTransport.h"
#include "transport/TransportConfig.h"
#include "utils/MimeType.h"

using namespace VolcengineTos;

TosClientImpl::TosClientImpl(const std::string &endpoint, const std::string &region, const StaticCredentials &cred)
{
  TransportConfig conf;
  transport_ = std::make_shared<DefaultTransport>(conf);

  config_.setTransportConfig(conf);
  config_.setEndpoint(endpoint);
  config_.setRegion(region);

  // set scheme and host
  if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTPS)){
    scheme_ = http::SchemeHTTPS;
    host_ = endpoint.substr(std::strlen(http::SchemeHTTPS)+3, endpoint.length() - std::strlen(http::SchemeHTTPS)-3);
  } else if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTP)) {
    scheme_ = http::SchemeHTTP;
    host_ = endpoint.substr(std::strlen(http::SchemeHTTP)+3, endpoint.length() - std::strlen(http::SchemeHTTP)-3);
  } else {
    scheme_ = http::SchemeHTTP;
    host_ = endpoint;
  }
  credentials_ = std::make_shared<StaticCredentials>(cred);
  signer_ = std::make_shared<SignV4>(credentials_, region);

  if (userAgent_.empty()) userAgent_ = DefaultUserAgent();
  urlMode_ = URL_MODE_DEFAULT;
}

std::string isValidBucketName(const std::string& name) {
  if (name.empty() || name.length() < 3 || name.length() > 63) {
    return "tos: bucket name length must between [3, 64)";
  }
  for (char c : name) {
    if (!(('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-')){
      return "tos: bucket name can consist only of lowercase letters, numbers, and '-' ";
    }
  }
  if (name[0] == '-' || name[name.length()-1] == '-'){
    return "tos: bucket name must begin and end with a letter or number";
  }
  return "";
}

std::string isValidKey(const std::string& key) {
  if (key.empty()){
    return "tos: object name is empty";
  }
  if (key[0] == '/' || key[key.size()-1] == '/' || key.find("//") != -1) {
    return "tos: object name is illegal";
  }
  return "";
}

std::string isValidKeys(const std::vector<std::string>& keys) {
  for (auto & key : keys) {
    std::string ret = isValidKey(key);
    if (!ret.empty()) {
      return ret;
    }
  }
  return "";
}

std::string isValidNames(const std::string &bucket, const std::vector<std::string>& keys){
  std::string checkBkt = isValidBucketName(bucket);
  if (checkBkt.empty()){
    return isValidKeys(keys);
  } else{
    return checkBkt;
  }
}

void setContentType(RequestBuilder &rb, const std::string &objectKey) {
  std::string contentType = rb.findHeader(http::HEADER_CONTENT_TYPE);
  if (contentType.empty()){
    // request does not attach content-type
    // auto recognize default, check if user disable it by withAutoRecognizeContentType(false)
    if (rb.isAutoRecognizeContentType()){
      // set content type before upload
      contentType = MimeType::getMimetypeByObjectKey(objectKey);
      rb.withHeader(http::HEADER_CONTENT_TYPE, contentType);
    }
  }
}
static void createBucketSetOptionHeader(RequestBuilder & rb, const CreateBucketInput &input){
  if (!input.getAcl().empty()) rb.withHeader(HEADER_ACL, input.getAcl());
  if (!input.getGrantFullControl().empty()) rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
  if (!input.getGrantRead().empty()) rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
  if (!input.getGrantReadAcp().empty()) rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
  if (!input.getGrantWrite().empty()) rb.withHeader(HEADER_GRANT_WRITE, input.getGrantWrite());
  if (!input.getGrantWriteAcp().empty()) rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
}
Outcome<TosError, CreateBucketOutput>
TosClientImpl::createBucket(const CreateBucketInput &input) {
  Outcome<TosError, CreateBucketOutput> res;
  std::string check = isValidBucketName(input.getBucket());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }

  auto rb = newBuilder(input.getBucket(), "");
  createBucketSetOptionHeader(rb, input);
  auto req = rb.Build(http::MethodPut, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  CreateBucketOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setLocation(tosRes.result()->findHeader(http::HEADER_LOCATION));
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, HeadBucketOutput>
TosClientImpl::headBucket(const std::string &bucket) {
  Outcome<TosError, HeadBucketOutput> res;
  std::string check = isValidBucketName(bucket);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setSuccess(false);
    res.setE(error);
    return res;
  }
  auto req = newBuilder(bucket, "").Build(http::MethodHead, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  HeadBucketOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setRegion(tosRes.result()->findHeader(HEADER_BUCKET_REGION));
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, DeleteBucketOutput>
TosClientImpl::deleteBucket(const std::string &bucket) {
  Outcome<TosError, DeleteBucketOutput> res;
  std::string check = isValidBucketName(bucket);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setSuccess(false);
    res.setE(error);
    return res;
  }
  auto req = newBuilder(bucket, "").Build(http::MethodDelete);
  auto tosRes = roundTrip(req, 204);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  DeleteBucketOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, ListBucketsOutput>
TosClientImpl::listBuckets(const ListBucketsInput &input) {
  Outcome<TosError, ListBucketsOutput> res;
  auto req = newBuilder("", "").Build(http::MethodGet);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  ListBucketsOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());

  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  res.setSuccess(true);
  res.setR(output);
  return res;
}

Outcome<TosError, GetObjectOutput>
TosClientImpl::getObject(const std::string &bucket, const std::string &objectKey) {
  Outcome<TosError, GetObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  this->getObject(rb, res);
  return res;
}
Outcome<TosError, GetObjectOutput>
TosClientImpl::getObject(const std::string &bucket, const std::string &objectKey,
                         const RequestOptionBuilder &builder) {
  Outcome<TosError, GetObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->getObject(rb, res);
  return res;
}
Outcome<TosError, HeadObjectOutput>
TosClientImpl::headObject(const std::string &bucket, const std::string &objectKey) {
  Outcome<TosError, HeadObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  this->headObject(rb, res);
  return res;
}
Outcome<TosError, HeadObjectOutput>
TosClientImpl::headObject(const std::string &bucket, const std::string &objectKey,
                          const RequestOptionBuilder &builder) {
  Outcome<TosError, HeadObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->headObject(rb, res);
  return res;
}
Outcome<TosError, DeleteObjectOutput>
TosClientImpl::deleteObject(const std::string &bucket, const std::string &objectKey) {
  Outcome<TosError, DeleteObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  this->deleteObject(rb, res);
  return res;
}
Outcome<TosError, DeleteObjectOutput>
TosClientImpl::deleteObject(const std::string &bucket, const std::string &objectKey,
                            const RequestOptionBuilder &builder) {
  Outcome<TosError, DeleteObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->deleteObject(rb, res);
  return res;
}
Outcome<TosError, DeleteMultiObjectsOutput>
TosClientImpl::deleteMultiObjects(const std::string &bucket, DeleteMultiObjectsInput &input) {
  Outcome<TosError, DeleteMultiObjectsOutput> res;
  std::string data = input.toJsonString();
  std::string dataMd5 = CryptoUtils::md5Sum(data);
  auto rb = newBuilder(bucket, "");
  this->deleteMultiObjects(rb, data, dataMd5, res);
  return res;
}
Outcome<TosError, DeleteMultiObjectsOutput>
TosClientImpl::deleteMultiObjects(const std::string &bucket, DeleteMultiObjectsInput &input,
                                  const RequestOptionBuilder &builder) {
  Outcome<TosError, DeleteMultiObjectsOutput> res;
  std::string data = input.toJsonString();
  std::string dataMd5 = CryptoUtils::md5Sum(data);
  auto rb = newBuilder(bucket, "", builder);
  this->deleteMultiObjects(rb, data, dataMd5, res);
  return res;
}
Outcome<TosError, PutObjectOutput>
TosClientImpl::putObject(const std::string &bucket, const std::string &objectKey,
                         const std::shared_ptr<std::iostream> &content) {
  Outcome<TosError, PutObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  setContentType(rb, objectKey);
  auto req = rb.Build(http::MethodPut, content);
  this->putObject(req, res);
  return res;
}
Outcome<TosError, PutObjectOutput>
TosClientImpl::putObject(const std::string &bucket, const std::string &objectKey,
                         const std::shared_ptr<std::iostream> &content,
                         const RequestOptionBuilder &builder) {
  Outcome<TosError, PutObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  setContentType(rb, objectKey);
  auto req = rb.Build(http::MethodPut, content);
  this->putObject(req, res);
  return res;
}

Outcome<TosError, AppendObjectOutput>
TosClientImpl::appendObject(const std::string &bucket, const std::string &objectKey,
                            const std::shared_ptr<std::iostream> &content, int64_t offset) {
  Outcome<TosError, AppendObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  setContentType(rb, objectKey);
  rb.withQuery("append", "");
  rb.withQuery("offset", std::to_string(offset));
  auto req = rb.Build(http::MethodPost, content);
  this->appendObject(req, res);
  return res;
}
Outcome<TosError, AppendObjectOutput>
TosClientImpl::appendObject(const std::string &bucket, const std::string &objectKey,
                            const std::shared_ptr<std::iostream> &content, int64_t offset,
                            const RequestOptionBuilder &builder) {
  Outcome<TosError, AppendObjectOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  setContentType(rb, objectKey);
  rb.withQuery("append", "");
  rb.withQuery("offset", std::to_string(offset));
  auto req = rb.Build(http::MethodPost, content);
  this->appendObject(req, res);
  return res;
}

Outcome<TosError, SetObjectMetaOutput>
TosClientImpl::setObjectMeta(const std::string &bucket, const std::string &objectKey,
                             const RequestOptionBuilder &builder) {
  Outcome<TosError, SetObjectMetaOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->setObjectMeta(rb, res);
  return res;
}
Outcome<TosError, ListObjectsOutput>
TosClientImpl::listObjects(const std::string &bucket, const ListObjectsInput &input) {
  Outcome<TosError, ListObjectsOutput> res;
  auto rb = newBuilder(bucket, "");
  rb.withQuery("prefix", input.getPrefix());
  rb.withQuery("delimiter", input.getDelimiter());
  rb.withQuery("marker", input.getMarker());
  rb.withQuery("max-keys", std::to_string(input.getMaxKeys()));
  rb.withQuery("reverse", std::to_string(input.isReverse()));
  rb.withQuery("encoding-type", input.getEncodingType());
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  ListObjectsOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, ListObjectVersionsOutput>
TosClientImpl::listObjectVersions(const std::string &bucket,
                                  const ListObjectVersionsInput &input) {
  Outcome<TosError, ListObjectVersionsOutput> res;
  auto rb = newBuilder(bucket, "");
  rb.withQuery("prefix", input.getPrefix());
  rb.withQuery("delimiter", input.getDelimiter());
  rb.withQuery("key-marker", input.getKeyMarker());
  rb.withQuery("max-keys", std::to_string(input.getMaxKeys()));
  rb.withQuery("encoding-type", input.getEncodingType());
  rb.withQuery("versions", "");
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  ListObjectVersionsOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObject(const std::string &bucket,
                          const std::string &srcObjectKey, const std::string &dstObjectKey) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
  std::string check = isValidKeys(keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(bucket, dstObjectKey, bucket, srcObjectKey, res);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObject(const std::string &bucket,
                          const std::string &srcObjectKey, const std::string &dstObjectKey,
                          const RequestOptionBuilder &builder) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
  std::string check = isValidKeys(keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(bucket, dstObjectKey, bucket, srcObjectKey, builder, res);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObjectTo(const std::string &bucket, const std::string &dstBucket,
                            const std::string &dstObjectKey, const std::string &srcObjectKey) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
  std::string check = isValidNames(dstBucket, keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(dstBucket, dstObjectKey, bucket, srcObjectKey, res);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObjectTo(const std::string &bucket, const std::string &dstBucket,
                            const std::string &dstObjectKey, const std::string &srcObjectKey,
                            const RequestOptionBuilder &builder) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
  std::string check = isValidNames(dstBucket, keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(dstBucket, dstObjectKey, bucket, srcObjectKey, builder, res);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObjectFrom(const std::string &bucket, const std::string &srcBucket,
                              const std::string &srcObjectKey, const std::string &dstObjectKey) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {srcObjectKey, dstObjectKey};
  std::string check = isValidNames(srcBucket, keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(bucket, dstObjectKey, srcBucket, srcObjectKey, res);
  return res;
}
Outcome<TosError, CopyObjectOutput>
TosClientImpl::copyObjectFrom(const std::string &bucket, const std::string &srcBucket,
                              const std::string &srcObjectKey, const std::string &dstObjectKey,
                              const RequestOptionBuilder &builder) {
  Outcome<TosError, CopyObjectOutput> res;
  std::vector<std::string> keys = {srcObjectKey, dstObjectKey};
  std::string check = isValidNames(srcBucket, keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  this->copyObject(bucket, dstObjectKey, srcBucket, srcObjectKey, builder, res);
  return res;
}
static std::string copyRange(long startOffset, long partSize) {
  std::string cr;
  if (startOffset == 0){
    if (partSize != 0) {
      cr += "bytes=";
      cr += std::to_string(startOffset);
      cr += "-";
      cr += std::to_string(startOffset+partSize-1);
    } else {
      cr += "bytes=";
      cr += std::to_string(startOffset);
      cr += "-";
    }
  } else if (partSize != 0){
    cr += "bytes=0-";
    cr += std::to_string(partSize-1);
  }
  return cr;
}
Outcome<TosError, UploadPartCopyOutput>
TosClientImpl::uploadPartCopy(const std::string &bucket, const UploadPartCopyInput &input) {
  Outcome<TosError, UploadPartCopyOutput> res;
  std::vector<std::string> keys = {input.getDestinationKey()};
  std::string check = isValidNames(input.getSourceBucket(), keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getDestinationKey());
  this->uploadPartCopy(rb, input, res);
  return res;
}
Outcome<TosError, UploadPartCopyOutput>
TosClientImpl::uploadPartCopy(const std::string &bucket, const UploadPartCopyInput &input,
                              const RequestOptionBuilder &builder) {
  Outcome<TosError, UploadPartCopyOutput> res;
  std::vector<std::string> keys = {input.getDestinationKey()};
  std::string check = isValidNames(input.getSourceBucket(), keys);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getDestinationKey(), builder);
  this->uploadPartCopy(rb, input, res);
  return res;
}

void setAclGrant(const PutObjectAclInput &input, RequestBuilder & rb) {
  const auto& grant = input.getAclGrant();
  if (!grant.getAcl().empty()) rb.withHeader(HEADER_ACL, grant.getAcl());
  if (!grant.getGrantFullControl().empty()) rb.withHeader(HEADER_GRANT_FULL_CONTROL, grant.getGrantFullControl());
  if (!grant.getGrantRead().empty()) rb.withHeader(HEADER_GRANT_READ, grant.getGrantRead());
  if (!grant.getGrantReadAcp().empty()) rb.withHeader(HEADER_GRANT_READ_ACP, grant.getGrantReadAcp());
  if (!grant.getGrantWrite().empty()) rb.withHeader(HEADER_GRANT_WRITE, grant.getGrantWrite());
  if (!grant.getGrantWriteAcp().empty()) rb.withHeader(HEADER_GRANT_WRITE_ACP, grant.getGrantWriteAcp());
}
Outcome<TosError, PutObjectAclOutput>
TosClientImpl::putObjectAcl(const std::string &bucket, const PutObjectAclInput &input) {
  Outcome<TosError, PutObjectAclOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rules = input.getAclRules();
  std::shared_ptr<std::stringstream> ss = nullptr;
  std::string jsonRules(rules.toJsonString());
  if (jsonRules != "null") {
    ss = std::make_shared<std::stringstream>(jsonRules);
    std::cout << ss->str() << std::endl;
  }
  auto rb = newBuilder(bucket, input.getKey());
  rb.withQuery("acl", "");
  setAclGrant(input, rb);
  auto req = rb.Build(http::MethodPut, ss);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  PutObjectAclOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, GetObjectAclOutput>
TosClientImpl::getObjectAcl(const std::string &bucket, const std::string &objectKey) {
  Outcome<TosError, GetObjectAclOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  this->getObjectAcl(rb, res);
  return res;
}
Outcome<TosError, GetObjectAclOutput>
TosClientImpl::getObjectAcl(const std::string &bucket, const std::string &objectKey,
                            const RequestOptionBuilder &builder) {
  Outcome<TosError, GetObjectAclOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->getObjectAcl(rb, res);
  return res;
}

Outcome<TosError, CreateMultipartUploadOutput>
TosClientImpl::createMultipartUpload(const std::string &bucket, const std::string &objectKey) {
  Outcome<TosError, CreateMultipartUploadOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  setContentType(rb, objectKey);
  this->createMultipartUpload(rb, res);
  return res;
}
Outcome<TosError, CreateMultipartUploadOutput>
TosClientImpl::createMultipartUpload(const std::string &bucket, const std::string &objectKey,
                                     const RequestOptionBuilder &builder) {
  Outcome<TosError, CreateMultipartUploadOutput> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  setContentType(rb, objectKey);
  this->createMultipartUpload(rb, res);
  return res;
}
Outcome<TosError, UploadPartOutput>
TosClientImpl::uploadPart(const std::string &bucket, const UploadPartInput &input) {
  Outcome<TosError, UploadPartOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getKey());
  this->uploadPart(rb, input, res);
  return res;
}
Outcome<TosError, UploadPartOutput>
TosClientImpl::uploadPart(const std::string &bucket, const UploadPartInput &input,
                          const RequestOptionBuilder &builder) {
  Outcome<TosError, UploadPartOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getKey(), builder);
  this->uploadPart(rb, input, res);
  return res;
}
Outcome<TosError, CompleteMultipartUploadOutput>
TosClientImpl::completeMultipartUpload(const std::string &bucket, CompleteMultipartUploadInput &input) {
  Outcome<TosError, CompleteMultipartUploadOutput> res;
  int partsNum = input.getUploadedPartsLength();
  inner::InnerCompleteMultipartUploadInput multipart(partsNum);
  for (int i = 0; i < partsNum; ++i) {
    inner::InnerUploadedPart part(input.getUploadedParts()[i].getPartNumber(), input.getUploadedParts()[i].getEtag());
    multipart.setPartsByIdx(part, i);
  }
  multipart.sort();
  auto content = std::make_shared<std::stringstream>(multipart.toJsonString());
  auto rb = newBuilder(bucket, input.getKey());
  rb.withQuery("uploadId", input.getUploadId());
  auto req = rb.Build(http::MethodPost, content);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  CompleteMultipartUploadOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, CompleteMultipartUploadOutput>
TosClientImpl::completeMultipartUpload(const std::string &bucket, CompleteMultipartCopyUploadInput &input) {
  Outcome<TosError, CompleteMultipartUploadOutput> res;
  int partsNum = input.getUploadedPartsLength();
  inner::InnerCompleteMultipartUploadInput multipart(partsNum);
  for (int i = 0; i < partsNum; ++i) {
    inner::InnerUploadedPart part(input.getUploadedParts()[i].getPartNumber(), input.getUploadedParts()[i].getEtag());
    multipart.setPartsByIdx(part, i);
  }
  multipart.sort();
  auto content = std::make_shared<std::stringstream>(multipart.toJsonString());
  auto rb = newBuilder(bucket, input.getKey());
  rb.withQuery("uploadId", input.getUploadId());
  auto req = rb.Build(http::MethodPost, content);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  CompleteMultipartUploadOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, AbortMultipartUploadOutput>
TosClientImpl::abortMultipartUpload(const std::string &bucket, const AbortMultipartUploadInput &input) {
  Outcome<TosError, AbortMultipartUploadOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getKey());
  rb.withQuery("uploadId", input.getUploadId());
  auto req = rb.Build(http::MethodDelete, nullptr);
  auto tosRes = roundTrip(req, 204);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  AbortMultipartUploadOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}
Outcome<TosError, ListUploadedPartsOutput>
TosClientImpl::listUploadedParts(const std::string &bucket, const ListUploadedPartsInput &input) {
  Outcome<TosError, ListUploadedPartsOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getKey());
  this->listUploadedParts(rb, input.getUploadId(), res);
  return res;
}
Outcome<TosError, ListUploadedPartsOutput>
TosClientImpl::listUploadedParts(const std::string &bucket, const ListUploadedPartsInput &input,
                                 const RequestOptionBuilder &builder) {
  Outcome<TosError, ListUploadedPartsOutput> res;
  std::string check = isValidKey(input.getKey());
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, input.getKey(), builder);
  this->listUploadedParts(rb, input.getUploadId(), res);
  return res;
}
Outcome<TosError, ListMultipartUploadsOutput>
TosClientImpl::listMultipartUploads(const std::string &bucket, const ListMultipartUploadsInput &input) {
  Outcome<TosError, ListMultipartUploadsOutput> res;
  auto rb = newBuilder(bucket, "");
  rb.withQuery("uploads", "");
  rb.withQuery("prefix", input.getPrefix());
  rb.withQuery("delimiter", input.getDelimiter());
  rb.withQuery("key-marker", input.getKeyMarker());
  rb.withQuery("upload-id-marker", input.getUploadIdMarker());
  rb.withQuery("max-uploads", std::to_string(input.getMaxUploads()));
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return res;
  }
  ListMultipartUploadsOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
  return res;
}

Outcome<TosError, std::string>
TosClientImpl::preSignedURL(const std::string &httpMethod, const std::string &bucket,
                            const std::string &objectKey, std::chrono::duration<int> ttl) {
  Outcome<TosError, std::string> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey);
  this->preSignedURL(rb, httpMethod, ttl, res);
  return res;
}
Outcome<TosError, std::string> TosClientImpl::preSignedURL(const std::string &httpMethod, const std::string &bucket,
                                                           const std::string &objectKey, std::chrono::duration<int> ttl,
                                                           const RequestOptionBuilder &builder) {
  Outcome<TosError, std::string> res;
  std::string check = isValidKey(objectKey);
  if (!check.empty()) {
    TosError error;
    error.setMessage(check);
    res.setE(error);
    res.setSuccess(false);
    return res;
  }
  auto rb = newBuilder(bucket, objectKey, builder);
  this->preSignedURL(rb, httpMethod, ttl, res);
  return res;
}

Outcome<TosError, std::shared_ptr<TosResponse>> TosClientImpl::roundTrip(const std::shared_ptr<TosRequest> &request, int expectedCode) {
  auto resp = transport_->roundTrip(request);
  Outcome<TosError, std::shared_ptr<TosResponse>> ret;
  // check error
  if (resp->getStatusCode() == expectedCode){
    ret.setR(resp);
    ret.setSuccess(true);
    return ret;
  }
  ret.setSuccess(false);
  TosError se;
  if (resp->getStatusCode() >= 400) {
    if(resp->getContent()){
      std::stringstream ss;
      ss << resp->getContent()->rdbuf();
      std::string error = ss.str();
      se.fromJsonString(error);
      if (se.getRequestId().empty() && se.getMessage().empty() && se.getCode().empty() && se.getHostId().empty()) {
        // json parse error
        se.setMessage(error);
        se.setCode("json parse error");
      }
      se.setStatusCode(resp->getStatusCode());
      ret.setE(se);
      return ret;
    }
    // 特别处理 404
    if (resp->getStatusCode() == 404) {
      se.setCode("not found");
      se.setStatusCode(resp->getStatusCode());
      se.setRequestId(resp->getRequestID());
      ret.setE(se);
      return ret;
    }
  }
  se.setStatusCode(resp->getStatusCode());
  se.setCode("UnexpectedStatusCode error");
  se.setMessage(resp->getStatusMsg());
  se.setRequestId(resp->getRequestID());
  ret.setE(se);
  return ret;
}

RequestBuilder TosClientImpl::newBuilder(const std::string &bucket, const std::string &object) {
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> queries;
  auto rb = RequestBuilder(signer_, scheme_, host_, bucket, object, urlMode_, headers, queries);
  rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
  return rb;
}
RequestBuilder TosClientImpl::newBuilder(const std::string &bucket, const std::string &object, const RequestOptionBuilder &builder) {
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> queries;
  auto rb = RequestBuilder(signer_, scheme_, host_, bucket, object, urlMode_, headers, queries);
  rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
  auto headerIter = builder.getHeaders().begin();
  for (; headerIter != builder.getHeaders().end() ; ++headerIter) {
    rb.withHeader(headerIter->first, headerIter->second);
  }
  auto queryIter = builder.getQueries().begin();
  for (; queryIter != builder.getQueries().end() ; ++queryIter) {
    rb.withQuery(queryIter->first, queryIter->second);
  }
  rb.setContentLength(builder.getContentLength());
  rb.setAutoRecognizeContentType(builder.isAutoRecognizeContentType());
  rb.setRange(builder.getRange());
  return rb;
}

int expectedCode(const RequestBuilder& rb) {
  return rb.getRange().isNull() ? 200 : 206;
}

void TosClientImpl::getObject(RequestBuilder &rb, Outcome<TosError, GetObjectOutput> &res) {
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, expectedCode(rb));
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  GetObjectOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setContentRange(rb.findHeader(http::HEADER_CONTENT_RANGE));
  output.setContent(tosRes.result()->getContent());
  output.setObjectMetaFromResponse(*tosRes.result());
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::headObject(RequestBuilder &rb, Outcome<TosError, HeadObjectOutput> &res) {
  auto req = rb.Build(http::MethodHead, nullptr);
  auto tosRes = roundTrip(req, expectedCode(rb));
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  HeadObjectOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setContentRange(rb.findHeader(http::HEADER_CONTENT_RANGE));
  output.setObjectMeta(*(tosRes.result()));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::deleteObject(RequestBuilder &rb,Outcome<TosError, DeleteObjectOutput> &res) {
  auto req = rb.Build(http::MethodDelete, nullptr);
  auto tosRes = roundTrip(req, 204);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  DeleteObjectOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setDeleteMarker(tosRes.result()->findHeader(HEADER_DELETE_MARKER) == "true");
  output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::deleteMultiObjects(RequestBuilder &rb, const std::string &data, const std::string & dataMd5, Outcome<TosError, DeleteMultiObjectsOutput> &res) {
  rb.withHeader(http::HEADER_CONTENT_MD5, dataMd5);
  rb.withQuery("delete", "");
  std::cout << data << std::endl;
  std::cout << dataMd5 << std::endl;
  auto req = rb.Build(http::MethodPost, std::make_shared<std::stringstream>(data));
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  std::stringstream out;
  out << tosRes.result()->getContent()->rdbuf();
  DeleteMultiObjectsOutput output;
  output.fromJsonString(out.str());
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::putObject(const std::shared_ptr<TosRequest> &req, Outcome<TosError, PutObjectOutput> &res) {
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  PutObjectOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
  output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
  output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
  output.setSseCustomerKeyMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::appendObject(const std::shared_ptr<TosRequest>& req, Outcome<TosError, AppendObjectOutput> &res) {
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  auto nextOffset = tosRes.result()->findHeader(HEADER_NEXT_APPEND_OFFSET);
  AppendObjectOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
  output.setNextAppendOffset(stoi(nextOffset));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::setObjectMeta(RequestBuilder &rb, Outcome<TosError, SetObjectMetaOutput> &res) {
  rb.withQuery("metadata", "");
  auto req = rb.Build(http::MethodPost, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  SetObjectMetaOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::copyObject(const std::string &dstBucket, const std::string &dstObject,
                               const std::string &srcBucket, const std::string &srcObject,
                               Outcome<TosError, CopyObjectOutput> &outcome) {
  auto rb = newBuilder(dstBucket, dstObject);
  auto req = rb.BuildWithCopySource(http::MethodPut, srcBucket, srcObject);
  this->copyObject(req, outcome);
}
void TosClientImpl::copyObject(const std::string &dstBucket, const std::string &dstObject,
                               const std::string &srcBucket, const std::string &srcObject,
                               const RequestOptionBuilder &builder, Outcome<TosError, CopyObjectOutput> &outcome) {
  auto rb = newBuilder(dstBucket, dstObject, builder);
  auto req = rb.BuildWithCopySource(http::MethodPut, srcBucket, srcObject);
  this->copyObject(req, outcome);
}
void TosClientImpl::copyObject(std::shared_ptr<TosRequest> &req, Outcome<TosError, CopyObjectOutput> &res) {
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  CopyObjectOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
}

void TosClientImpl::uploadPartCopy(RequestBuilder& rb, const UploadPartCopyInput& input,
                                   Outcome<TosError, UploadPartCopyOutput> &res) {
  rb.withQuery("partNumber", std::to_string(input.getPartNumber()));
  rb.withQuery("uploadId", input.getUploadId());
  rb.withQuery("versionId", input.getSourceVersionId());
  rb.withHeader(HEADER_COPY_SOURCE_RANGE, copyRange(input.getStartOffset(), input.getPartSize()));
  auto req = rb.BuildWithCopySource
             (http::MethodPut, input.getSourceBucket(), input.getSourceKey());
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  inner::InnerUploadPartCopyOutput out;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  out.fromJsonString(ss.str());
  UploadPartCopyOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
  output.setSourceVersionId(tosRes.result()->findHeader(HEADER_COPY_SOURCE_VERSION_ID));
  output.setPartNumber(input.getPartNumber());
  output.setEtag(out.getEtag());
  output.setLastModified(out.getLastModified());
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::getObjectAcl(RequestBuilder &rb, Outcome<TosError, GetObjectAclOutput> &res) {
  rb.withQuery("acl", "");
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  GetObjectAclOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::createMultipartUpload(RequestBuilder &rb, Outcome<TosError, CreateMultipartUploadOutput> &res) {
  rb.withQuery("uploads", "");
  auto req = rb.Build(http::MethodPost, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  inner::MultipartUpload upload;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  upload.fromJsonString(ss.str());
  CreateMultipartUploadOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setBucket(upload.getBucket());
  output.setKey(upload.getKey());
  output.setUploadId(upload.getUploadId());
  output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
  output.setSseCustomerMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::uploadPart(RequestBuilder &rb, const UploadPartInput &input,
                               Outcome<TosError, UploadPartOutput> &res) {
  if ((int)input.getPartSize() != input.getPartSize()){
    TosError se;
    std::string errMsg = "UploadPart method has overhead size: ";
    errMsg += std::to_string(input.getPartSize());
    se.setMessage(errMsg);
    res.setE(se);
    res.setSuccess(false);
    return;
  }
  rb.withQuery("uploadId", input.getUploadId());
  rb.withQuery("partNumber", std::to_string(input.getPartNumber()));
  auto req = rb.Build(http::MethodPut, input.getContent());
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  UploadPartOutput output;
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  output.setPartNumber(input.getPartNumber());
  output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
  output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
  output.setSseCustomerMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
  res.setSuccess(true);
  res.setR(output);
}
void TosClientImpl::listUploadedParts(RequestBuilder &rb, const std::string& uploadId, Outcome<TosError, ListUploadedPartsOutput> &res) {
  rb.withQuery("uploadId", uploadId);
  auto req = rb.Build(http::MethodGet, nullptr);
  auto tosRes = roundTrip(req, 200);
  if (!tosRes.isSuccess()){
    res.setE(tosRes.error());
    res.setSuccess(false);
    return;
  }
  ListUploadedPartsOutput output;
  std::stringstream ss;
  ss << tosRes.result()->getContent()->rdbuf();
  output.fromJsonString(ss.str());
  output.setRequestInfo(tosRes.result()->GetRequestInfo());
  res.setSuccess(true);
  res.setR(output);
}

void TosClientImpl::preSignedURL(RequestBuilder &rb, const std::string& method,
                                 const std::chrono::duration<int>& ttl,
                                 Outcome<TosError, std::string> &res) {
  auto req = rb.Build(method, nullptr);
  auto query = signer_->signQuery(req, ttl);
  for (auto & iter : query) {
    req->setSingleQuery(iter.first, iter.second);
  }
  auto url = req->toUrl().toString();
  res.setR(url);
  res.setSuccess(true);
}