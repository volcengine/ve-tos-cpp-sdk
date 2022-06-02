#include <utility>
#include <memory>
#include <curl/curl.h>
#include "TosClient.h"
#include "TosClientImpl.h"
#include "transport/http/HttpClient.h"

using namespace VolcengineTos;

void VolcengineTos::InitializeClient() {
  if (!hasInitHttpClient) {
    curl_global_init(CURL_GLOBAL_ALL);
    hasInitHttpClient = true;
  }
}

void VolcengineTos::CloseClient() {
  curl_global_cleanup();
  hasInitHttpClient = false;
}

TosClient::TosClient(const std::string &endpoint, const std::string &region,
                     const std::string &accessKeyId, const std::string &secretKeyId) :
  TosClient(endpoint, region, StaticCredentials(accessKeyId, secretKeyId))
{
}

TosClient::TosClient(const std::string &endpoint, const std::string &region,
                     const std::string &accessKeyId, const std::string &secretKeyId, const std::string &securityToken) :
  TosClient(endpoint, region, StaticCredentials(accessKeyId, secretKeyId, securityToken))
{
}
TosClient::TosClient(const std::string &endpoint, const std::string &region, const StaticCredentials &cred) :
  tosClientImpl_(std::make_shared<TosClientImpl>(endpoint, region, cred))
{
}

TosClient::TosClient(const std::string &endpoint, const std::string &region, const FederationCredentials &cred) :
  tosClientImpl_(std::make_shared<TosClientImpl>(endpoint, region, cred))
{
}

TosClient::~TosClient() = default;

Outcome<TosError, CreateBucketOutput> TosClient::createBucket(const CreateBucketInput &input) {
  return tosClientImpl_->createBucket(input);
}
Outcome<TosError, HeadBucketOutput> TosClient::headBucket(const std::string &bucket) {
  return tosClientImpl_->headBucket(bucket);
}
Outcome<TosError, DeleteBucketOutput> TosClient::deleteBucket(const std::string &bucket) {
  return tosClientImpl_->deleteBucket(bucket);
}
Outcome<TosError, ListBucketsOutput> TosClient::listBuckets(const ListBucketsInput &input) {
  return tosClientImpl_->listBuckets(input);
}


Outcome<TosError, PutObjectOutput> TosClient::putObject(const std::string &bucket, const std::string &objectKey,
                                                        const std::shared_ptr<std::iostream> &content) {
  return tosClientImpl_->putObject(bucket, objectKey, content);
}
Outcome<TosError, PutObjectOutput> TosClient::putObject(const std::string &bucket, const std::string &objectKey,
                                                        const std::shared_ptr<std::iostream> &content, const RequestOptionBuilder &builder) {
  return tosClientImpl_->putObject(bucket, objectKey, content, builder);
}

Outcome<TosError, UploadFileOutput>
TosClient::uploadFile(const std::string &bucket, const UploadFileInput &input) {
  return tosClientImpl_->uploadFile(bucket, input, {});
}
Outcome<TosError, UploadFileOutput>
TosClient::uploadFile(const std::string &bucket, const UploadFileInput &input,
                      const RequestOptionBuilder &builder) {
  return tosClientImpl_->uploadFile(bucket, input, builder);
}
Outcome<TosError, GetObjectOutput> TosClient::getObject(const std::string &bucket, const std::string &objectKey) {
  return tosClientImpl_->getObject(bucket, objectKey);
}
Outcome<TosError, GetObjectOutput> TosClient::getObject(const std::string &bucket, const std::string &objectKey,
                                                        const RequestOptionBuilder &builder) {
  return tosClientImpl_->getObject(bucket, objectKey, builder);
}
Outcome<TosError, HeadObjectOutput> TosClient::headObject(const std::string &bucket, const std::string &objectKey) {
  return tosClientImpl_->headObject(bucket, objectKey);
}
Outcome<TosError, HeadObjectOutput> TosClient::headObject(const std::string &bucket, const std::string &objectKey,
                                                          const RequestOptionBuilder &builder) {
  return tosClientImpl_->headObject(bucket, objectKey, builder);
}
Outcome<TosError, DeleteObjectOutput> TosClient::deleteObject(const std::string &bucket, const std::string &objectKey) {
  return tosClientImpl_->deleteObject(bucket, objectKey);
}
Outcome<TosError, DeleteObjectOutput> TosClient::deleteObject(const std::string &bucket, const std::string &objectKey,
                                                              const RequestOptionBuilder &builder) {
  return tosClientImpl_->deleteObject(bucket, objectKey, builder);
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClient::deleteMultiObjects(const std::string &bucket, DeleteMultiObjectsInput &input) {
  return tosClientImpl_->deleteMultiObjects(bucket, input);
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClient::deleteMultiObjects(const std::string &bucket, DeleteMultiObjectsInput &input,
                                                                          const RequestOptionBuilder &builder) {
  return tosClientImpl_->deleteMultiObjects(bucket, input, builder);
}


Outcome<TosError, AppendObjectOutput> TosClient::appendObject(const std::string &bucket, const std::string &objectKey,
                                                              const std::shared_ptr<std::iostream> &content, int64_t offset) {
  return tosClientImpl_->appendObject(bucket, objectKey, content, offset);
}
Outcome<TosError, AppendObjectOutput> TosClient::appendObject(const std::string &bucket, const std::string &objectKey,
                                                              const std::shared_ptr<std::iostream> &content, int64_t offset,
                                                              const RequestOptionBuilder &builder) {
  return tosClientImpl_->appendObject(bucket, objectKey, content, offset, builder);
}

Outcome<TosError, SetObjectMetaOutput> TosClient::setObjectMeta(const std::string &bucket, const std::string &objectKey,
                                                                const RequestOptionBuilder &builder) {
  return tosClientImpl_->setObjectMeta(bucket, objectKey, builder);
}

Outcome<TosError, ListObjectsOutput> TosClient::listObjects(const std::string &bucket, const ListObjectsInput &input) {
  return tosClientImpl_->listObjects(bucket, input);
}
Outcome<TosError, ListObjectVersionsOutput> TosClient::listObjectVersions(const std::string &bucket, const ListObjectVersionsInput &input) {
  return tosClientImpl_->listObjectVersions(bucket, input);
}


Outcome<TosError, CopyObjectOutput> TosClient::copyObject(const std::string &bucket,
                                                          const std::string &srcObjectKey, const std::string &dstObjectKey) {
  return tosClientImpl_->copyObject(bucket, srcObjectKey, dstObjectKey);
}
Outcome<TosError, CopyObjectOutput> TosClient::copyObject(const std::string &bucket,
                                                          const std::string &srcObjectKey, const std::string &dstObjectKey,
                                                          const RequestOptionBuilder &builder) {
  return tosClientImpl_->copyObject(bucket, srcObjectKey, dstObjectKey, builder);
}
Outcome<TosError, CopyObjectOutput> TosClient::copyObjectTo(const std::string &bucket, const std::string &dstBucket,
                                                            const std::string &dstObjectKey, const std::string &srcObjectKey) {
  return tosClientImpl_->copyObjectTo(bucket, dstBucket, dstObjectKey, srcObjectKey);
}
Outcome<TosError, CopyObjectOutput> TosClient::copyObjectTo(const std::string &bucket, const std::string &dstBucket,
                                                            const std::string &dstObjectKey, const std::string &srcObjectKey,
                                                            const RequestOptionBuilder &builder) {
  return tosClientImpl_->copyObjectTo(bucket, dstBucket, dstObjectKey, srcObjectKey, builder);
}
Outcome<TosError, CopyObjectOutput> TosClient::copyObjectFrom(const std::string &bucket, const std::string &srcBucket,
                                                              const std::string &srcObjectKey, const std::string &dstObjectKey) {
  return tosClientImpl_->copyObjectFrom(bucket, srcBucket, srcObjectKey, dstObjectKey);
}
Outcome<TosError, CopyObjectOutput> TosClient::copyObjectFrom(const std::string &bucket, const std::string &srcBucket,
                                                              const std::string &srcObjectKey, const std::string &dstObjectKey,
                                                              const RequestOptionBuilder &builder) {
  return tosClientImpl_->copyObjectFrom(bucket, srcBucket, srcObjectKey, dstObjectKey, builder);
}
Outcome<TosError, UploadPartCopyOutput>TosClient::uploadPartCopy(const std::string &bucket, const UploadPartCopyInput &input) {
  return tosClientImpl_->uploadPartCopy(bucket, input);
}
Outcome<TosError, UploadPartCopyOutput> TosClient::uploadPartCopy(const std::string &bucket, const UploadPartCopyInput &input,
                                                                  const RequestOptionBuilder &builder) {
  return tosClientImpl_->uploadPartCopy(bucket, input, builder);
}


Outcome<TosError, PutObjectAclOutput> TosClient::putObjectAcl(const std::string &bucket, const PutObjectAclInput &input) {
  return tosClientImpl_->putObjectAcl(bucket, input);
}
Outcome<TosError, GetObjectAclOutput> TosClient::getObjectAcl(const std::string &bucket, const std::string &objectKey) {
  return tosClientImpl_->getObjectAcl(bucket, objectKey);
}
Outcome<TosError, GetObjectAclOutput> TosClient::getObjectAcl(const std::string &bucket, const std::string &objectKey,
                                                              const RequestOptionBuilder &builder) {
  return tosClientImpl_->getObjectAcl(bucket, objectKey, builder);
}


Outcome<TosError, CreateMultipartUploadOutput> TosClient::createMultipartUpload(const std::string &bucket, const std::string &objectKey) {
  return tosClientImpl_->createMultipartUpload(bucket, objectKey);
}
Outcome<TosError, CreateMultipartUploadOutput> TosClient::createMultipartUpload(const std::string &bucket, const std::string &objectKey,
                                                                                const RequestOptionBuilder &builder) {
  return tosClientImpl_->createMultipartUpload(bucket, objectKey, builder);
}
Outcome<TosError, UploadPartOutput> TosClient::uploadPart(const std::string &bucket, const UploadPartInput &input) {
  return tosClientImpl_->uploadPart(bucket, input);
}
Outcome<TosError, UploadPartOutput> TosClient::uploadPart(const std::string &bucket, const UploadPartInput &input,
                                                          const RequestOptionBuilder &builder) {
  return tosClientImpl_->uploadPart(bucket, input, builder);
}
Outcome<TosError, CompleteMultipartUploadOutput> TosClient::completeMultipartUpload(const std::string &bucket, CompleteMultipartUploadInput &input) {
  return tosClientImpl_->completeMultipartUpload(bucket, input);
}
Outcome<TosError, CompleteMultipartUploadOutput> TosClient::completeMultipartUpload(const std::string &bucket, CompleteMultipartCopyUploadInput &input) {
  return tosClientImpl_->completeMultipartUpload(bucket, input);
}
Outcome<TosError, AbortMultipartUploadOutput> TosClient::abortMultipartUpload(const std::string &bucket, const AbortMultipartUploadInput &input) {
  return tosClientImpl_->abortMultipartUpload(bucket, input);
}
Outcome<TosError, ListUploadedPartsOutput> TosClient::listUploadedParts(const std::string &bucket, const ListUploadedPartsInput &input) {
  return tosClientImpl_->listUploadedParts(bucket, input);
}
Outcome<TosError, ListUploadedPartsOutput> TosClient::listUploadedParts(const std::string &bucket, const ListUploadedPartsInput &input,
                                                                        const RequestOptionBuilder &builder) {
  return tosClientImpl_->listUploadedParts(bucket, input, builder);
}
Outcome<TosError, ListMultipartUploadsOutput> TosClient::listMultipartUploads(const std::string &bucket, const ListMultipartUploadsInput &input) {
  return tosClientImpl_->listMultipartUploads(bucket, input);
}

Outcome<TosError, std::string> TosClient::preSignedURL(const std::string &httpMethod,
                                                       const std::string &bucket, const std::string &objectKey,
                                                       std::chrono::duration<int> ttl) {
  return tosClientImpl_->preSignedURL(httpMethod, bucket, objectKey, ttl);
}
Outcome<TosError, std::string> TosClient::preSignedURL(const std::string &httpMethod,
                                                       const std::string &bucket, const std::string &objectKey,
                                                       std::chrono::duration<int> ttl, const RequestOptionBuilder &builder) {
  return tosClientImpl_->preSignedURL(httpMethod, bucket, objectKey, ttl);
}
Outcome<TosError, PutBucketPolicyOutput>
TosClient::putBucketPolicy(const std::string &bucket,
                           const std::string &policy) {
  return tosClientImpl_->putBucketPolicy(bucket, policy);
}
Outcome<TosError, GetBucketPolicyOutput>
TosClient::getBucketPolicy(const std::string &bucket) {
  return tosClientImpl_->getBucketPolicy(bucket);
}
Outcome<TosError, DeleteBucketPolicyOutput>
TosClient::deleteBucketPolicy(const std::string &bucket) {
  return tosClientImpl_->deleteBucketPolicy(bucket);
}
