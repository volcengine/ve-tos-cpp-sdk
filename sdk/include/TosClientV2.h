#pragma once

#include "TosClient.h"
#include <memory>
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
#include "model/object/PreSignedURLOutput.h"
#include "model/object/PreSignedURLInput.h"
#include "model/bucket/PutBucketCORSInput.h"
#include "model/bucket/PutBucketCORSOutput.h"
#include "model/bucket/GetBucketCORSInput.h"
#include "model/bucket/GetBucketCORSOutput.h"
#include "model/bucket/DeleteBucketCORSOutput.h"
#include "model/bucket/DeleteBucketCORSInput.h"
#include "model/object/ListObjectsType2Input.h"
#include "model/object/ListObjectsType2Output.h"
#include "model/bucket/PutBucketStorageClassOutput.h"
#include "model/bucket/PutBucketStorageClassInput.h"
#include "model/bucket/GetBucketLocationOutput.h"
#include "model/bucket/GetBucketLocationInput.h"
#include "model/bucket/PutBucketLifecycleInput.h"
#include "model/bucket/PutBucketLifecycleOutput.h"
#include "model/bucket/GetBucketLifecycleOutput.h"
#include "model/bucket/DeleteBucketLifecycleOutput.h"
#include "model/bucket/GetBucketLifecycleInput.h"
#include "model/bucket/DeleteBucketLifecycleInput.h"
#include "model/bucket/PutBucketPolicyInput.h"
#include "model/bucket/GetBucketPolicyInput.h"
#include "model/bucket/DeleteBucketPolicyInput.h"
#include "model/bucket/PutBucketMirrorBackInput.h"
#include "model/bucket/PutBucketMirrorBackOutput.h"
#include "model/bucket/GetBucketMirrorBackInput.h"
#include "model/bucket/GetBucketMirrorBackOutput.h"
#include "model/bucket/DeleteBucketMirrorBackInput.h"
#include "model/bucket/DeleteBucketMirrorBackOutput.h"
#include "model/object/PutObjectTaggingInput.h"
#include "model/object/PutObjectTaggingOutput.h"
#include "model/object/GetObjectTaggingInput.h"
#include "model/object/GetObjectTaggingOutput.h"
#include "model/object/DeleteObjectTaggingInput.h"
#include "model/object/DeleteObjectTaggingOutput.h"
#include "model/acl/PutBucketAclInput.h"
#include "model/acl/PutBucketAclOutput.h"
#include "model/acl/GetBucketAclInput.h"
#include "model/acl/GetBucketAclOutput.h"
#include "model/object/FetchObjectInput.h"
#include "model/object/FetchObjectOutput.h"
#include "model/object/PutFetchTaskInput.h"
#include "model/object/PutFetchTaskOutput.h"
#include "model/acl/PreSignedPostSignatureInput.h"
#include "model/acl/PreSignedPostSignatureOutput.h"
#include "SchemeHostParameter.h"
#include "model/bucket/PutBucketReplicationInput.h"
#include "model/bucket/PutBucketReplicationOutput.h"
#include "model/bucket/GetBucketReplicationInput.h"
#include "model/bucket/GetBucketReplicationOutput.h"
#include "model/bucket/DeleteBucketReplicationInput.h"
#include "model/bucket/DeleteBucketReplicationOutput.h"
#include "model/bucket/PutBucketVersioningInput.h"
#include "model/bucket/PutBucketVersioningOutput.h"
#include "model/bucket/GetBucketVersioningInput.h"
#include "model/bucket/GetBucketVersioningOutput.h"
#include "model/bucket/PutBucketWebsiteInput.h"
#include "model/bucket/PutBucketWebsiteOutput.h"
#include "model/bucket/GetBucketWebsiteInput.h"
#include "model/bucket/GetBucketWebsiteOutput.h"
#include "model/bucket/DeleteBucketWebsiteInput.h"
#include "model/bucket/DeleteBucketWebsiteOutput.h"
#include "model/bucket/PutBucketCustomDomainInput.h"
#include "model/bucket/PutBucketCustomDomainOutput.h"
#include "model/bucket/ListBucketCustomDomainInput.h"
#include "model/bucket/ListBucketCustomDomainOutput.h"
#include "model/bucket/DeleteBucketCustomDomainInput.h"
#include "model/bucket/DeleteBucketCustomDomainOutput.h"
#include "model/bucket/PutBucketNotificationInput.h"
#include "model/bucket/PutBucketNotificationOutput.h"
#include "model/bucket/GetBucketNotificationInput.h"
#include "model/bucket/GetBucketNotificationOutput.h"
#include "model/bucket/PutBucketRealTimeLogInput.h"
#include "model/bucket/PutBucketRealTimeLogOutput.h"
#include "model/bucket/GetBucketRealTimeLogInput.h"
#include "model/bucket/GetBucketRealTimeLogOutput.h"
#include "model/bucket/DeleteBucketRealTimeLogInput.h"
#include "model/bucket/DeleteBucketRealTimeLogOutput.h"
#include "model/object/ResumableCopyObjectInput.h"
#include "model/object/ResumableCopyObjectOutput.h"
#include "model/acl/PreSignedPolicyURLOutput.h"
#include "model/acl/PreSignedPolicyURLInput.h"
#include "model/object/RenameObjectOutput.h"
#include "model/object/RenameObjectInput.h"
#include "model/object/RestoreObjectOutput.h"
#include "model/object/RestoreObjectInput.h"
#include "model/bucket/PutBucketRenameOutput.h"
#include "model/bucket/PutBucketRenameInput.h"
#include "model/bucket/GetBucketRenameOutput.h"
#include "model/bucket/GetBucketRenameInput.h"
#include "model/bucket/DeleteBucketRenameInput.h"
#include "model/bucket/DeleteBucketRenameOutput.h"
#include "auth/EnvCredentials.h"
#include "auth/EcsCredentials.h"

namespace VolcengineTos {

void InitializeClient();
void CloseClient();

class TosClientV2 : public TosClient {
public:
    using TosClient::abortMultipartUpload;
    using TosClient::appendObject;
    using TosClient::completeMultipartUpload;
    using TosClient::copyObject;
    using TosClient::copyObjectFrom;
    using TosClient::copyObjectTo;
    using TosClient::createBucket;
    using TosClient::createMultipartUpload;
    using TosClient::deleteBucket;
    using TosClient::deleteBucketPolicy;
    using TosClient::deleteMultiObjects;
    using TosClient::deleteObject;
    using TosClient::getBucketPolicy;
    using TosClient::getObject;
    using TosClient::getObjectAcl;
    using TosClient::headBucket;
    using TosClient::headObject;
    using TosClient::listBuckets;
    using TosClient::listMultipartUploads;
    using TosClient::listObjects;
    using TosClient::listObjectVersions;
    using TosClient::listUploadedParts;
    using TosClient::preSignedURL;
    using TosClient::putBucketPolicy;
    using TosClient::putObject;
    using TosClient::putObjectAcl;
    using TosClient::setObjectMeta;
    using TosClient::uploadFile;
    using TosClient::uploadPart;
    using TosClient::uploadPartCopy;

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

    // Credentials
    TosClientV2(const std::string& region, const std::shared_ptr<Credentials>& cred);
    TosClientV2(const std::string& region, const std::shared_ptr<Credentials>& cred, const ClientConfig& config);

    ~TosClientV2() override = default;

    Outcome<TosError, CreateBucketV2Output> createBucket(const CreateBucketV2Input& input) const;
    Outcome<TosError, HeadBucketV2Output> headBucket(const HeadBucketV2Input& input) const;
    Outcome<TosError, DeleteBucketOutput> deleteBucket(const DeleteBucketInput& input) const;

    Outcome<TosError, GetObjectV2Output> getObject(const GetObjectV2Input& input) const;

    Outcome<TosError, GetObjectToFileOutput> getObjectToFile(const GetObjectToFileInput& input) const;

    Outcome<TosError, HeadObjectV2Output> headObject(const HeadObjectV2Input& input) const;

    Outcome<TosError, CopyObjectV2Output> copyObject(const CopyObjectV2Input& input) const;

    Outcome<TosError, DeleteObjectOutput> deleteObject(const DeleteObjectInput& input) const;

    Outcome<TosError, DeleteMultiObjectsOutput> deleteMultiObjects(DeleteMultiObjectsInput& input) const;

    Outcome<TosError, PutObjectV2Output> putObject(const PutObjectV2Input& input) const;
    Outcome<TosError, PutObjectFromFileOutput> putObjectFromFile(const PutObjectFromFileInput& input) const;

    Outcome<TosError, AppendObjectV2Output> appendObject(const AppendObjectV2Input& input) const;

    Outcome<TosError, SetObjectMetaOutput> setObjectMeta(const SetObjectMetaInput& input) const;

    Outcome<TosError, ListObjectsV2Output> listObjects(const ListObjectsV2Input& input) const;

    Outcome<TosError, ListObjectVersionsV2Output> listObjectVersions(const ListObjectVersionsV2Input& input) const;

    Outcome<TosError, PutObjectAclV2Output> putObjectAcl(const PutObjectAclV2Input& input) const;

    Outcome<TosError, GetObjectAclV2Output> getObjectAcl(const GetObjectAclV2Input& input) const;

    Outcome<TosError, CreateMultipartUploadOutput> createMultipartUpload(const CreateMultipartUploadInput& input) const;

    Outcome<TosError, UploadPartV2Output> uploadPart(const UploadPartV2Input& input) const;

    Outcome<TosError, UploadPartFromFileOutput> uploadPartFromFile(const UploadPartFromFileInput& input) const;

    Outcome<TosError, CompleteMultipartUploadV2Output> completeMultipartUpload(
            CompleteMultipartUploadV2Input& input) const;

    Outcome<TosError, AbortMultipartUploadOutput> abortMultipartUpload(const AbortMultipartUploadInput& input) const;
    Outcome<TosError, UploadPartCopyV2Output> uploadPartCopy(const UploadPartCopyV2Input& input) const;

    Outcome<TosError, ListPartsOutput> listParts(const ListPartsInput& input) const;

    Outcome<TosError, ListMultipartUploadsV2Output> listMultipartUploads(
            const ListMultipartUploadsV2Input& input) const;

    Outcome<TosError, UploadFileV2Output> uploadFile(const UploadFileV2Input& input) const;
    Outcome<TosError, DownloadFileOutput> downloadFile(const DownloadFileInput& input) const;

    Outcome<TosError, PreSignedURLOutput> preSignedURL(const PreSignedURLInput& input) const;

    // 2.4.0
    Outcome<TosError, PutBucketCORSOutput> putBucketCORS(const PutBucketCORSInput& input) const;
    Outcome<TosError, GetBucketCORSOutput> getBucketCORS(const GetBucketCORSInput& input) const;
    Outcome<TosError, DeleteBucketCORSOutput> deleteBucketCORS(const DeleteBucketCORSInput& input) const;
    Outcome<TosError, ListObjectsType2Output> listObjectsType2(const ListObjectsType2Input& input) const;
    Outcome<TosError, PutBucketStorageClassOutput> putBucketStorageClass(const PutBucketStorageClassInput& input) const;
    Outcome<TosError, GetBucketLocationOutput> getBucketLocation(const GetBucketLocationInput& input) const;
    Outcome<TosError, PutBucketLifecycleOutput> putBucketLifecycle(const PutBucketLifecycleInput& input) const;
    Outcome<TosError, GetBucketLifecycleOutput> getBucketLifecycle(const GetBucketLifecycleInput& input) const;
    Outcome<TosError, DeleteBucketLifecycleOutput> deleteBucketLifecycle(const DeleteBucketLifecycleInput& input) const;
    Outcome<TosError, PutBucketPolicyOutput> putBucketPolicy(const PutBucketPolicyInput& input) const;
    Outcome<TosError, GetBucketPolicyOutput> getBucketPolicy(const GetBucketPolicyInput& input) const;
    Outcome<TosError, DeleteBucketPolicyOutput> deleteBucketPolicy(const DeleteBucketPolicyInput& input) const;
    Outcome<TosError, PutBucketMirrorBackOutput> putBucketMirrorBack(const PutBucketMirrorBackInput& input) const;
    Outcome<TosError, GetBucketMirrorBackOutput> getBucketMirrorBack(const GetBucketMirrorBackInput& input) const;
    Outcome<TosError, DeleteBucketMirrorBackOutput> deleteBucketMirrorBack(
            const DeleteBucketMirrorBackInput& input) const;
    Outcome<TosError, PutObjectTaggingOutput> putObjectTagging(const PutObjectTaggingInput& input) const;
    Outcome<TosError, GetObjectTaggingOutput> getObjectTagging(const GetObjectTaggingInput& input) const;
    Outcome<TosError, DeleteObjectTaggingOutput> deleteObjectTagging(const DeleteObjectTaggingInput& input) const;
    Outcome<TosError, PutBucketAclOutput> putBucketAcl(const PutBucketAclInput& input) const;
    Outcome<TosError, GetBucketAclOutput> getBucketAcl(const GetBucketAclInput& input) const;
    Outcome<TosError, FetchObjectOutput> fetchObject(const FetchObjectInput& input) const;
    Outcome<TosError, PutFetchTaskOutput> putFetchTask(const PutFetchTaskInput& input) const;
    Outcome<TosError, PreSignedPostSignatureOutput> preSignedPostSignature(
            const PreSignedPostSignatureInput& input) const;

    // 2.5.0
    Outcome<TosError, PutBucketReplicationOutput> putBucketReplication(const PutBucketReplicationInput& input) const;
    Outcome<TosError, GetBucketReplicationOutput> getBucketReplication(const GetBucketReplicationInput& input) const;
    Outcome<TosError, DeleteBucketReplicationOutput> deleteBucketReplication(
            const DeleteBucketReplicationInput& input) const;
    Outcome<TosError, PutBucketVersioningOutput> putBucketVersioning(const PutBucketVersioningInput& input) const;
    Outcome<TosError, GetBucketVersioningOutput> getBucketVersioning(const GetBucketVersioningInput& input) const;
    Outcome<TosError, PutBucketWebsiteOutput> putBucketWebsite(const PutBucketWebsiteInput& input) const;
    Outcome<TosError, GetBucketWebsiteOutput> getBucketWebsite(const GetBucketWebsiteInput& input) const;
    Outcome<TosError, DeleteBucketWebsiteOutput> deleteBucketWebsite(const DeleteBucketWebsiteInput& input) const;
    Outcome<TosError, PutBucketCustomDomainOutput> putBucketCustomDomain(const PutBucketCustomDomainInput& input) const;
    Outcome<TosError, ListBucketCustomDomainOutput> listBucketCustomDomain(
            const ListBucketCustomDomainInput& input) const;
    Outcome<TosError, DeleteBucketCustomDomainOutput> deleteBucketCustomDomain(
            const DeleteBucketCustomDomainInput& input) const;
    Outcome<TosError, PutBucketNotificationOutput> putBucketNotification(const PutBucketNotificationInput& input) const;
    Outcome<TosError, GetBucketNotificationOutput> getBucketNotification(const GetBucketNotificationInput& input) const;
    Outcome<TosError, PutBucketRealTimeLogOutput> putBucketRealTimeLog(const PutBucketRealTimeLogInput& input) const;
    Outcome<TosError, GetBucketRealTimeLogOutput> getBucketRealTimeLog(const GetBucketRealTimeLogInput& input) const;
    Outcome<TosError, DeleteBucketRealTimeLogOutput> deleteBucketRealTimeLog(
            const DeleteBucketRealTimeLogInput& input) const;

    Outcome<TosError, ResumableCopyObjectOutput> resumableCopyObject(const ResumableCopyObjectInput& input) const;
    Outcome<TosError, PreSignedPolicyURLOutput> preSignedPolicyURL(const PreSignedPolicyURLInput& input) const;

    // 2.6.0
    Outcome<TosError, RenameObjectOutput> renameObject(const RenameObjectInput& input);
    Outcome<TosError, RestoreObjectOutput> restoreObject(const RestoreObjectInput& input);
    Outcome<TosError, PutBucketRenameOutput> putBucketRename(const PutBucketRenameInput& input);
    Outcome<TosError, GetBucketRenameOutput> getBucketRename(const GetBucketRenameInput& input);
    Outcome<TosError, DeleteBucketRenameOutput> deleteBucketRename(const DeleteBucketRenameInput& input);

private:
    std::shared_ptr<TosClientImpl> tosClientImpl_;
};
}  // namespace VolcengineTos
