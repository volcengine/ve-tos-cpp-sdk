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

Outcome<TosError, CreateBucketV2Output> TosClientV2::createBucket(const CreateBucketV2Input& input) {
    return tosClientImpl_->createBucket(input);
}
Outcome<TosError, HeadBucketV2Output> TosClientV2::headBucket(const HeadBucketV2Input& input) {
    return tosClientImpl_->headBucket(input);
}
Outcome<TosError, DeleteBucketOutput> TosClientV2::deleteBucket(const DeleteBucketInput& input) {
    return tosClientImpl_->deleteBucket(input);
}

Outcome<TosError, GetObjectV2Output> TosClientV2::getObject(const GetObjectV2Input& input) {
    return tosClientImpl_->getObject(input, nullptr);
}
Outcome<TosError, GetObjectToFileOutput> TosClientV2::getObjectToFile(const GetObjectToFileInput& input) {
    return tosClientImpl_->getObjectToFile(input);
}
Outcome<TosError, HeadObjectV2Output> TosClientV2::headObject(const HeadObjectV2Input& input) {
    return tosClientImpl_->headObject(input);
}
Outcome<TosError, DeleteObjectOutput> TosClientV2::deleteObject(const DeleteObjectInput& input) {
    return tosClientImpl_->deleteObject(input);
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClientV2::deleteMultiObjects(DeleteMultiObjectsInput& input) {
    return tosClientImpl_->deleteMultiObjects(input);
}
Outcome<TosError, PutObjectV2Output> TosClientV2::putObject(const PutObjectV2Input& input) {
    return tosClientImpl_->putObject(input);
}
Outcome<TosError, PutObjectFromFileOutput> TosClientV2::putObjectFromFile(const PutObjectFromFileInput& input) {
    return tosClientImpl_->putObjectFromFile(input);
}
Outcome<TosError, CopyObjectV2Output> TosClientV2::copyObject(const CopyObjectV2Input& input) {
    return tosClientImpl_->copyObject(input);
}
Outcome<TosError, AppendObjectV2Output> TosClientV2::appendObject(const AppendObjectV2Input& input) {
    return tosClientImpl_->appendObject(input);
}
Outcome<TosError, SetObjectMetaOutput> TosClientV2::setObjectMeta(const SetObjectMetaInput& input) {
    return tosClientImpl_->setObjectMeta(input);
}
Outcome<TosError, ListObjectsV2Output> TosClientV2::listObjects(const ListObjectsV2Input& input) {
    return tosClientImpl_->listObjects(input);
}
Outcome<TosError, ListObjectVersionsV2Output> TosClientV2::listObjectVersions(const ListObjectVersionsV2Input& input) {
    return tosClientImpl_->listObjectVersions(input);
}
Outcome<TosError, PutObjectAclV2Output> TosClientV2::putObjectAcl(const PutObjectAclV2Input& input) {
    return tosClientImpl_->putObjectAcl(input);
}
Outcome<TosError, GetObjectAclV2Output> TosClientV2::getObjectAcl(const GetObjectAclV2Input& input) {
    return tosClientImpl_->getObjectAcl(input);
}
Outcome<TosError, CreateMultipartUploadOutput> TosClientV2::createMultipartUpload(
        const CreateMultipartUploadInput& input) {
    return tosClientImpl_->createMultipartUpload(input);
}
Outcome<TosError, UploadPartV2Output> TosClientV2::uploadPart(const UploadPartV2Input& input) {
    return tosClientImpl_->uploadPart(input, nullptr);
}
Outcome<TosError, UploadPartFromFileOutput> TosClientV2::uploadPartFromFile(const UploadPartFromFileInput& input) {
    return tosClientImpl_->uploadPartFromFile(input, nullptr);
}
Outcome<TosError, CompleteMultipartUploadV2Output> TosClientV2::completeMultipartUpload(
        CompleteMultipartUploadV2Input& input) {
    return tosClientImpl_->completeMultipartUpload(input);
}
Outcome<TosError, AbortMultipartUploadOutput> TosClientV2::abortMultipartUpload(
        const AbortMultipartUploadInput& input) {
    return tosClientImpl_->abortMultipartUpload(input);
}

Outcome<TosError, UploadPartCopyV2Output> TosClientV2::uploadPartCopy(const UploadPartCopyV2Input& input) {
    return tosClientImpl_->uploadPartCopy(input);
}
Outcome<TosError, ListPartsOutput> TosClientV2::listParts(const ListPartsInput& input) {
    return tosClientImpl_->listParts(input);
}
Outcome<TosError, ListMultipartUploadsV2Output> TosClientV2::listMultipartUploads(
        const ListMultipartUploadsV2Input& input) {
    return tosClientImpl_->listMultipartUploads(input);
}
Outcome<TosError, UploadFileV2Output> TosClientV2::uploadFile(const UploadFileV2Input& input) {
    return tosClientImpl_->uploadFile(input);
}

Outcome<TosError, DownloadFileOutput> TosClientV2::downloadFile(const DownloadFileInput& input) {
    return tosClientImpl_->downloadFile(input);
}
Outcome<TosError, PreSignedURLOutput> TosClientV2::preSignedURL(const PreSignedURLInput& input) {
    return tosClientImpl_->preSignedURL(input);
}

Outcome<TosError, PutBucketCORSOutput> TosClientV2::putBucketCORS(const PutBucketCORSInput& input) {
    return tosClientImpl_->putBucketCORS(input);
}
Outcome<TosError, GetBucketCORSOutput> TosClientV2::getBucketCORS(const GetBucketCORSInput& input) {
    return tosClientImpl_->getBucketCORS(input);
}
Outcome<TosError, DeleteBucketCORSOutput> TosClientV2::deleteBucketCORS(const DeleteBucketCORSInput& input) {
    return tosClientImpl_->deleteBucketCORS(input);
}
Outcome<TosError, ListObjectsType2Output> TosClientV2::listObjectsType2(const ListObjectsType2Input& input) {
    return tosClientImpl_->listObjectsType2(input);
}
Outcome<TosError, PutBucketStorageClassOutput> TosClientV2::putBucketStorageClass(
        const PutBucketStorageClassInput& input) {
    return tosClientImpl_->putBucketStorageClass(input);
}
Outcome<TosError, GetBucketLocationOutput> TosClientV2::getBucketLocation(const GetBucketLocationInput& input) {
    return tosClientImpl_->getBucketLocation(input);
}
Outcome<TosError, PutBucketLifecycleOutput> TosClientV2::putBucketLifecycle(const PutBucketLifecycleInput& input) {
    return tosClientImpl_->putBucketLifecycle(input);
}
Outcome<TosError, GetBucketLifecycleOutput> TosClientV2::getBucketLifecycle(const GetBucketLifecycleInput& input) {
    return tosClientImpl_->getBucketLifecycle(input);
}
Outcome<TosError, DeleteBucketLifecycleOutput> TosClientV2::deleteBucketLifecycle(
        const DeleteBucketLifecycleInput& input) {
    return tosClientImpl_->deleteBucketLifecycle(input);
}
Outcome<TosError, PutBucketPolicyOutput> TosClientV2::putBucketPolicy(const PutBucketPolicyInput& input) {
    return tosClientImpl_->putBucketPolicy(input);
}
Outcome<TosError, GetBucketPolicyOutput> TosClientV2::getBucketPolicy(const GetBucketPolicyInput& input) {
    return tosClientImpl_->getBucketPolicy(input);
}
Outcome<TosError, DeleteBucketPolicyOutput> TosClientV2::deleteBucketPolicy(const DeleteBucketPolicyInput& input) {
    return tosClientImpl_->deleteBucketPolicy(input);
}
Outcome<TosError, PutBucketMirrorBackOutput> TosClientV2::putBucketMirrorBack(const PutBucketMirrorBackInput& input) {
    return tosClientImpl_->putBucketMirrorBack(input);
}
Outcome<TosError, GetBucketMirrorBackOutput> TosClientV2::getBucketMirrorBack(const GetBucketMirrorBackInput& input) {
    return tosClientImpl_->getBucketMirrorBack(input);
}
Outcome<TosError, DeleteBucketMirrorBackOutput> TosClientV2::deleteBucketMirrorBack(
        const DeleteBucketMirrorBackInput& input) {
    return tosClientImpl_->deleteBucketMirrorBack(input);
}
Outcome<TosError, PutObjectTaggingOutput> TosClientV2::putObjectTagging(const PutObjectTaggingInput& input) {
    return tosClientImpl_->putObjectTagging(input);
}
Outcome<TosError, GetObjectTaggingOutput> TosClientV2::getObjectTagging(const GetObjectTaggingInput& input) {
    return tosClientImpl_->getObjectTagging(input);
}
Outcome<TosError, DeleteObjectTaggingOutput> TosClientV2::deleteObjectTagging(const DeleteObjectTaggingInput& input) {
    return tosClientImpl_->deleteObjectTagging(input);
}
Outcome<TosError, PutBucketAclOutput> TosClientV2::putBucketAcl(const PutBucketAclInput& input) {
    return tosClientImpl_->putBucketAcl(input);
}
Outcome<TosError, GetBucketAclOutput> TosClientV2::getBucketAcl(const GetBucketAclInput& input) {
    return tosClientImpl_->getBucketAcl(input);
}
Outcome<TosError, FetchObjectOutput> TosClientV2::fetchObject(const FetchObjectInput& input) {
    return tosClientImpl_->fetchObject(input);
}
Outcome<TosError, PutFetchTaskOutput> TosClientV2::putFetchTask(const PutFetchTaskInput& input) {
    return tosClientImpl_->putFetchTask(input);
}
Outcome<TosError, PreSignedPostSignatureOutput> TosClientV2::preSignedPostSignature(
        const PreSignedPostSignatureInput& input) {
    return tosClientImpl_->preSignedPostSignature(input);
}