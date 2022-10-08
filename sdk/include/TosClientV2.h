#pragma once

#include "TosClient.h"

#include <string>
#include "Config.h"
#include "Outcome.h"
#include "TosError.h"
#include "TosResponse.h"
#include "TosRequest.h"
#include "model/bucket/CreateBucketV2Output.h"
#include "model/bucket/CreateBucketV2Input.h"
#include "model/bucket/HeadBucketV2Output.h"
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
#include "model/bucket/HeadBucketV2Input.h"
#include "model/bucket/DeleteBucketInput.h"
#include "model/object/GetObjectV2Output.h"
#include "model/object/GetObjectV2Input.h"
#include "model/object/HeadObjectV2Output.h"
#include "model/object/HeadObjectV2Input.h"
#include "model/object/DeleteObjectInput.h"
#include "model/object/ListPartsInput.h"
#include "model/object/ListPartsOutput.h"
#include "model/object/PutObjectV2Output.h"
#include "model/object/PutObjectV2Input.h"
#include "model/object/AppendObjectV2Input.h"
#include "model/object/AppendObjectV2Output.h"
#include "model/object/SetObjectMetaInput.h"
#include "model/object/ListObjectsV2Input.h"
#include "model/object/ListObjectsV2Output.h"
#include "model/object/ListObjectVersionsV2Input.h"
#include "model/object/ListObjectVersionsV2Output.h"
#include "model/object/CopyObjectV2Input.h"
#include "model/object/CopyObjectV2Output.h"
#include "model/object/UploadPartCopyV2Input.h"
#include "model/object/UploadPartCopyV2Output.h"
#include "model/acl/PutObjectAclV2Input.h"
#include "model/acl/PutObjectAclV2Output.h"
#include "model/acl/GetObjectAclV2Input.h"
#include "model/acl/GetObjectAclV2Output.h"
#include "model/object/CreateMultipartUploadInput.h"
#include "model/object/UploadPartV2Input.h"
#include "model/object/UploadPartV2Output.h"
#include "model/object/ListMultipartUploadsV2Input.h"
#include "model/object/ListMultipartUploadsV2Output.h"
#include "model/object/CompleteMultipartUploadV2Output.h"
#include "model/object/CompleteMultipartUploadV2Input.h"
#include "model/object/GetObjectToFileOutput.h"
#include "model/object/GetObjectToFileInput.h"
#include "model/object/PutObjectFromFileOutput.h"
#include "model/object/PutObjectFromFileIntput.h"
#include "model/object/UploadPartFromFileOutput.h"
#include "model/object/UploadPartFromFileInput.h"
#include "model/object/UploadFileV2Output.h"
#include "model/object/UploadFileV2Input.h"
#include "Type.h"
#include "model/object/DownloadFileInput.h"
#include "model/object/DownloadFileOutput.h"

namespace VolcengineTos {

void InitializeClient();
void CloseClient();

class TosClientV2 : public TosClient {
public:
    TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId);
    TosClientV2(const std::string& region, const StaticCredentials& cred);
    TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                const std::string& securityToken);
    TosClientV2(const std::string& region, const FederationCredentials& cred);
    TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                const ClientConfig& config);
    TosClientV2(const std::string& region, const StaticCredentials& cred, const ClientConfig& config);
    TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                const std::string& securityToken, const ClientConfig& config);
    TosClientV2(const std::string& region, const FederationCredentials& cred, const ClientConfig& config);

    ~TosClientV2() override = default;

    Outcome<TosError, CreateBucketV2Output> createBucket(const CreateBucketV2Input& input);
    Outcome<TosError, HeadBucketV2Output> headBucket(const HeadBucketV2Input& input);
    Outcome<TosError, DeleteBucketOutput> deleteBucket(const DeleteBucketInput& input);

    Outcome<TosError, GetObjectV2Output> getObject(const GetObjectV2Input& input);

    Outcome<TosError, GetObjectToFileOutput> getObjectToFile(const GetObjectToFileInput& input);

    Outcome<TosError, HeadObjectV2Output> headObject(const HeadObjectV2Input& input);

    Outcome<TosError, CopyObjectV2Output> copyObject(const CopyObjectV2Input& input);

    Outcome<TosError, DeleteObjectOutput> deleteObject(const DeleteObjectInput& input);

    Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(DeleteMultiObjectsInput& input);

    Outcome<TosError, PutObjectV2Output> putObject(const PutObjectV2Input& input);
    Outcome<TosError, PutObjectFromFileOutput> putObjectFromFile(const PutObjectFromFileInput& input);

    Outcome<TosError, AppendObjectV2Output> appendObject(const AppendObjectV2Input& input);

    Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const SetObjectMetaInput& input);

    Outcome<TosError, ListObjectsV2Output> listObjects(const ListObjectsV2Input& input);

    Outcome<TosError, ListObjectVersionsV2Output> listObjectVersions(const ListObjectVersionsV2Input& input);

    Outcome<TosError, PutObjectAclV2Output> putObjectAcl(const PutObjectAclV2Input& input);

    Outcome<TosError, GetObjectAclV2Output> getObjectAcl(const GetObjectAclV2Input& input);

    Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const CreateMultipartUploadInput& input);

    Outcome<TosError, UploadPartV2Output> uploadPart(const UploadPartV2Input& input);

    Outcome<TosError, UploadPartFromFileOutput> uploadPartFromFile(const UploadPartFromFileInput& input);

    Outcome<TosError, CompleteMultipartUploadV2Output> completeMultipartUpload(CompleteMultipartUploadV2Input& input);

    Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(const AbortMultipartUploadInput& input);
    Outcome<TosError, UploadPartCopyV2Output> uploadPartCopy(const UploadPartCopyV2Input& input);

    Outcome<TosError, ListPartsOutput> listParts(const ListPartsInput& input);

    Outcome<TosError, ListMultipartUploadsV2Output> listMultipartUploads(const ListMultipartUploadsV2Input& input);

    Outcome<TosError, UploadFileV2Output> uploadFile(const UploadFileV2Input& input);
    Outcome<TosError, DownloadFileOutput> downloadFile(const DownloadFileInput& input);

private:
    std::shared_ptr<TosClientImpl> tosClientImpl_;
};
}  // namespace VolcengineTos
