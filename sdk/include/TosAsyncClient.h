//
// Created by ByteDance on 2025/10/20.
//
#pragma once
#include "TosClient.h"
#include "auth/StaticCredentials.h"
#include "model/async/bucket/input/CreateBucketAsyncInput.h"
#include "model/async/bucket/input/HeadBucketAsyncInput.h"
#include "model/async/bucket/input/ListBucketsAsyncInput.h"
#include "model/async/bucket/output/CreateBucketAsyncOutput.h"
#include "model/async/bucket/output/HeadBucketAsyncOutput.h"
#include "model/async/bucket/output/ListBucketsAsyncOutput.h"
#include "model/async/object/input/GetObjectAsyncInput.h"
#include "model/async/object/input/HeadObjectAsyncInput.h"
#include "model/async/bucket/input/ListObjectVersionsAsyncInput.h"
#include "model/async/bucket/input/ListObjectsAsyncInput.h"
#include "model/async/object/input/PutObjectAsyncInput.h"
#include "model/async/object/output/GetObjectAsyncOutput.h"
#include "model/async/object/output/HeadObjectAsyncOutput.h"
#include "model/async/bucket/output/ListObjectVersionsAsyncOutput.h"
#include "model/async/bucket/output/ListObjectsAsyncOutput.h"
#include "model/async/object/file/input/GetFileStatusAsyncInput.h"
#include "model/async/object/file/input/ModifyObjectAsyncInput.h"
#include "model/async/object/file/output/GetFileStatusAsyncOutput.h"
#include "model/async/object/file/output/ModifyObjectAsyncOutput.h"
#include "model/async/object/input/AppendObjectAsyncInput.h"
#include "model/async/object/input/CompleteMultipartUploadAsyncInput.h"
#include "model/async/object/input/CopyObjectAsyncInput.h"
#include "model/async/object/input/CreateMultipartUploadAsyncInput.h"
#include "model/async/object/input/DeleteObjectAsyncInput.h"
#include "model/async/object/input/GetObjectToFileAsyncInput.h"
#include "model/async/object/input/GetSymlinkAsyncInput.h"
#include "model/async/object/input/PutObjectFromFileAsyncInput.h"
#include "model/async/object/input/PutSymlinkAsyncInput.h"
#include "model/async/object/input/RenameObjectAsyncInput.h"
#include "model/async/object/input/SetObjectMetaAsyncInput.h"
#include "model/async/object/input/UploadPartAsyncInput.h"
#include "model/async/object/input/UploadPartCopyAsyncInput.h"
#include "model/async/object/output/AppendObjectAsyncOutput.h"
#include "model/async/object/output/CompleteMultipartUploadAsyncOutput.h"
#include "model/async/object/output/CopyObjectAsyncOutput.h"
#include "model/async/object/output/CreateMultipartUploadAsyncOutput.h"
#include "model/async/object/output/DeleteObjectAsyncOutput.h"
#include "model/async/object/output/GetObjectToFileAsyncOutput.h"
#include "model/async/object/output/GetSymlinkAsyncOutput.h"
#include "model/async/object/output/PutObjectAsyncOutput.h"
#include "model/async/object/output/PutObjectFromFileAsyncOutput.h"
#include "model/async/object/output/PutSymlinkAsyncOutput.h"
#include "model/async/object/output/RenameObjectAsyncOutput.h"
#include "model/async/object/output/SetObjectMetaAsyncOutput.h"
#include "model/async/object/output/UploadPartAsyncOutput.h"
#include "model/async/object/output/UploadPartCopyAsyncOutput.h"
#include "model/object/AbortMultipartUploadInput.h"
#include "model/object/AbortMultipartUploadOutput.h"
#include "model/object/ListObjectsType2Input.h"
#include "model/object/ListObjectsType2Output.h"
#include "model/object/SetObjectTimeInput.h"
#include "model/object/SetObjectTimeOutput.h"
#include "tos_async/FileTransfer.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace VolcengineTos {
class TosClientBase;
void InitializeTosAsyncClient();
void CloseTosAsyncClient();

class TosAsyncClient {
public:
    friend class GetObjectAsyncInput;

    static std::atomic<uint64_t> g_pipline_destroy_cnt;
    static std::atomic<uint64_t> g_pipline_create_cnt;

    TosAsyncClient(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId);
    TosAsyncClient(const std::string& region, const StaticCredentials& cred);
    TosAsyncClient(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                   const std::string& securityToken);
    TosAsyncClient(const std::string& region, const FederationCredentials& cred);
    TosAsyncClient(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                   const ClientConfig& config);
    TosAsyncClient(const std::string& region, const StaticCredentials& cred, const ClientConfig& config);
    TosAsyncClient(const std::string& region, const std::string& accessKeyId, const std::string& secretKeyId,
                   const std::string& securityToken, const ClientConfig& config);
    TosAsyncClient(const std::string& region, const FederationCredentials& cred, const ClientConfig& config);

    // Credentials
    TosAsyncClient(const std::string& region, const std::shared_ptr<Credentials>& cred);
    TosAsyncClient(const std::string& region, const std::shared_ptr<Credentials>& cred, const ClientConfig& config);

    ~TosAsyncClient();
    void close() const;

    void getObjectAsync(const GetObjectAsyncInput& input, const OnDataReceiveWithEvent& on_data_receive,
                        const std::function<void(Outcome<TosError, GetObjectAsyncOutput>&)>& on_request_done) const;
    void getObjectToFdRangeAsync(
            const GetObjectAsyncInput& input, FileRange range, FileTransferOptions options,
            const std::function<void(Outcome<TosError, GetObjectAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;

    void getObjectToFileAsync(
            const GetObjectToFileAsyncInput& input,
            const std::function<void(Outcome<TosError, GetObjectToFileAsyncOutput>&)>& on_request_done) const;

    void putObjectAsync(const PutObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
                        const std::function<void(Outcome<TosError, PutObjectAsyncOutput>&)>& on_request_done) const;
    void putObjectFromFdRangeAsync(
            const PutObjectAsyncInput& input, FileRange range, FileTransferOptions options,
            const std::function<void(Outcome<TosError, PutObjectAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;

    void putObjectFromFileAsync(
            const PutObjectFromFileAsyncInput& input,
            const std::function<void(Outcome<TosError, PutObjectFromFileAsyncOutput>&)>& on_request_done) const;

    void modifyObjectAsync(
            const ModifyObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
            const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&)>& on_request_done) const;
    void modifyObjectFromFdRangeAsync(
            const ModifyObjectAsyncInput& input, FileRange range, FileTransferOptions options,
            const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;
    void modifyObjectFromFileAsync(
            const ModifyObjectAsyncInput& input, const std::string& file_path, FileTransferOptions options,
            const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;

    void appendObjectAsync(
            const AppendObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
            const std::function<void(Outcome<TosError, AppendObjectAsyncOutput>&)>& on_request_done) const;
    void appendObjectFromFdRangeAsync(
            const AppendObjectAsyncInput& input, FileRange range, FileTransferOptions options,
            const std::function<void(Outcome<TosError, AppendObjectAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;

    void copyObjectAsync(const CopyObjectAsyncInput& input,
                         const std::function<void(Outcome<TosError, CopyObjectAsyncOutput>&)>& on_request_done) const;

    void headObjectAsync(const HeadObjectAsyncInput& input,
                         const std::function<void(Outcome<TosError, HeadObjectAsyncOutput>&)>& on_request_done) const;

    void deleteObjectAsync(
            const DeleteObjectAsyncInput& input,
            const std::function<void(Outcome<TosError, DeleteObjectAsyncOutput>&)>& on_request_done) const;

    void setObjectMetaAsync(
            const SetObjectMetaAsyncInput& input,
            const std::function<void(Outcome<TosError, SetObjectMetaAsyncOutput>&)>& on_request_done) const;

    void createBucketAsync(
            const CreateBucketAsyncInput& input,
            const std::function<void(Outcome<TosError, CreateBucketAsyncOutput>&)>& on_request_done) const;

    void headBucketAsync(const HeadBucketAsyncInput& input,
                         const std::function<void(Outcome<TosError, HeadBucketAsyncOutput>&)>& on_request_done) const;

    void getBucketTypeAsync(const std::string& bucketName,
                            const std::function<void(Outcome<TosError, BucketType>&)>& on_request_done) const;

    void listBucketsAsync(const ListBucketsAsyncInput& input,
                          const std::function<void(Outcome<TosError, ListBucketsAsyncOutput>&)>& on_request_done) const;

    void listObjectsAsync(const ListObjectsAsyncInput& input,
                          const std::function<void(Outcome<TosError, ListObjectsAsyncOutput>&)>& on_request_done) const;

    void listObjectsType2Async(
            const ListObjectsType2Input& input,
            const std::function<void(Outcome<TosError, ListObjectsType2Output>&)>& on_request_done) const;

    void listObjectVersionsAsync(
            const ListObjectVersionsAsyncInput& input,
            const std::function<void(Outcome<TosError, ListObjectVersionsAsyncOutput>&)>& on_request_done) const;

    void createMultipartUploadAsync(
            const CreateMultipartUploadAsyncInput& input,
            const std::function<void(Outcome<TosError, CreateMultipartUploadAsyncOutput>&)>& on_request_done) const;

    void abortMultipartUploadAsync(
            const AbortMultipartUploadInput& input,
            const std::function<void(Outcome<TosError, AbortMultipartUploadOutput>&)>& on_request_done) const;

    void setObjectTimeAsync(
            const SetObjectTimeInput& input,
            const std::function<void(Outcome<TosError, SetObjectTimeOutput>&)>& on_request_done) const;

    void uploadPartAsync(const UploadPartAsyncInput& input, const OnDataSendWithEvent& on_data_send,
                         const std::function<void(Outcome<TosError, UploadPartAsyncOutput>&)>& on_request_done) const;
    void uploadPartFromFdRangeAsync(
            const UploadPartAsyncInput& input, FileRange range, FileTransferOptions options,
            const std::function<void(Outcome<TosError, UploadPartAsyncOutput>&, FileTransferResult)>&
                    on_request_done) const;

    void completeMultipartUploadAsync(
            const CompleteMultipartUploadAsyncInput& input,
            const std::function<void(Outcome<TosError, CompleteMultipartUploadAsyncOutput>&)>& on_request_done) const;

    void uploadPartCopyUploadAsync(
            const UploadPartCopyAsyncInput& input,
            const std::function<void(Outcome<TosError, UploadPartCopyAsyncOutput>&)>& on_request_done) const;

    void putSymlinkAsync(const PutSymlinkAsyncInput& input,
                         const std::function<void(Outcome<TosError, PutSymlinkAsyncOutput>&)>& on_request_done) const;

    void getSymlinkAsync(const GetSymlinkAsyncInput& input,
                         const std::function<void(Outcome<TosError, GetSymlinkAsyncOutput>&)>& on_request_done) const;

    void renameObjectAsync(
            const RenameObjectAsyncInput& input,
            const std::function<void(Outcome<TosError, RenameObjectAsyncOutput>&)>& on_request_done) const;

    void getFileStatusAsync(
            const GetFileStatusAsyncInput& input,
            const std::function<void(Outcome<TosError, GetFileStatusAsyncOutput>&)>& on_request_done) const;

    std::shared_ptr<TosClientBase> getTosClient() const {
        return tosClientBase_;
    }

private:
    std::shared_ptr<TosClientBase> tosClientBase_;
    FileTransferOptions fileTransferOptions_;
};

}  // namespace VolcengineTos
