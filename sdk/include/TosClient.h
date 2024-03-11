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
#include "model/bucket/PutBucketPolicyOutput.h"
#include "model/bucket/GetBucketPolicyOutput.h"
#include "model/bucket/DeleteBucketPolicyOutput.h"
#include "model/object/UploadFileOutput.h"
#include "model/object/UploadFileInput.h"
#include "auth/FederationCredentials.h"
#include "ClientConfig.h"

namespace VolcengineTos {
static const char* TOS_SDK_VERSION = "v2.6.6";
#ifdef WIN32
static const char* PLATFORM_NAME = "windows";
#elif __linux__
static const char* PLATFORM_NAME = "linux";
#elif __APPLE__
static const char* PLATFORM_NAME = "macOS";
#else
static const char* PLATFORM_NAME = "unknown";
#endif

#if defined(__aarch64__)
static const char* CPU_ARCH = "arm64";
#elif defined(__x86_64__)
static const char* CPU_ARCH = "amd64";
#elif defined(__i386__)
static const char* CPU_ARCH = "i386";
#else
static const char* CPU_ARCH = "others";
#endif

static std::string DefaultUserAgent() {
    std::stringstream ss;
    ss << "ve-tos-cpp-sdk/" << TOS_SDK_VERSION << " (" << PLATFORM_NAME << '/' << CPU_ARCH << ';' << "C++"
       << __cplusplus << ")";
    return ss.str();
}

class TosClientImpl;
class TosClient {
public:
    TosClient(const std::string& endpoint, const std::string& region, const std::string& accessKeyId,
              const std::string& secretKeyId);

    TosClient(const std::string& endpoint, const std::string& region, const std::string& accessKeyId,
              const std::string& secretKeyId, const std::string& securityToken);

    TosClient(const std::string& endpoint, const std::string& region, const StaticCredentials& cred);
    TosClient(const std::string& endpoint, const std::string& region, const FederationCredentials& cred);

    TosClient(const std::string& endpoint, const std::string& region, const StaticCredentials& cred,
              const ClientConfig& config);
    TosClient(const std::string& endpoint, const std::string& region, const FederationCredentials& cred,
              const ClientConfig& config);

    virtual ~TosClient();

    [[gnu::deprecated]] Outcome<TosError, CreateBucketOutput> createBucket(const CreateBucketInput& input);
    [[gnu::deprecated]] Outcome<TosError, HeadBucketOutput> headBucket(const std::string& bucket);
    [[gnu::deprecated]] Outcome<TosError, DeleteBucketOutput> deleteBucket(const std::string& bucket);
    Outcome<TosError, ListBucketsOutput> listBuckets(const ListBucketsInput& input);
    // 设置桶策略
    [[gnu::deprecated]] Outcome<TosError, PutBucketPolicyOutput> putBucketPolicy(const std::string& bucket,
                                                                                 const std::string& policy);
    [[gnu::deprecated]] Outcome<TosError, GetBucketPolicyOutput> getBucketPolicy(const std::string& bucket);
    [[gnu::deprecated]] Outcome<TosError, DeleteBucketPolicyOutput> deleteBucketPolicy(const std::string& bucket);

    [[gnu::deprecated]] Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket,
                                                                     const std::string& objectKey);
    // optional. setting withXXX properties.
    //   withVersionID: which version of this object.
    //   withRange: the range of content.
    //   withIfModifiedSince: return if the object modified after the given date,
    //     otherwise return status code 304.
    //   withIfUnmodifiedSince, withIfMatch, withIfNoneMatch set If-Unmodified-Since, If-Match and If-None-Match.
    [[gnu::deprecated]] Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket,
                                                                     const std::string& objectKey,
                                                                     const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket,
                                                                       const std::string& objectKey);
    // optional. setting withXXX properties.
    //   withVersionID: which version of this object.
    //   withRange: the range of content.
    //   withIfModifiedSince: return if the object modified after the given date,
    //   otherwise return status code 304.
    //   withIfUnmodifiedSince, withIfMatch, withIfNoneMatch set If-Unmodified-Since, If-Match and If-None-Match.
    [[gnu::deprecated]] Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket,
                                                                       const std::string& objectKey,
                                                                       const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket,
                                                                           const std::string& objectKey);
    // withVersionID: which version of this object will be deleted
    [[gnu::deprecated]] Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket,
                                                                           const std::string& objectKey,
                                                                           const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(const std::string& bucket,
                                                                                       DeleteMultiObjectsInput& input);
    [[gnu::deprecated]] Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(
            const std::string& bucket, DeleteMultiObjectsInput& input, const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket,
                                                                     const std::string& objectKey,
                                                                     const std::shared_ptr<std::iostream>& content);
    // optional. setting withXXX properties.
    //   withContentType: set Content-Type.
    //   withContentDisposition: set Content-Disposition.
    //   withContentLanguage: set Content-Language.
    //   withContentEncoding: set Content-Encoding.
    //   withCacheControl: set Cache-Control.
    //   withExpires: set Expires.
    //   withMeta: set meta header(s).
    //   withContentSHA256: set Content-Sha256.
    //   withContentMD5: set Content-MD5.
    //   withExpires: set Expires.
    //   withServerSideEncryptionCustomer: set server side encryption options.
    //   withACL, withACLGrantFullControl, withACLGrantRead, withACLGrantReadAcp,
    //   withACLGrantWrite, withACLGrantWriteAcp set object acl.
    //   withStorageClass set storage class, 'STANDARD|IA'
    [[gnu::deprecated]] Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket,
                                                                     const std::string& objectKey,
                                                                     const std::shared_ptr<std::iostream>& content,
                                                                     const RequestOptionBuilder& builder);

    [[gnu::deprecated]] Outcome<TosError, UploadFileOutput> uploadFile(const std::string& bucket,
                                                                       const UploadFileInput& input);
    [[gnu::deprecated]] Outcome<TosError, UploadFileOutput> uploadFile(const std::string& bucket,
                                                                       const UploadFileInput& input,
                                                                       const RequestOptionBuilder& builder);

    [[gnu::deprecated]] Outcome<TosError, AppendObjectOutput> appendObject(
            const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream>& content,
            int64_t offset);
    // optional. setting withXXX properties.
    //   withContentType: set Content-Type.
    //   withContentDisposition: set Content-Disposition.
    //   withContentLanguage: set Content-Language.
    //   withContentEncoding: set Content-Encoding.
    //   withCacheControl: set Cache-Control.
    //   withExpires: set Expires.
    //   withMeta: set meta header(s).
    //   withExpires: set Expires.
    //   withACL, withACLGrantFullControl, withACLGrantRead, withACLGrantReadAcp,
    //   withACLGrantWrite, withACLGrantWriteAcp set object acl.
    //   withStorageClass set storage class, 'STANDARD|IA'
    //   above options only take effect when offset parameter is 0.
    //
    //   withContentSHA256: set Content-Sha256.
    //   withContentMD5: set Content-MD5.
    [[gnu::deprecated]] Outcome<TosError, AppendObjectOutput> appendObject(
            const std::string& bucket, const std::string& objectKey, const std::shared_ptr<std::iostream>& content,
            int64_t offset, const RequestOptionBuilder& builder);

    // optional. setting withXXX properties.
    //   withContentType set Content-Type.
    //   withContentDisposition set Content-Disposition.
    //   withContentLanguage set Content-Language.
    //   withContentEncoding set Content-Encoding.
    //   withCacheControl set Cache-Control.
    //   withExpires set Expires.
    //   withMeta set meta header(s).
    //   withVersionID which version of this object will be set
    [[gnu::deprecated]] Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const std::string& bucket,
                                                                             const std::string& objectKey,
                                                                             const RequestOptionBuilder& builder);

    [[gnu::deprecated]] Outcome<TosError, ListObjectsOutput> listObjects(const std::string& bucket,
                                                                         const ListObjectsInput& input);
    [[gnu::deprecated]] Outcome<TosError, ListObjectVersionsOutput> listObjectVersions(
            const std::string& bucket, const ListObjectVersionsInput& input);

    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket,
                                                                       const std::string& srcObjectKey,
                                                                       const std::string& dstObjectKey);
    // optional. setting withXXX properties.
    //   withVersionID the version id of source object.
    //   withMetadataDirective copy source object metadata or replace with new object metadata.
    //
    //   withACL withACLGrantFullControl withACLGrantRead withACLGrantReadAcp
    //   withACLGrantWrite withACLGrantWriteAcp set object acl.
    //
    //   withCopySourceIfMatch withCopySourceIfNoneMatch withCopySourceIfModifiedSince
    //   withCopySourceIfUnmodifiedSince set copy conditions.
    //
    //   withStorageClass set storage class, 'STANDARD|IA'
    //
    //   withServerSideEncryptionCustomer: Copy SSE-C加密对象，源对象的加密算法、加密密钥、密钥MD5
    //
    //   if copyObject called with withMetadataDirective(TosHeaders.METADATA_DIRECTIVE_REPLACE),
    //   these properties can be used:
    //     withContentType set Content-Type.
    //     withContentDisposition set Content-Disposition.
    //     withContentLanguage set Content-Language.
    //     withContentEncoding set Content-Encoding.
    //     withCacheControl set Cache-Control.
    //     withExpires set Expires.
    //     withMeta set meta header(s),
    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket,
                                                                       const std::string& srcObjectKey,
                                                                       const std::string& dstObjectKey,
                                                                       const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket,
                                                                         const std::string& dstBucket,
                                                                         const std::string& dstObjectKey,
                                                                         const std::string& srcObjectKey);
    // optional. setting withXXX properties.
    //   withVersionID the version id of source object.
    //   withMetadataDirective copy source object metadata or replace with new object metadata.
    //
    //   withACL withACLGrantFullControl withACLGrantRead withACLGrantReadAcp
    //   withACLGrantWrite withACLGrantWriteAcp set object acl.
    //
    //   withCopySourceIfMatch withCopySourceIfNoneMatch withCopySourceIfModifiedSince
    //   withCopySourceIfUnmodifiedSince set copy conditions.
    //
    //   withStorageClass set storage class, 'STANDARD|IA'
    //
    //   withServerSideEncryptionCustomer: Copy SSE-C加密对象，源对象的加密算法、加密密钥、密钥MD5
    //
    //   if copyObjectTo called with withMetadataDirective(TosHeaders.METADATA_DIRECTIVE_REPLACE),
    //   these properties can be used:
    //     withContentType set Content-Type.
    //     withContentDisposition set Content-Disposition.
    //     withContentLanguage set Content-Language.
    //     withContentEncoding set Content-Encoding.
    //     withCacheControl set Cache-Control.
    //     withExpires set Expires.
    //     withMeta set meta header(s),
    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket,
                                                                         const std::string& dstBucket,
                                                                         const std::string& dstObjectKey,
                                                                         const std::string& srcObjectKey,
                                                                         const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket,
                                                                           const std::string& srcBucket,
                                                                           const std::string& srcObjectKey,
                                                                           const std::string& dstObjectKey);

    // optional. setting withXXX properties.
    //   withVersionID the version id of source object.
    //   withMetadataDirective copy source object metadata or replace with new object metadata.
    //
    //   withACL withACLGrantFullControl withACLGrantRead withACLGrantReadAcp
    //   withACLGrantWrite withACLGrantWriteAcp set object acl.
    //
    //   withCopySourceIfMatch withCopySourceIfNoneMatch withCopySourceIfModifiedSince
    //   withCopySourceIfUnmodifiedSince set copy conditions.
    //
    //   withStorageClass set storage class, 'STANDARD|IA'
    //
    //   withServerSideEncryptionCustomer: Copy SSE-C加密对象，源对象的加密算法、加密密钥、密钥MD5
    //
    //   if copyObjectFrom called with withMetadataDirective(TosHeaders.METADATA_DIRECTIVE_REPLACE),
    //   these properties can be used:
    //     withContentType set Content-Type.
    //     withContentDisposition set Content-Disposition.
    //     withContentLanguage set Content-Language.
    //     withContentEncoding set Content-Encoding.
    //     withCacheControl set Cache-Control.
    //     withExpires set Expires.
    //     withMeta set meta header(s),
    [[gnu::deprecated]] Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket,
                                                                           const std::string& srcBucket,
                                                                           const std::string& srcObjectKey,
                                                                           const std::string& dstObjectKey,
                                                                           const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket,
                                                                               const UploadPartCopyInput& input);
    // optional. setting withXXX properties.
    //   withCopySourceIfMatch, withCopySourceIfNoneMatch, withCopySourceIfModifiedSince and
    //   withCopySourceIfUnmodifiedSince set copy conditions
    //   withServerSideEncryption: set server side encryption algorithm, 'AES256'.
    //   withServerSideEncryptionCustomer: Copy SSE-C加密对象，源对象的加密算法、加密密钥、密钥MD5
    [[gnu::deprecated]] Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket,
                                                                               const UploadPartCopyInput& input,
                                                                               const RequestOptionBuilder& builder);

    [[gnu::deprecated]] Outcome<TosError, PutObjectAclOutput> putObjectAcl(const std::string& bucket,
                                                                           const PutObjectAclInput& input);
    [[gnu::deprecated]] Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket,
                                                                           const std::string& objectKey);
    // withVersionID the version of the object
    [[gnu::deprecated]] Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket,
                                                                           const std::string& objectKey,
                                                                           const RequestOptionBuilder& builder);

    [[gnu::deprecated]] Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(
            const std::string& bucket, const std::string& objectKey);
    // optional. setting withXXX properties.
    //   withContentType set Content-Type.
    //   withContentDisposition set Content-Disposition.
    //   withContentLanguage set Content-Language.
    //   withContentEncoding set Content-Encoding.
    //   withCacheControl set Cache-Control.
    //   withExpires set Expires.
    //   withMeta set meta header(s).
    //   withContentSHA256 set Content-Sha256.
    //   withContentMD5 set Content-MD5.
    //   withExpires set Expires.
    //   withServerSideEncryptionCustomer set server side encryption options.
    //   withACL, WithACLGrantFullControl, withACLGrantRead, withACLGrantReadAcp,
    //   withACLGrantWrite, withACLGrantWriteAcp set object acl.
    //   withStorageClass set storage class, 'STANDARD|IA'.
    //   withServerSideEncryption: set server side encryption algorithm, 'AES256'.
    [[gnu::deprecated]] Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(
            const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket,
                                                                       const UploadPartInput& input);
    [[gnu::deprecated]] Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket,
                                                                       const UploadPartInput& input,
                                                                       const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(
            const std::string& bucket, CompleteMultipartUploadInput& input);
    [[gnu::deprecated]] Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(
            const std::string& bucket, CompleteMultipartCopyUploadInput& input);
    [[gnu::deprecated]] Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(
            const std::string& bucket, const AbortMultipartUploadInput& input);
    [[gnu::deprecated]] Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(
            const std::string& bucket, const ListUploadedPartsInput& input);
    [[gnu::deprecated]] Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(
            const std::string& bucket, const ListUploadedPartsInput& input, const RequestOptionBuilder& builder);
    [[gnu::deprecated]] Outcome<TosError, ListMultipartUploadsOutput> listMultipartUploads(
            const std::string& bucket, const ListMultipartUploadsInput& input);

    [[gnu::deprecated]] Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod,
                                                                    const std::string& bucket,
                                                                    const std::string& objectKey,
                                                                    std::chrono::duration<int> ttl);
    [[gnu::deprecated]] Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod,
                                                                    const std::string& bucket,
                                                                    const std::string& objectKey,
                                                                    std::chrono::duration<int> ttl,
                                                                    const RequestOptionBuilder& builder);

private:
    std::shared_ptr<TosClientImpl> tosClientImpl_;
};
}  // namespace VolcengineTos
