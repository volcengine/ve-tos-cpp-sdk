#include <utility>
#include <memory>
#include <curl/curl.h>
#include "TosClientV2.h"
#include "TosClientImpl.h"
#include "transport/http/HttpClient.h"
#include "utils/LogUtils.h"
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

TosClientV2::TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId)
        : TosClientV2(region, StaticCredentials(accessKeyId, secretKeyId)) {
}
TosClientV2::TosClientV2(const std::string& region, const StaticCredentials& cred)
        : TosClient("", region, cred), tosClientImpl_(std::make_shared<TosClientImpl>("", region, cred)) {
}

TosClientV2::TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                         const std::string& securityToken)
        : TosClientV2(region, StaticCredentials(accessKeyId, secretKeyId, securityToken)) {
}
TosClientV2::TosClientV2(const std::string& region, const FederationCredentials& cred)
        : TosClient("", region, cred), tosClientImpl_(std::make_shared<TosClientImpl>("", region, cred)) {
}

TosClientV2::TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                         const ClientConfig& config)
        : TosClientV2(region, StaticCredentials(accessKeyId, secretKeyId), config) {
}
TosClientV2::TosClientV2(const std::string& region, const StaticCredentials& cred, const ClientConfig& config)
        : TosClient(config.endPoint, region, cred, config),
          tosClientImpl_(std::make_shared<TosClientImpl>(config.endPoint, region, cred, config)) {
}

TosClientV2::TosClientV2(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                         const std::string& securityToken, const ClientConfig& config)
        : TosClientV2(region, StaticCredentials(accessKeyId, secretKeyId, securityToken), config) {
}
TosClientV2::TosClientV2(const std::string& region, const FederationCredentials& cred, const ClientConfig& config)
        : TosClient(config.endPoint, region, cred, config),
          tosClientImpl_(std::make_shared<TosClientImpl>(config.endPoint, region, cred, config)) {
}

Outcome<TosError, CreateBucketV2Output> TosClientV2::createBucket(const CreateBucketV2Input& input) const {
    return tosClientImpl_->createBucket(input);
}
Outcome<TosError, HeadBucketV2Output> TosClientV2::headBucket(const HeadBucketV2Input& input) const {
    return tosClientImpl_->headBucket(input);
}
Outcome<TosError, DeleteBucketOutput> TosClientV2::deleteBucket(const DeleteBucketInput& input) const {
    return tosClientImpl_->deleteBucket(input);
}

Outcome<TosError, GetObjectV2Output> TosClientV2::getObject(const GetObjectV2Input& input) const {
    return tosClientImpl_->getObject(input, nullptr, nullptr);
}
Outcome<TosError, GetObjectV2Output> TosClientV2::getObject(const GetObjectV2Input& input,
                                                            std::shared_ptr<std::iostream> resContent,
                                                            std::shared_ptr<DataConsumeCallBack> callBack) const {
    return tosClientImpl_->getObject(input, nullptr, std::move(resContent));
}
Outcome<TosError, GetObjectToFileOutput> TosClientV2::getObjectToFile(const GetObjectToFileInput& input) const {
    return tosClientImpl_->getObjectToFile(input);
}
Outcome<TosError, HeadObjectV2Output> TosClientV2::headObject(const HeadObjectV2Input& input) const {
    return tosClientImpl_->headObject(input);
}
Outcome<TosError, DeleteObjectOutput> TosClientV2::deleteObject(const DeleteObjectInput& input) const {
    return tosClientImpl_->deleteObject(input);
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClientV2::deleteMultiObjects(DeleteMultiObjectsInput& input) const {
    return tosClientImpl_->deleteMultiObjects(input);
}
Outcome<TosError, PutObjectV2Output> TosClientV2::putObject(const PutObjectV2Input& input) const {
    return tosClientImpl_->putObject(input);
}
Outcome<TosError, PutObjectFromFileOutput> TosClientV2::putObjectFromFile(const PutObjectFromFileInput& input) const {
    return tosClientImpl_->putObjectFromFile(input);
}
Outcome<TosError, CopyObjectV2Output> TosClientV2::copyObject(const CopyObjectV2Input& input) const {
    return tosClientImpl_->copyObject(input);
}
Outcome<TosError, AppendObjectV2Output> TosClientV2::appendObject(const AppendObjectV2Input& input) const {
    return tosClientImpl_->appendObject(input);
}
Outcome<TosError, SetObjectMetaOutput> TosClientV2::setObjectMeta(const SetObjectMetaInput& input) const {
    return tosClientImpl_->setObjectMeta(input);
}
Outcome<TosError, ListObjectsV2Output> TosClientV2::listObjects(const ListObjectsV2Input& input) const {
    return tosClientImpl_->listObjects(input);
}
Outcome<TosError, ListObjectVersionsV2Output> TosClientV2::listObjectVersions(
        const ListObjectVersionsV2Input& input) const {
    return tosClientImpl_->listObjectVersions(input);
}
Outcome<TosError, PutObjectAclV2Output> TosClientV2::putObjectAcl(const PutObjectAclV2Input& input) const {
    return tosClientImpl_->putObjectAcl(input);
}
Outcome<TosError, GetObjectAclV2Output> TosClientV2::getObjectAcl(const GetObjectAclV2Input& input) const {
    return tosClientImpl_->getObjectAcl(input);
}
Outcome<TosError, CreateMultipartUploadOutput> TosClientV2::createMultipartUpload(
        const CreateMultipartUploadInput& input) const {
    return tosClientImpl_->createMultipartUpload(input);
}
Outcome<TosError, UploadPartV2Output> TosClientV2::uploadPart(const UploadPartV2Input& input) const {
    return tosClientImpl_->uploadPart(input, nullptr);
}
Outcome<TosError, UploadPartFromFileOutput> TosClientV2::uploadPartFromFile(
        const UploadPartFromFileInput& input) const {
    return tosClientImpl_->uploadPartFromFile(input, nullptr);
}
Outcome<TosError, CompleteMultipartUploadV2Output> TosClientV2::completeMultipartUpload(
        CompleteMultipartUploadV2Input& input) const {
    return tosClientImpl_->completeMultipartUpload(input);
}
Outcome<TosError, AbortMultipartUploadOutput> TosClientV2::abortMultipartUpload(
        const AbortMultipartUploadInput& input) const {
    return tosClientImpl_->abortMultipartUpload(input);
}

Outcome<TosError, UploadPartCopyV2Output> TosClientV2::uploadPartCopy(const UploadPartCopyV2Input& input) const {
    return tosClientImpl_->uploadPartCopy(input);
}
Outcome<TosError, ListPartsOutput> TosClientV2::listParts(const ListPartsInput& input) const {
    return tosClientImpl_->listParts(input);
}
Outcome<TosError, ListMultipartUploadsV2Output> TosClientV2::listMultipartUploads(
        const ListMultipartUploadsV2Input& input) const {
    return tosClientImpl_->listMultipartUploads(input);
}
Outcome<TosError, UploadFileV2Output> TosClientV2::uploadFile(const UploadFileV2Input& input) const {
    return tosClientImpl_->uploadFile(input);
}

Outcome<TosError, DownloadFileOutput> TosClientV2::downloadFile(const DownloadFileInput& input) const {
    return tosClientImpl_->downloadFile(input);
}
Outcome<TosError, PreSignedURLOutput> TosClientV2::preSignedURL(const PreSignedURLInput& input) const {
    return tosClientImpl_->preSignedURL(input);
}

Outcome<TosError, PutBucketCORSOutput> TosClientV2::putBucketCORS(const PutBucketCORSInput& input) const {
    return tosClientImpl_->putBucketCORS(input);
}
Outcome<TosError, GetBucketCORSOutput> TosClientV2::getBucketCORS(const GetBucketCORSInput& input) const {
    return tosClientImpl_->getBucketCORS(input);
}
Outcome<TosError, DeleteBucketCORSOutput> TosClientV2::deleteBucketCORS(const DeleteBucketCORSInput& input) const {
    return tosClientImpl_->deleteBucketCORS(input);
}
Outcome<TosError, ListObjectsType2Output> TosClientV2::listObjectsType2(const ListObjectsType2Input& input) const {
    return tosClientImpl_->listObjectsType2(input);
}
Outcome<TosError, PutBucketStorageClassOutput> TosClientV2::putBucketStorageClass(
        const PutBucketStorageClassInput& input) const {
    return tosClientImpl_->putBucketStorageClass(input);
}
Outcome<TosError, GetBucketLocationOutput> TosClientV2::getBucketLocation(const GetBucketLocationInput& input) const {
    return tosClientImpl_->getBucketLocation(input);
}
Outcome<TosError, PutBucketLifecycleOutput> TosClientV2::putBucketLifecycle(
        const PutBucketLifecycleInput& input) const {
    return tosClientImpl_->putBucketLifecycle(input);
}
Outcome<TosError, GetBucketLifecycleOutput> TosClientV2::getBucketLifecycle(
        const GetBucketLifecycleInput& input) const {
    return tosClientImpl_->getBucketLifecycle(input);
}
Outcome<TosError, DeleteBucketLifecycleOutput> TosClientV2::deleteBucketLifecycle(
        const DeleteBucketLifecycleInput& input) const {
    return tosClientImpl_->deleteBucketLifecycle(input);
}
Outcome<TosError, PutBucketPolicyOutput> TosClientV2::putBucketPolicy(const PutBucketPolicyInput& input) const {
    return tosClientImpl_->putBucketPolicy(input);
}
Outcome<TosError, GetBucketPolicyOutput> TosClientV2::getBucketPolicy(const GetBucketPolicyInput& input) const {
    return tosClientImpl_->getBucketPolicy(input);
}
Outcome<TosError, DeleteBucketPolicyOutput> TosClientV2::deleteBucketPolicy(
        const DeleteBucketPolicyInput& input) const {
    return tosClientImpl_->deleteBucketPolicy(input);
}
Outcome<TosError, PutBucketMirrorBackOutput> TosClientV2::putBucketMirrorBack(
        const PutBucketMirrorBackInput& input) const {
    return tosClientImpl_->putBucketMirrorBack(input);
}
Outcome<TosError, GetBucketMirrorBackOutput> TosClientV2::getBucketMirrorBack(
        const GetBucketMirrorBackInput& input) const {
    return tosClientImpl_->getBucketMirrorBack(input);
}
Outcome<TosError, DeleteBucketMirrorBackOutput> TosClientV2::deleteBucketMirrorBack(
        const DeleteBucketMirrorBackInput& input) const {
    return tosClientImpl_->deleteBucketMirrorBack(input);
}
Outcome<TosError, PutObjectTaggingOutput> TosClientV2::putObjectTagging(const PutObjectTaggingInput& input) const {
    return tosClientImpl_->putObjectTagging(input);
}
Outcome<TosError, GetObjectTaggingOutput> TosClientV2::getObjectTagging(const GetObjectTaggingInput& input) const {
    return tosClientImpl_->getObjectTagging(input);
}
Outcome<TosError, DeleteObjectTaggingOutput> TosClientV2::deleteObjectTagging(
        const DeleteObjectTaggingInput& input) const {
    return tosClientImpl_->deleteObjectTagging(input);
}
Outcome<TosError, PutBucketAclOutput> TosClientV2::putBucketAcl(const PutBucketAclInput& input) const {
    return tosClientImpl_->putBucketAcl(input);
}
Outcome<TosError, GetBucketAclOutput> TosClientV2::getBucketAcl(const GetBucketAclInput& input) const {
    return tosClientImpl_->getBucketAcl(input);
}
Outcome<TosError, FetchObjectOutput> TosClientV2::fetchObject(const FetchObjectInput& input) const {
    return tosClientImpl_->fetchObject(input);
}
Outcome<TosError, PutFetchTaskOutput> TosClientV2::putFetchTask(const PutFetchTaskInput& input) const {
    return tosClientImpl_->putFetchTask(input);
}
Outcome<TosError, PreSignedPostSignatureOutput> TosClientV2::preSignedPostSignature(
        const PreSignedPostSignatureInput& input) const {
    return tosClientImpl_->preSignedPostSignature(input);
}
Outcome<TosError, PutBucketReplicationOutput> TosClientV2::putBucketReplication(
        const PutBucketReplicationInput& input) const {
    return tosClientImpl_->putBucketReplication(input);
}
Outcome<TosError, GetBucketReplicationOutput> TosClientV2::getBucketReplication(
        const GetBucketReplicationInput& input) const {
    return tosClientImpl_->getBucketReplication(input);
}
Outcome<TosError, DeleteBucketReplicationOutput> TosClientV2::deleteBucketReplication(
        const DeleteBucketReplicationInput& input) const {
    return tosClientImpl_->deleteBucketReplication(input);
}
Outcome<TosError, PutBucketVersioningOutput> TosClientV2::putBucketVersioning(
        const PutBucketVersioningInput& input) const {
    return tosClientImpl_->putBucketVersioning(input);
}
Outcome<TosError, GetBucketVersioningOutput> TosClientV2::getBucketVersioning(
        const GetBucketVersioningInput& input) const {
    return tosClientImpl_->getBucketVersioning(input);
}
Outcome<TosError, PutBucketWebsiteOutput> TosClientV2::putBucketWebsite(const PutBucketWebsiteInput& input) const {
    return tosClientImpl_->putBucketWebsite(input);
}
Outcome<TosError, GetBucketWebsiteOutput> TosClientV2::getBucketWebsite(const GetBucketWebsiteInput& input) const {
    return tosClientImpl_->getBucketWebsite(input);
}
Outcome<TosError, DeleteBucketWebsiteOutput> TosClientV2::deleteBucketWebsite(
        const DeleteBucketWebsiteInput& input) const {
    return tosClientImpl_->deleteBucketWebsite(input);
}
Outcome<TosError, PutBucketNotificationOutput> TosClientV2::putBucketNotification(
        const PutBucketNotificationInput& input) const {
    return tosClientImpl_->putBucketNotification(input);
}
Outcome<TosError, GetBucketNotificationOutput> TosClientV2::getBucketNotification(
        const GetBucketNotificationInput& input) const {
    return tosClientImpl_->getBucketNotification(input);
}
Outcome<TosError, PutBucketCustomDomainOutput> TosClientV2::putBucketCustomDomain(
        const PutBucketCustomDomainInput& input) const {
    return tosClientImpl_->putBucketCustomDomain(input);
}
Outcome<TosError, ListBucketCustomDomainOutput> TosClientV2::listBucketCustomDomain(
        const ListBucketCustomDomainInput& input) const {
    return tosClientImpl_->listBucketCustomDomain(input);
}
Outcome<TosError, DeleteBucketCustomDomainOutput> TosClientV2::deleteBucketCustomDomain(
        const DeleteBucketCustomDomainInput& input) const {
    return tosClientImpl_->deleteBucketCustomDomain(input);
}
Outcome<TosError, PutBucketRealTimeLogOutput> TosClientV2::putBucketRealTimeLog(
        const PutBucketRealTimeLogInput& input) const {
    return tosClientImpl_->putBucketRealTimeLog(input);
}
Outcome<TosError, GetBucketRealTimeLogOutput> TosClientV2::getBucketRealTimeLog(
        const GetBucketRealTimeLogInput& input) const {
    return tosClientImpl_->getBucketRealTimeLog(input);
}
Outcome<TosError, DeleteBucketRealTimeLogOutput> TosClientV2::deleteBucketRealTimeLog(
        const DeleteBucketRealTimeLogInput& input) const {
    return tosClientImpl_->deleteBucketRealTimeLog(input);
}

Outcome<TosError, ResumableCopyObjectOutput> TosClientV2::resumableCopyObject(
        const ResumableCopyObjectInput& input) const {
    return tosClientImpl_->resumableCopyObject(input);
}
Outcome<TosError, PreSignedPolicyURLOutput> TosClientV2::preSignedPolicyURL(
        const PreSignedPolicyURLInput& input) const {
    return tosClientImpl_->preSignedPolicyURL(input);
}
