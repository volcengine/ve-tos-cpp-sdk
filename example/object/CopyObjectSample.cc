#include <iostream>
#include "CopyObjectSample.h"
#include <fstream>

using namespace VolcengineTos;

int CopyObjectSample::CopyObjectWithAcl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 destbucket
    std::string bucketName = "destbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，destdir/exampleobject.txt。
    std::string objectName = "destdir/exampleobject.txt";
    // 填写 Bucket 名称，例如 srcbucket
    std::string srcBucketName = "srcbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，srcdir/exampleobject.txt。
    std::string srcObjectName = "srcdir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    CopyObjectV2Input input(bucketName, objectName, srcBucketName, srcObjectName);
    // 设置 ACL 为 PublicRead
    input.setAcl(ACLType::PublicRead);
    input.setStorageClass(StorageClassType::STANDARD);
    input.setMetadataDirective(MetadataDirectiveType::REPLACE);
    auto output = client.copyObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "CopyObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "CopyObject success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int CopyObjectSample::CopyObjectWithSourceMeta() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 destbucket
    std::string bucketName = "destbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，destdir/exampleobject.txt。
    std::string objectName = "destdir/exampleobject.txt";
    // 填写 Bucket 名称，例如 srcbucket
    std::string srcBucketName = "srcbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，srcdir/exampleobject.txt。
    std::string srcObjectName = "srcdir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    CopyObjectV2Input input(bucketName, objectName, srcBucketName, srcObjectName);
    auto output = client.copyObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "CopyObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "CopyObject success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int CopyObjectSample::CopyObjectWithDestMeta() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 destbucket
    std::string bucketName = "destbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，destdir/exampleobject.txt。
    std::string objectName = "destdir/exampleobject.txt";
    // 填写 Bucket 名称，例如 srcbucket
    std::string srcBucketName = "srcbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，srcdir/exampleobject.txt。
    std::string srcObjectName = "srcdir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    CopyObjectV2Input input(bucketName, objectName, srcBucketName, srcObjectName);
    input.setMetadataDirective(MetadataDirectiveType::REPLACE);
    input.setMeta({{"self-test", "yes"}});
    auto output = client.copyObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "CopyObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "CopyObject success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int CopyObjectSample::UploadPartCopy() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 destbucket
    std::string bucketName = "destbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，destdir/exampleobject.txt。
    std::string objectName = "destdir/exampleobject.txt";
    // 填写 Bucket 名称，例如 srcbucket
    std::string srcBucketName = "srcbucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，srcdir/exampleobject.txt。
    std::string srcObjectName = "srcdir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    CreateMultipartUploadInput input_part_create(bucketName, objectName);
    auto upload = client.createMultipartUpload(input_part_create);
    if (!upload.isSuccess()) {
        std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
        return -1;
    }
    // 获取要copy的对象大小
    HeadObjectV2Input headInput(bucketName, objectName);
    auto headOutput = client.headObject(headInput);
    if (!headOutput.isSuccess()) {
        std::cout << "head object error: " << headOutput.error().String() << std::endl;
        return -1;
    }

    int64_t objectSize = headOutput.result().getContentLength();
    // 分片大小 5MB 为例
    int partSize = 5 * 1024 * 1024;
    int partCount = (int)(objectSize / partSize);
    std::vector<UploadedPartV2> copyParts(partCount);

    // 计算分片个数。
    if (objectSize % partSize != 0) {
        partCount++;
    }
    int64_t copySourceRangeEnd_ = 0;
    // 对每一个分片进行拷贝。
    for (int i = 1; i <= partCount; i++) {
        auto offset = partSize * (i - 1);
        auto size = (partSize < objectSize - offset) ? partSize : (objectSize - offset);
        UploadPartCopyV2Input input(bucketName, objectName, srcBucketName, srcObjectName, i + 1,
                                    upload.result().getUploadId());
        input.setCopySourceRangeStart(offset);
        input.setCopySourceRangeEnd(offset + size - 1);
        auto output = client.uploadPartCopy(input);
        if (!output.isSuccess()) {
            std::cout << "upload part " << i << "failed, error is: " << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
        UploadedPartV2 temp(output.result().getPartNumber(), output.result().getETag());
        copyParts[i] = temp;
    }

    CompleteMultipartUploadV2Input input(bucketName, objectName, upload.result().getUploadId(), copyParts);
    auto complete = client.completeMultipartUpload(input);
    if (!complete.isSuccess()) {
        std::cout << "CompleteMultipartUpload error: " << complete.error().String() << std::endl;
        return -1;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}