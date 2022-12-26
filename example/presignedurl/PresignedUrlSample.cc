
#include "PresignedUrlSample.h"

using namespace VolcengineTos;

void PresignedUrlSample::GenPreSignedPostSigned() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写 Object 完整路径，完整路径中请不要包含 Bucket 的名称，例如 exampledir/exampleobject.txt 对象
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置 Post 表单中签名有效时间，单位：秒
    int64_t expires = 86400;
    // 生成 Post 上传预签名
    PreSignedPostSignatureInput input(bucketName, objectName, expires);
    auto output = client.preSignedPostSignature(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GeneratePreSignedUrl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return;
    }

    auto res = output.result();
    std::cout << "GeneratePreSignedUrl Success:"
              << "\n"
              << "Policy: " << res.getPolicy() << "\n"
              << "Algorithm: " << res.getAlgorithm() << "\n"
              << "Credential: " << res.getCredential() << "\n"
              << "Date: " << res.getDate() << "\n"
              << "OriginPolicy: " << res.getOriginPolicy() << "\n"
              << "Signature: " << res.getSignature() << "\n";
    // 释放网络等资源
    CloseClient();
    return;
}

void PresignedUrlSample::GenPreSignedPostSignedWithMultiForm() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写 Object 完整路径，完整路径中请不要包含 Bucket 的名称，例如 exampledir/exampleobject.txt 对象
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置 Post 表单中签名有效时间，单位：秒
    int64_t expires = 86400;
    // 生成 Post 上传预签名
    PreSignedPostSignatureInput input(bucketName, objectName, expires);
    // 设置拼接到 policy 中的条件组 conditions，以 x-tos-acl 和 key 为例
    input.setConditions({PostSignatureCondition("x-tos-acl", "public-read"),
                         PostSignatureCondition("key", "sdktest", std::make_shared<std::string>("starts-with"))});
    // 设置拼接到 policy 中的条件组 conditions 中的 content-length-range 字段
    input.setContentLengthRange(std::make_shared<ContentLengthRange>(50, 1025));
    auto output = client.preSignedPostSignature(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GeneratePreSignedUrl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return;
    }

    auto res = output.result();
    std::cout << "GeneratePreSignedUrl Success:"
              << "\n"
              << "Policy: " << res.getPolicy() << "\n"
              << "Algorithm: " << res.getAlgorithm() << "\n"
              << "Credential: " << res.getCredential() << "\n"
              << "Date: " << res.getDate() << "\n"
              << "OriginPolicy: " << res.getOriginPolicy() << "\n"
              << "Signature: " << res.getSignature() << "\n";
    // 释放网络等资源
    CloseClient();
    return;
}

void PresignedUrlSample::GenPutPreSignedUrl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写 Object 完整路径，完整路径中请不要包含 Bucket 的名称，例如 exampledir/exampleobject.txt 对象
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置预签名 URL 有效时间，单位：秒，
    int64_t expires = 86400;
    // 生成预签名 URL
    PreSignedURLInput input(HttpMethodType::Put, bucketName, objectName, expires);
    auto output = client.preSignedURL(input);
    if (!output.isSuccess()) {
        std::cout << "GeneratePreSignedUrl Success, Gen Url:" << output.result().getSignUrl() << std::endl;
    } else {
        // 异常处理
        std::cout << "GeneratePreSignedUrl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return;
    }

    // 释放网络等资源
    CloseClient();
    return;
}

void PresignedUrlSample::GenGetPreSignedUrl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写 Object 完整路径，完整路径中请不要包含 Bucket 的名称，例如 exampledir/exampleobject.txt 对象
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置预签名 URL 有效时间，单位：秒，
    int64_t expires = 86400;
    // 生成预签名 URL
    PreSignedURLInput input(HttpMethodType::Get, bucketName, objectName, expires);
    auto output = client.preSignedURL(input);
    if (!output.isSuccess()) {
        std::cout << "GeneratePreSignedUrl Success, Gen Url:" << output.result().getSignUrl() << std::endl;
    } else {
        // 异常处理
        std::cout << "GeneratePreSignedUrl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return;
    }

    // 释放网络等资源
    CloseClient();
    return;
}
