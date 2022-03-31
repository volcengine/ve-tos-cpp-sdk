#pragma once

#include <string>
#include "Config.h"
#include "Outcome.h"
#include "TosError.h"
#include "TosResponse.h"
#include "TosRequest.h"
#include "model/bucket/CreateBucketOutput.h"
#include "model/bucket/CreateBucketInput.h"
#include "model/bucket/HeadBucketOutput.h"
#include "model/bucket/DeleteBucketOutput.h"
#include "model/bucket/ListBucketsOutput.h"
#include "model/object/GetObjectOutput.h"
#include "model/bucket/ListBucketsInput.h"
#include "RequestOptionBuilder.h"
#include "model/object/HeadObjectOutput.h"
#include "model/object/DeleteObjectOutput.h"
#include "model/object/DeleteMultiObjectsOutput.h"
#include "model/object/DeleteMultiObjectsInput.h"
#include "model/object/PutObjectOutput.h"
#include "model/object/AppendObjectOutput.h"
#include "model/object/SetObjectMetaOutput.h"
#include "model/object/ListObjectsInput.h"
#include "model/object/ListObjectsOutput.h"
#include "model/object/ListObjectVersionsOutput.h"
#include "model/object/ListObjectVersionsInput.h"
#include "model/object/CopyObjectOutput.h"
#include "model/object/UploadPartCopyOutput.h"
#include "model/object/UploadPartCopyInput.h"
#include "model/acl/PutObjectAclInput.h"
#include "model/acl/PutObjectAclOutput.h"
#include "model/acl/GetObjectAclOutput.h"
#include "model/object/CreateMultipartUploadOutput.h"
#include "model/object/UploadPartOutput.h"
#include "model/object/UploadPartInput.h"
#include "model/object/CompleteMultipartUploadOutput.h"
#include "model/object/CompleteMultipartUploadInput.h"
#include "model/object/AbortMultipartUploadOutput.h"
#include "model/object/AbortMultipartUploadInput.h"
#include "model/object/ListUploadedPartsInput.h"
#include "model/object/ListUploadedPartsOutput.h"
#include "model/object/ListMultipartUploadsOutput.h"
#include "model/object/ListMultipartUploadsInput.h"
#include "auth/StaticCredentials.h"

namespace VolcengineTos {
static const char* TOS_SDK_VERSION = "0.1.0";
#if defined(WINDOWS_PLATFORM)
static const char* PLATFORM_NAME = "Windows";
#elif defined(LINUX_PLATFORM)
static const char* PLATFORM_NAME = "Linux";
#elif defined(APPLE_PLATFORM)
static const char* PLATFORM_NAME = "MacOS";
#else
static const char* PLATFORM_NAME = "Unknown";
#endif
static std::string DefaultUserAgent()
{
  std::stringstream ss;
  ss << "ve-tos-cpp-sdk/" << TOS_SDK_VERSION << " (" << PLATFORM_NAME << ")";
  return ss.str();
}

void InitializeClient();
void CloseClient();

class TosClientImpl;
class TosClient {
public:
  TosClient(const std::string& endpoint, const std::string& region,
            const std::string &accessKeyId, const std::string &secretKeyId);

  TosClient(const std::string& endpoint, const std::string& region,
            const std::string &accessKeyId, const std::string &secretKeyId, const std::string &securityToken);

  TosClient(const std::string& endpoint, const std::string& region, const StaticCredentials &cred);
  virtual ~TosClient();

  Outcome<TosError, CreateBucketOutput> createBucket(const CreateBucketInput& input);
  Outcome<TosError, HeadBucketOutput> headBucket(const std::string& bucket);
  Outcome<TosError, DeleteBucketOutput> deleteBucket(const std::string& bucket);
  Outcome<TosError, ListBucketsOutput> listBuckets(const ListBucketsInput& input);

  Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket, const std::string& objectKey);
  Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket, const std::string& objectKey);
  Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket, const std::string& objectKey);
  Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(const std::string& bucket, DeleteMultiObjectsInput& input);
  Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(const std::string& bucket, DeleteMultiObjectsInput& input, const RequestOptionBuilder& builder);
  Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream> &content);
  Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream> &content, const RequestOptionBuilder& builder);

  Outcome<TosError, AppendObjectOutput> appendObject(const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream> &content, int64_t offset);
  Outcome<TosError, AppendObjectOutput> appendObject(const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream> &content, int64_t offset, const RequestOptionBuilder& builder);

  Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);

  Outcome<TosError, ListObjectsOutput> listObjects(const std::string& bucket, const ListObjectsInput& input);
  Outcome<TosError, ListObjectVersionsOutput> listObjectVersions(const std::string& bucket, const ListObjectVersionsInput& input);

  Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket, const std::string& srcObjectKey, const std::string& dstObjectKey);
  Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket, const std::string& srcObjectKey, const std::string& dstObjectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket, const std::string& dstBucket, const std::string& dstObjectKey, const std::string& srcObjectKey);
  Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket, const std::string& dstBucket, const std::string& dstObjectKey, const std::string& srcObjectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket, const std::string& srcBucket, const std::string& srcObjectKey, const std::string& dstObjectKey);
  Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket, const std::string& srcBucket, const std::string& srcObjectKey, const std::string& dstObjectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket, const UploadPartCopyInput& input);
  Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket, const UploadPartCopyInput& input, const RequestOptionBuilder& builder);

  Outcome<TosError, PutObjectAclOutput> putObjectAcl(const std::string& bucket, const PutObjectAclInput& input);
  Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket, const std::string& objectKey);
  Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);

  Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const std::string& bucket, const std::string& objectKey);
  Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);
  Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket, const UploadPartInput& input);
  Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket, const UploadPartInput& input, const RequestOptionBuilder& builder);
  Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(const std::string& bucket, CompleteMultipartUploadInput& input);
  Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(const std::string& bucket, CompleteMultipartCopyUploadInput& input);
  Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(const std::string& bucket, const AbortMultipartUploadInput& input);
  Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(const std::string& bucket, const ListUploadedPartsInput& input);
  Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(const std::string& bucket, const ListUploadedPartsInput& input, const RequestOptionBuilder& builder);
  Outcome<TosError, ListMultipartUploadsOutput> listMultipartUploads(const std::string& bucket, const ListMultipartUploadsInput& input);

  Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod, const std::string& bucket, const std::string& objectKey, std::chrono::duration<int> ttl);
  Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod, const std::string& bucket, const std::string& objectKey, std::chrono::duration<int> ttl, const RequestOptionBuilder& builder);

private:
  std::shared_ptr<TosClientImpl> tosClientImpl_;
};
} // namespace VolcengineTos