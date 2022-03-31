#pragma once

#include "TosError.h"
#include "TosResponse.h"
#include "Outcome.h"
#include "RequestBuilder.h"
#include "RequestOptionBuilder.h"
#include "transport/Transport.h"
#include "Config.h"
#include "model/object/GetObjectOutput.h"
#include "model/object/HeadObjectOutput.h"
#include "model/object/DeleteObjectOutput.h"
#include "model/object/DeleteMultiObjectsOutput.h"
#include "model/object/PutObjectOutput.h"
#include "model/object/AppendObjectOutput.h"
#include "model/object/SetObjectMetaOutput.h"
#include "model/object/CopyObjectOutput.h"
#include "model/object/UploadPartCopyInput.h"
#include "model/object/UploadPartCopyOutput.h"
#include "model/acl/GetObjectAclOutput.h"
#include "model/object/CreateMultipartUploadOutput.h"
#include "model/object/UploadPartOutput.h"
#include "model/object/UploadPartInput.h"
#include "model/object/ListUploadedPartsOutput.h"
#include "model/acl/PutObjectAclOutput.h"
#include "model/acl/PutObjectAclInput.h"
#include "model/bucket/ListBucketsOutput.h"
#include "model/bucket/CreateBucketOutput.h"
#include "model/bucket/HeadBucketOutput.h"
#include "model/bucket/DeleteBucketOutput.h"
#include "model/bucket/CreateBucketInput.h"
#include "model/bucket/ListBucketsInput.h"
#include "model/object/CompleteMultipartUploadOutput.h"
#include "model/object/AbortMultipartUploadOutput.h"
#include "model/object/CompleteMultipartUploadInput.h"
#include "model/object/ListMultipartUploadsOutput.h"
#include "model/object/ListMultipartUploadsInput.h"
#include "model/object/ListObjectsOutput.h"
#include "model/object/ListObjectsInput.h"
#include "model/object/ListObjectVersionsInput.h"
#include "model/object/ListObjectVersionsOutput.h"
#include "model/object/AbortMultipartUploadInput.h"
#include "model/object/ListUploadedPartsInput.h"
#include "model/object/DeleteMultiObjectsInput.h"
#include "TosClient.h"

namespace VolcengineTos {
class TosClientImpl {
public:
  TosClientImpl(const std::string &endpoint, const std::string &region, const StaticCredentials &cred);
  ~TosClientImpl() = default;

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

protected:
  /**
     * URL_MODE_DEFAULT url pattern is http(s)://{bucket}.domain/{object}
   */
  static const int URL_MODE_DEFAULT = 0;

private:
  Outcome<TosError, std::shared_ptr<TosResponse>> roundTrip(const std::shared_ptr<TosRequest> &request, int expectedCode);
  RequestBuilder newBuilder(const std::string &bucket, const std::string &object);
  RequestBuilder newBuilder(const std::string &bucket, const std::string &object, const RequestOptionBuilder &builder);
  std::string scheme_;
  std::string host_;
  int urlMode_ = URL_MODE_DEFAULT;
  std::string userAgent_ = DefaultUserAgent();
  std::shared_ptr<Credentials> credentials_;
  std::shared_ptr<Signer> signer_;
  std::shared_ptr<Transport> transport_;
  Config config_;

  void getObject(RequestBuilder &rb, Outcome<TosError, GetObjectOutput> &res);
  void headObject(RequestBuilder &rb, Outcome<TosError, HeadObjectOutput> &res);
  void deleteObject(RequestBuilder &rb, Outcome<TosError, DeleteObjectOutput> &res);
  void deleteMultiObjects(RequestBuilder &rb, const std::string &data, const std::string & dataMd5, Outcome<TosError, DeleteMultiObjectsOutput> &res);
  void putObject(const std::shared_ptr<TosRequest> &req, Outcome<TosError, PutObjectOutput> &res);
  void appendObject(const std::shared_ptr<TosRequest> &req, Outcome<TosError, AppendObjectOutput> &res);
  void setObjectMeta(RequestBuilder &rb, Outcome<TosError, SetObjectMetaOutput> &res);
  void copyObject(const std::string &dstBucket, const std::string &dstObject,
                  const std::string &srcBucket, const std::string &srcObject,
                  Outcome<TosError, CopyObjectOutput> &outcome);
  void copyObject(const std::string &dstBucket, const std::string &dstObject,
                  const std::string &srcBucket, const std::string &srcObject,
                  const RequestOptionBuilder &builder,
                  Outcome<TosError, CopyObjectOutput> &outcome);
  void copyObject(std::shared_ptr<TosRequest> &req, Outcome<TosError, CopyObjectOutput> &res);
  void uploadPartCopy(RequestBuilder &rb, const UploadPartCopyInput &input, Outcome<TosError, UploadPartCopyOutput> &res);
  void getObjectAcl(RequestBuilder &rb, Outcome<TosError, GetObjectAclOutput> &res);
  void createMultipartUpload(RequestBuilder &rb, Outcome<TosError, CreateMultipartUploadOutput> &res);
  void uploadPart(RequestBuilder &rb, const UploadPartInput &input, Outcome<TosError, UploadPartOutput> &res);
  void listUploadedParts(RequestBuilder &rb, const std::string &uploadId, Outcome<TosError, ListUploadedPartsOutput> &res);
  void preSignedURL(RequestBuilder &rb, const std::string &method, const std::chrono::duration<int> &ttl, Outcome<TosError, std::string> &res);
};
}// namespace VolcengineTos
