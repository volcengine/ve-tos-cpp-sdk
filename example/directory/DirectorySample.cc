#include <iostream>
#include "DirectorySample.h"

using namespace VolcengineTos;

int DirectorySample::PutFolder() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/。
    std::string objectName = "exampledir/";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    auto ss = std::make_shared<std::stringstream>("");
    PutObjectV2Input input(bucketName, objectName, ss);
    auto output = client.putObject(input);
    if (output.isSuccess()) {
        std::cout << "PutObject success. the object etag:" << output.result().getETag() << std::endl;
    } else {
        // 异常处理
        std::cout << "PutObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DirectorySample::PutMultiLevelFolder() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，maindir/subdir/。
    std::string objectName = "maindir/subdir/";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    auto ss = std::make_shared<std::stringstream>("");
    PutObjectV2Input input(bucketName, objectName, ss);
    auto output = client.putObject(input);
    if (output.isSuccess()) {
        std::cout << "PutObject success. the object etag:" << output.result().getETag() << std::endl;
    } else {
        // 异常处理
        std::cout << "PutObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DirectorySample::ListFolder() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsType2Input input(bucketName);
    std::string prefix = "";
    input.setDelimiter("/");
    bool isTruncated = true;
    std::string nextContinuationToken = "";
    while (isTruncated) {
        input.setContinuationToken(nextContinuationToken);
        input.setPrefix(prefix);
        auto output = client.listObjectsType2(input);
        if (output.isSuccess()) {
            nextContinuationToken = output.result().getNextContinuationToken();
            isTruncated = output.result().isTruncated();

            std::cout << "ListObjects success." << std::endl;
            for (const auto& object : output.result().getContents()) {
                std::cout << "object"
                          << ",name:" << object.getKey() << ",size:" << object.getSize()
                          << ",lastmodify time:" << object.getStringFormatLastModified()
                          << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
            }
            for (const auto& prefix : output.result().getCommonPrefixes()) {
                std::cout << "subDir is:" << prefix.getPrefix() << std::endl;
            }
        } else {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DirectorySample::DeleteFolder() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsType2Input input(bucketName);
    std::string prefix = "";
    input.setDelimiter("/");
    bool isTruncated = true;
    std::string nextContinuationToken = "";

    DeleteObjectInput deleteInput;
    deleteInput.setBucket(bucketName);
    while (isTruncated) {
        input.setContinuationToken(nextContinuationToken);
        input.setPrefix(prefix);
        auto output = client.listObjectsType2(input);
        if (output.isSuccess()) {
            nextContinuationToken = output.result().getNextContinuationToken();
            isTruncated = output.result().isTruncated();

            for (const auto& object : output.result().getContents()) {
                deleteInput.setKey(object.getKey());
                auto deleteOutput = client.deleteObject(deleteInput);
                if (!output.isSuccess()) {
                    std::cout << "delete failed. object name is:" << object.getKey() << std::endl;
                }
            }
        } else {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}