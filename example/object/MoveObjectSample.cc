#include <iostream>
#include "MoveObjectSample.h"

using namespace VolcengineTos;

int MoveObjectSample::CopyAndDeleteObject() {
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
    if (output.isSuccess()) {
        // 异常处理
        std::cout << "CopyObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "CopyObject success. the object etag:" << output.result().getETag() << std::endl;

    DeleteObjectInput deleteInput(bucketName, objectName);
    auto deleteOutput = client.deleteObject(deleteInput);
    if (deleteOutput.isSuccess()) {
        // 异常处理
        std::cout << "DeleteObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "DeleteObject success." << std::endl;
    std::cout << " version id is:" << deleteOutput.result().getVersionId()
              << " is delete marker:" << deleteOutput.result().isDeleteMarker() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
std::string subReplace(std::string resourceStr, std::string subStr, std::string newStr) {
    std::string dstStr = resourceStr;
    std::string::size_type pos = 0;
    dstStr.replace(pos, subStr.length(), newStr);
    return dstStr;
}
int MoveObjectSample::CopyAndDeleteDirectory() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "destbucket";
    std::string srcBucketName = "srcbucket";
    std::string dir1 = "exampledir1/";
    std::string dir2 = "exampledir2/";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsType2Input input(bucketName);
    bool isTruncated = true;
    std::string nextContinuationToken = "";

    while (isTruncated) {
        input.setContinuationToken(nextContinuationToken);
        auto output = client.listObjectsType2(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextContinuationToken = output.result().getNextContinuationToken();
        isTruncated = output.result().isTruncated();

        for (const auto& object : output.result().getContents()) {
            auto newKey = object.getKey();
            if (newKey == dir1) {
                newKey = dir2;
            } else {
                newKey = subReplace(newKey, dir1, dir2);
            }
            CopyObjectV2Input input(bucketName, newKey, srcBucketName, object.getKey());
            auto output = client.copyObject(input);
            if (output.isSuccess()) {
                std::cout << "CopyObject success. the object etag:" << output.result().getETag() << std::endl;
            } else {
                // 异常处理
                std::cout << "CopyObject failed." << output.error().String() << std::endl;
            }
            DeleteObjectInput deleteInput(bucketName, object.getKey());
            auto deleteOutput = client.deleteObject(deleteInput);
            if (deleteOutput.isSuccess()) {
                std::cout << "DeleteObject success." << std::endl;
                std::cout << " version id is:" << deleteOutput.result().getVersionId()
                          << " is delete marker:" << deleteOutput.result().isDeleteMarker() << std::endl;

            } else {
                // 异常处理
                if (deleteOutput.error().getStatusCode() != 404) {
                    std::cout << "DeleteObject failed:" << deleteOutput.error().String() << std::endl;
                }
            }
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}