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
