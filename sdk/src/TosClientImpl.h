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
#include "model/object/DeleteObjectInput.h"
#include "model/object/DeleteMultiObjectsOutput.h"
#include "model/object/PutObjectOutput.h"
#include "model/object/AppendObjectOutput.h"
#include "model/object/SetObjectMetaOutput.h"
#include "model/object/SetObjectMetaInput.h"
#include "model/object/CopyObjectOutput.h"
#include "model/object/CopyObjectV2Output.h"
#include "model/object/CopyObjectV2Input.h"
#include "model/object/UploadPartCopyInput.h"
#include "model/object/UploadPartCopyOutput.h"
#include "model/acl/GetObjectAclOutput.h"
#include "model/acl/GetObjectAclV2Input.h"
#include "model/acl/GetObjectAclV2Output.h"
#include "model/object/CreateMultipartUploadOutput.h"
#include "model/object/UploadPartOutput.h"
#include "model/object/UploadPartInput.h"
#include "model/object/ListUploadedPartsOutput.h"
#include "model/acl/PutObjectAclOutput.h"
#include "model/acl/PutObjectAclInput.h"
#include "model/bucket/ListBucketsOutput.h"
#include "model/bucket/CreateBucketOutput.h"
#include "model/bucket/CreateBucketV2Output.h"
#include "model/bucket/HeadBucketOutput.h"
#include "model/bucket/HeadBucketV2Output.h"
#include "model/bucket/HeadBucketV2Input.h"
#include "model/bucket/DeleteBucketOutput.h"
#include "model/bucket/DeleteBucketInput.h"
#include "model/bucket/CreateBucketV2Input.h"
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
#include "model/object/UploadFileCheckpoint.h"
#include "auth/FederationCredentials.h"
#include "model/object/AppendObjectV2Output.h"
#include "model/object/AppendObjectV2Input.h"
#include "model/object/CompleteMultipartUploadV2Output.h"
#include "model/object/CompleteMultipartUploadV2Input.h"
#include "model/object/CreateMultipartUploadInput.h"
#include "model/object/GetObjectV2Output.h"
#include "model/object/GetObjectV2Input.h"
#include "model/object/GetObjectToFileInput.h"
#include "model/object/GetObjectToFileOutput.h"
#include "model/object/HeadObjectV2Output.h"
#include "model/object/HeadObjectV2Input.h"
#include "model/object/ListObjectsV2Output.h"
#include "model/object/ListMultipartUploadsV2Output.h"
#include "model/object/ListMultipartUploadsV2Input.h"
#include "model/object/ListObjectsV2Input.h"
#include "model/object/ListObjectVersionsV2Input.h"
#include "model/object/ListObjectVersionsV2Output.h"
#include "model/object/ListPartsOutput.h"
#include "model/object/ListPartsInput.h"
#include "model/object/PutObjectV2Input.h"
#include "model/object/PutObjectV2Output.h"
#include "model/object/PutObjectFromFileOutput.h"
#include "model/object/PutObjectFromFileIntput.h"
#include "model/object/UploadPartCopyV2Input.h"
#include "model/object/UploadPartCopyV2Output.h"
#include "model/object/UploadPartV2Output.h"
#include "model/object/UploadPartV2Input.h"
#include "model/object/UploadPartFromFileOutput.h"
#include "model/object/UploadPartFromFileInput.h"
#include "model/acl/PutObjectAclV2Input.h"
#include "model/acl/PutObjectAclV2Output.h"
#include "model/object/UploadFileV2Input.h"
#include "model/object/UploadFileV2Output.h"
#include "model/object/DownloadFileOutput.h"
#include "model/object/DownloadFileInput.h"
#include "model/object/DownloadFileCheckpoint.h"
#include "model/object/UploadFileInfoV2.h"
#include "model/object/UploadFileCheckpointV2.h"
#include "model/object/PreSignedURLOutput.h"
#include "model/object/PreSignedURLInput.h"

namespace VolcengineTos {
class TosClientImpl {
public:
    TosClientImpl(const std::string& endpoint, const std::string& region, const StaticCredentials& cred);
    TosClientImpl(const std::string& endpoint, const std::string& region, const FederationCredentials& cred);
    TosClientImpl(const std::string& endpoint, const std::string& region, const StaticCredentials& cred,
                  const ClientConfig& config);
    TosClientImpl(const std::string& endpoint, const std::string& region, const FederationCredentials& cred,
                  const ClientConfig& config);

    ~TosClientImpl() = default;
    Outcome<TosError, CreateBucketOutput> createBucket(const CreateBucketInput& input);
    Outcome<TosError, CreateBucketV2Output> createBucket(const CreateBucketV2Input& input);
    Outcome<TosError, HeadBucketOutput> headBucket(const std::string& bucket);
    Outcome<TosError, HeadBucketV2Output> headBucket(const HeadBucketV2Input& input);
    Outcome<TosError, DeleteBucketOutput> deleteBucket(const std::string& bucket);
    Outcome<TosError, DeleteBucketOutput> deleteBucket(const DeleteBucketInput& input);
    Outcome<TosError, ListBucketsOutput> listBuckets(const ListBucketsInput& input);
    Outcome<TosError, PutBucketPolicyOutput> putBucketPolicy(const std::string& bucket, const std::string& policy);
    Outcome<TosError, GetBucketPolicyOutput> getBucketPolicy(const std::string& bucket);
    Outcome<TosError, DeleteBucketPolicyOutput> deleteBucketPolicy(const std::string& bucket);

    Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket, const std::string& objectKey);
    Outcome<TosError, GetObjectOutput> getObject(const std::string& bucket, const std::string& objectKey,
                                                 const RequestOptionBuilder& builder);
    Outcome<TosError, GetObjectV2Output> getObject(const GetObjectV2Input& input,
                                                   std::shared_ptr<uint64_t> hashCrc64ecma);
    Outcome<TosError, GetObjectToFileOutput> getObjectToFile(const GetObjectToFileInput& input);
    Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket, const std::string& objectKey);
    Outcome<TosError, HeadObjectOutput> headObject(const std::string& bucket, const std::string& objectKey,
                                                   const RequestOptionBuilder& builder);
    Outcome<TosError, HeadObjectV2Output> headObject(const HeadObjectV2Input& input);
    Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket, const std::string& objectKey);
    Outcome<TosError, DeleteObjectOutput> deleteObject(const std::string& bucket, const std::string& objectKey,
                                                       const RequestOptionBuilder& builder);
    Outcome<TosError, DeleteObjectOutput> deleteObject(const DeleteObjectInput& input);
    Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(const std::string& bucket,
                                                                   DeleteMultiObjectsInput& input);
    Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(const std::string& bucket,
                                                                   DeleteMultiObjectsInput& input,
                                                                   const RequestOptionBuilder& builder);
    Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(DeleteMultiObjectsInput& input);
    Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket, const std::string& objectKey,
                                                 const std::shared_ptr<std::iostream>& content);
    Outcome<TosError, PutObjectOutput> putObject(const std::string& bucket, const std::string& objectKey,
                                                 const std::shared_ptr<std::iostream>& content,
                                                 const RequestOptionBuilder& builder);
    Outcome<TosError, PutObjectV2Output> putObject(const PutObjectV2Input& input);
    Outcome<TosError, PutObjectFromFileOutput> putObjectFromFile(const PutObjectFromFileInput& input);
    Outcome<TosError, UploadFileOutput> uploadFile(const std::string& bucket, const UploadFileInput& input,
                                                   const RequestOptionBuilder& builder);
    Outcome<TosError, UploadFileV2Output> uploadFile(const UploadFileV2Input& input);
    Outcome<TosError, DownloadFileOutput> downloadFile(const DownloadFileInput& input);
    Outcome<TosError, AppendObjectOutput> appendObject(const std::string& bucket, const std::string& objectKey,
                                                       const std::shared_ptr<std::iostream>& content, int64_t offset);
    Outcome<TosError, AppendObjectOutput> appendObject(const std::string& bucket, const std::string& objectKey,
                                                       const std::shared_ptr<std::iostream>& content, int64_t offset,
                                                       const RequestOptionBuilder& builder);
    Outcome<TosError, AppendObjectV2Output> appendObject(const AppendObjectV2Input& input);
    Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const std::string& bucket, const std::string& objectKey,
                                                         const RequestOptionBuilder& builder);
    Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const SetObjectMetaInput& input);
    Outcome<TosError, ListObjectsOutput> listObjects(const std::string& bucket, const ListObjectsInput& input);
    Outcome<TosError, ListObjectsV2Output> listObjects(const ListObjectsV2Input& input);
    Outcome<TosError, ListObjectVersionsOutput> listObjectVersions(const std::string& bucket,
                                                                   const ListObjectVersionsInput& input);
    Outcome<TosError, ListObjectVersionsV2Output> listObjectVersions(const ListObjectVersionsV2Input& input);
    Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket, const std::string& srcObjectKey,
                                                   const std::string& dstObjectKey);
    Outcome<TosError, CopyObjectOutput> copyObject(const std::string& bucket, const std::string& srcObjectKey,
                                                   const std::string& dstObjectKey,
                                                   const RequestOptionBuilder& builder);
    Outcome<TosError, CopyObjectV2Output> copyObject(const CopyObjectV2Input& input);
    Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket, const std::string& dstBucket,
                                                     const std::string& dstObjectKey, const std::string& srcObjectKey);
    Outcome<TosError, CopyObjectOutput> copyObjectTo(const std::string& bucket, const std::string& dstBucket,
                                                     const std::string& dstObjectKey, const std::string& srcObjectKey,
                                                     const RequestOptionBuilder& builder);
    Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket, const std::string& srcBucket,
                                                       const std::string& srcObjectKey,
                                                       const std::string& dstObjectKey);
    Outcome<TosError, CopyObjectOutput> copyObjectFrom(const std::string& bucket, const std::string& srcBucket,
                                                       const std::string& srcObjectKey, const std::string& dstObjectKey,
                                                       const RequestOptionBuilder& builder);
    Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket, const UploadPartCopyInput& input);
    Outcome<TosError, UploadPartCopyOutput> uploadPartCopy(const std::string& bucket, const UploadPartCopyInput& input,
                                                           const RequestOptionBuilder& builder);
    Outcome<TosError, UploadPartCopyV2Output> uploadPartCopy(const UploadPartCopyV2Input& input);
    Outcome<TosError, PutObjectAclOutput> putObjectAcl(const std::string& bucket, const PutObjectAclInput& input);
    Outcome<TosError, PutObjectAclV2Output> putObjectAcl(const PutObjectAclV2Input& input);
    Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket, const std::string& objectKey);
    Outcome<TosError, GetObjectAclOutput> getObjectAcl(const std::string& bucket, const std::string& objectKey,
                                                       const RequestOptionBuilder& builder);
    Outcome<TosError, GetObjectAclV2Output> getObjectAcl(const GetObjectAclV2Input& input);
    Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const std::string& bucket,
                                                                         const std::string& objectKey);
    Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const std::string& bucket,
                                                                         const std::string& objectKey,
                                                                         const RequestOptionBuilder& builder);
    Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const CreateMultipartUploadInput& input);
    Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket, const UploadPartInput& input);
    Outcome<TosError, UploadPartOutput> uploadPart(const std::string& bucket, const UploadPartInput& input,
                                                   const RequestOptionBuilder& builder);
    Outcome<TosError, UploadPartV2Output> uploadPart(const UploadPartV2Input& input,
                                                     std::shared_ptr<uint64_t> hashCrc64ecma);
    Outcome<TosError, UploadPartFromFileOutput> uploadPartFromFile(const UploadPartFromFileInput& input,
                                                                   std::shared_ptr<uint64_t> hashCrc64ecma);
    Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(const std::string& bucket,
                                                                             CompleteMultipartUploadInput& input);
    Outcome<TosError, CompleteMultipartUploadOutput> completeMultipartUpload(const std::string& bucket,
                                                                             CompleteMultipartCopyUploadInput& input);
    Outcome<TosError, CompleteMultipartUploadV2Output> completeMultipartUpload(
            const CompleteMultipartUploadV2Input& input);

    Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(const std::string& bucket,
                                                                       const AbortMultipartUploadInput& input);
    Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(const AbortMultipartUploadInput& input);
    Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(const std::string& bucket,
                                                                 const ListUploadedPartsInput& input);
    Outcome<TosError, ListUploadedPartsOutput> listUploadedParts(const std::string& bucket,
                                                                 const ListUploadedPartsInput& input,
                                                                 const RequestOptionBuilder& builder);
    Outcome<TosError, ListPartsOutput> listParts(const ListPartsInput& input);
    Outcome<TosError, ListMultipartUploadsOutput> listMultipartUploads(const std::string& bucket,
                                                                       const ListMultipartUploadsInput& input);
    Outcome<TosError, ListMultipartUploadsV2Output> listMultipartUploads(const ListMultipartUploadsV2Input& input);
    Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod, const std::string& bucket,
                                                const std::string& objectKey, std::chrono::duration<int> ttl);
    Outcome<TosError, std::string> preSignedURL(const std::string& httpMethod, const std::string& bucket,
                                                const std::string& objectKey, std::chrono::duration<int> ttl,
                                                const RequestOptionBuilder& builder);
    Outcome<TosError, PreSignedURLOutput> preSignedURL(const PreSignedURLInput& input);

protected:
    /**
     * URL_MODE_DEFAULT url pattern is http(s)://{bucket}.domain/{object}
     */
    static const int URL_MODE_DEFAULT = 0;

private:
    Outcome<TosError, std::shared_ptr<TosResponse>> roundTrip(const std::shared_ptr<TosRequest>& request,
                                                              int expectedCode);
    RequestBuilder newBuilder(const std::string& bucket, const std::string& object);
    RequestBuilder newBuilder(const std::string& bucket, const std::string& object,
                              const RequestOptionBuilder& builder);
    RequestBuilder newBuilder(const std::string& bucket, const std::string& object,
                              const std::string& alternativeEndpoint, const std::map<std::string, std::string>& headers,
                              std::map<std::string, std::string>& queries);
    std::string scheme_;
    std::string host_;
    int urlMode_ = URL_MODE_DEFAULT;
    std::string userAgent_ = DefaultUserAgent();
    std::shared_ptr<Credentials> credentials_;
    std::shared_ptr<Signer> signer_;
    std::shared_ptr<Transport> transport_;
    Config config_;
    std::map<std::string, std::string> supportedRegion_ = {{"cn-beijing", "https://tos-cn-beijing.volces.com"},
                                                           {"cn-guangzhou", "https://tos-cn-guangzhou.volces.com"},
                                                           {"cn-shanghai", "https://tos-cn-shanghai.volces.com"}};

    void getObject(RequestBuilder& rb, Outcome<TosError, GetObjectOutput>& res);
    void headObject(RequestBuilder& rb, Outcome<TosError, HeadObjectOutput>& res);
    void deleteObject(RequestBuilder& rb, Outcome<TosError, DeleteObjectOutput>& res);
    void deleteMultiObjects(RequestBuilder& rb, const std::string& data, const std::string& dataMd5,
                            Outcome<TosError, DeleteMultiObjectsOutput>& res);
    void putObject(const std::shared_ptr<TosRequest>& req, Outcome<TosError, PutObjectOutput>& res);
    void appendObject(const std::shared_ptr<TosRequest>& req, Outcome<TosError, AppendObjectOutput>& res);
    void setObjectMeta(RequestBuilder& rb, Outcome<TosError, SetObjectMetaOutput>& res);
    void copyObject(const std::string& dstBucket, const std::string& dstObject, const std::string& srcBucket,
                    const std::string& srcObject, Outcome<TosError, CopyObjectOutput>& outcome);
    void copyObject(const std::string& dstBucket, const std::string& dstObject, const std::string& srcBucket,
                    const std::string& srcObject, const RequestOptionBuilder& builder,
                    Outcome<TosError, CopyObjectOutput>& outcome);
    void copyObject(std::shared_ptr<TosRequest>& req, Outcome<TosError, CopyObjectOutput>& res);
    void uploadPartCopy(RequestBuilder& rb, const UploadPartCopyInput& input,
                        Outcome<TosError, UploadPartCopyOutput>& res);
    void getObjectAcl(RequestBuilder& rb, Outcome<TosError, GetObjectAclOutput>& res);
    void createMultipartUpload(RequestBuilder& rb, Outcome<TosError, CreateMultipartUploadOutput>& res);
    Outcome<TosError, UploadFileCheckpoint> initCheckpoint(const std::string& bucket, const UploadFileInput& input,
                                                           const UploadFileInfo& info,
                                                           const std::string& checkpointFilePath,
                                                           const RequestOptionBuilder& builder);
    Outcome<TosError, UploadFileCheckpoint> getCheckpoint(const std::string& bucket, const UploadFileInput& input,
                                                          const UploadFileInfo& fileInfo,
                                                          const std::string& checkpointFilePath,
                                                          const RequestOptionBuilder& builder);
    Outcome<TosError, UploadFileCheckpointV2> initCheckpoint(const std::string& bucket, const std::string& key,
                                                             const UploadFileV2Input& input,
                                                             const UploadFileInfoV2& info,
                                                             const std::string& checkpointFilePath,
                                                             std::shared_ptr<UploadEvent> event);
    Outcome<TosError, UploadFileCheckpointV2> getCheckpoint(const UploadFileV2Input& input,
                                                            const UploadFileInfoV2& fileInfo,
                                                            const std::string& checkpointFilePath,
                                                            std::shared_ptr<UploadEvent> event);
    Outcome<TosError, UploadFileOutput> uploadPartConcurrent(const UploadFileInput& input,
                                                             UploadFileCheckpoint checkpoint,
                                                             const RequestOptionBuilder& builder);
    Outcome<TosError, DownloadFileCheckpoint> getCheckpoint(const DownloadFileInput& input,
                                                            const std::string& checkpointFilePath,
                                                            const DownloadFileFileInfo& fileInfo,
                                                            const HeadObjectV2Output& headOutput,
                                                            const std::shared_ptr<DownloadEvent>& event);
    Outcome<TosError, DownloadFileCheckpoint> initCheckpoint(const DownloadFileInput& input,
                                                             const HeadObjectV2Output& headOutput,
                                                             const DownloadFileFileInfo& info,
                                                             const std::string& checkpointFilePath);
    Outcome<TosError, UploadFileV2Output> uploadPartConcurrent(const UploadFileV2Input& input,
                                                               UploadFileCheckpointV2 checkpoint,
                                                               const std::string& checkpointFilePath,
                                                               std::shared_ptr<UploadEvent> event);
    Outcome<TosError, DownloadFileOutput> downloadPartConcurrent(
            const DownloadFileInput& input, const HeadObjectV2Output& headOutput, DownloadFileCheckpoint checkpoint,
            const std::string& checkpointPath, const DownloadFileFileInfo& dfi, std::shared_ptr<DownloadEvent> event);
    void uploadPart(RequestBuilder& rb, const UploadPartInput& input, Outcome<TosError, UploadPartOutput>& res);

    void listUploadedParts(RequestBuilder& rb, const std::string& uploadId,
                           Outcome<TosError, ListUploadedPartsOutput>& res);
    void preSignedURL(RequestBuilder& rb, const std::string& method, const std::chrono::duration<int>& ttl,
                      Outcome<TosError, std::string>& res);

    void initWithoutConfig(const std::string& endpoint, const std::string& region);
    void init(const std::string& endpoint, const std::string& region);
    void initWithConfig(const std::string& endpoint, const std::string& region, const ClientConfig& config);
    void init(const std::string& endpoint, const std::string& region, const ClientConfig& config);
    void initSchemeAndHost(const std::string& endpoint, const std::string& region);
    int64_t calContentLength(const std::shared_ptr<std::iostream>& content);
    void SetCrc64ParmToReq(const std::shared_ptr<TosRequest>& req);
    void SetProcessHandlerToReq(const std::shared_ptr<TosRequest>& req, DataTransferListener& handler);
    void SetRateLimiterToReq(const std::shared_ptr<TosRequest>& req, const std::shared_ptr<RateLimiter>& limiter);
    bool checkShouldRetry(const std::shared_ptr<TosRequest>& request, const std::shared_ptr<TosResponse>& response);
    RequestBuilder ParamFromConfToRb(RequestBuilder& rb);
};
}  // namespace VolcengineTos
