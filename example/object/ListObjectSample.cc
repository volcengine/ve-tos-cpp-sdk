#include <iostream>
#include "ListObjectSample.h"

using namespace VolcengineTos;

int ListObjectSample::ListObjectsNew() {
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
    input.setMaxKeys(10);
    auto output = client.listObjectsType2(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListObjects success." << std::endl;
    for (const auto& object : output.result().getContents()) {
        std::cout << "object"
                  << ",name:" << object.getKey() << ",size:" << object.getSize()
                  << ",lastmodify time:" << object.getStringFormatLastModified()
                  << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithContinuationTokenNew() {
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
    input.setMaxKeys(100);
    auto output = client.listObjectsType2(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListObjects success." << std::endl;
    for (const auto& object : output.result().getContents()) {
        std::cout << "object"
                  << ",name:" << object.getKey() << ",size:" << object.getSize()
                  << ",lastmodify time:" << object.getStringFormatLastModified()
                  << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
    }

    // 使用 continuationToken 列举对象
    if (output.result().isTruncated()) {
        input.setContinuationToken(output.result().getNextContinuationToken());
        auto output1 = client.listObjectsType2(input);
        if (!output1.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output1.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output1.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithEncodingTypeNew() {
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
    input.setMaxKeys(100);
    input.setEncodingType("url");
    auto output = client.listObjectsType2(input);
    if (!output.isSuccess()) {
        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithPrefixNew() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsType2Input input(bucketName);
    input.setPrefix(prefix);
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

        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListAllObjectsNew() {
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

        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int ListObjectSample::ListObjectsWithDelimiterNew() {
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
    input.setDelimiter("/");
    input.setMaxKeys(10);
    auto output = client.listObjectsType2(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListObjects success." << std::endl;
    for (const auto& object : output.result().getContents()) {
        std::cout << "object"
                  << ",name:" << object.getKey() << ",size:" << object.getSize()
                  << ",lastmodify time:" << object.getStringFormatLastModified()
                  << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
    }
    for (const auto& prefix : output.result().getCommonPrefixes()) {
        std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListAllObjectsWithDelimiterNew() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsType2Input input(bucketName);
    input.setDelimiter("/");
    bool isTruncated = true;
    std::string nextContinuationToken = "";
    while (isTruncated) {
        input.setContinuationToken(nextContinuationToken);
        auto output = client.listObjectsType2(input);
        if (!output.isSuccess()) {
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
                std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
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

int ListObjectSample::ListObjects() {
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

    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(10);
    auto output = client.listObjects(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListObjects success." << std::endl;
    for (const auto& object : output.result().getContents()) {
        std::cout << "object"
                  << ",name:" << object.getKey() << ",size:" << object.getSize()
                  << ",lastmodify time:" << object.getStringFormatLastModified()
                  << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithNextMarker() {
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

    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(100);
    auto output = client.listObjects(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "ListObjects success." << std::endl;
    for (const auto& object : output.result().getContents()) {
        std::cout << "object"
                  << ",name:" << object.getKey() << ",size:" << object.getSize()
                  << ",lastmodify time:" << object.getStringFormatLastModified()
                  << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
    }
    if (output.result().isTruncated()) {
        input.setMarker(output.result().getNextMarker());
        auto output1 = client.listObjects(input);
        if (!output1.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output1.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithEncodingType() {
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

    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(100);
    input.setEncodingType("url");
    auto output = client.listObjects(input);
    if (!output.isSuccess()) {
        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithPrefix() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsV2Input input(bucketName);
    input.setPrefix(prefix);
    bool isTruncated = true;
    std::string nextMarker = "";
    while (isTruncated) {
        input.setMarker(nextMarker);
        auto output = client.listObjects(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
        nextMarker = output.result().getNextMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListAllObjects() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(100);
    bool isTruncated = true;
    std::string nextMarker = "";
    while (isTruncated) {
        input.setMarker(nextMarker);
        auto output = client.listObjects(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextMarker = output.result().getNextMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithDelimiter() {
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

    ListObjectsV2Input input(bucketName);
    input.setDelimiter("/");
    input.setMaxKeys(10);
    auto output = client.listObjects(input);
    if (!output.isSuccess()) {
        std::cout << "ListObjects success." << std::endl;
        for (const auto& object : output.result().getContents()) {
            std::cout << "object"
                      << ",name:" << object.getKey() << ",size:" << object.getSize()
                      << ",lastmodify time:" << object.getStringFormatLastModified()
                      << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
        }
        for (const auto& prefix : output.result().getCommonPrefixes()) {
            std::cout << "Sub Dir:" << prefix.getPrefix() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "ListObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int ListObjectSample::ListAllObjectsWithDelimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectsV2Input input(bucketName);
    input.setDelimiter("/");
    bool isTruncated = true;
    std::string nextMarker = "";
    while (isTruncated) {
        input.setMarker(nextMarker);
        auto output = client.listObjects(input);
        if (!output.isSuccess()) {
            nextMarker = output.result().getNextMarker();
            isTruncated = output.result().isTruncated();

            std::cout << "ListObjects success." << std::endl;
            for (const auto& object : output.result().getContents()) {
                std::cout << "object"
                          << ",name:" << object.getKey() << ",size:" << object.getSize()
                          << ",lastmodify time:" << object.getStringFormatLastModified()
                          << ",storage class:" << object.getStringFormatStorageClass() << std::endl;
            }
            for (const auto& prefix : output.result().getCommonPrefixes()) {
                std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
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

int ListObjectSample::ListObjectsVersions() {
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

    ListObjectVersionsV2Input input(bucketName);
    input.setMaxKeys(10);
    auto output = client.listObjectVersions(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListObjectVersions success." << std::endl;
    for (const auto& versions : output.result().getVersions()) {
        std::cout << "versions"
                  << ",name:" << versions.getKey() << ",size:" << versions.getSize() << ",etag:" << versions.getETag()
                  << ",is latest:" << versions.isLatest() << ",version id:" << versions.getVersionId()
                  << ",hashCrc64:" << versions.getHashCrc64Ecma()
                  << ",lastmodify time:" << versions.getStringFormatLastModified()
                  << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
    }
    for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
        std::cout << "deleteMarkers"
                  << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                  << ",version id:" << deleteMarker.getVersionId()
                  << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithEncodingTypeVersions() {
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

    ListObjectVersionsV2Input input(bucketName);
    input.setMaxKeys(10);
    input.setEncodingType("url");
    auto output = client.listObjectVersions(input);
    if (!output.isSuccess()) {
        for (const auto& versions : output.result().getVersions()) {
            std::cout << "versions"
                      << ",name:" << versions.getKey() << ",size:" << versions.getSize()
                      << ",etag:" << versions.getETag() << ",is latest:" << versions.isLatest()
                      << ",version id:" << versions.getVersionId() << ",hashCrc64:" << versions.getHashCrc64Ecma()
                      << ",lastmodify time:" << versions.getStringFormatLastModified()
                      << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            std::cout << "deleteMarkers"
                      << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                      << ",version id:" << deleteMarker.getVersionId()
                      << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithPrefixVersions() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    input.setPrefix(prefix);
    input.setMaxKeys(10);
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
        nextKeyMarker = output.result().getKeyMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListObjectVersions success." << std::endl;
        for (const auto& versions : output.result().getVersions()) {
            std::cout << "versions"
                      << ",name:" << versions.getKey() << ",size:" << versions.getSize()
                      << ",etag:" << versions.getETag() << ",is latest:" << versions.isLatest()
                      << ",version id:" << versions.getVersionId() << ",hashCrc64:" << versions.getHashCrc64Ecma()
                      << ",lastmodify time:" << versions.getStringFormatLastModified()
                      << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            std::cout << "deleteMarkers"
                      << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                      << ",version id:" << deleteMarker.getVersionId()
                      << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListAllObjectsVersions() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    input.setMaxKeys(10);
    bool isTruncated = true;

    std::string nextKeyMarker = "";
    std::string nextVersionIdMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        input.setVersionIdMarker(nextVersionIdMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        nextVersionIdMarker = output.result().getNextVersionIdMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListObjectVersions success." << std::endl;
        for (const auto& versions : output.result().getVersions()) {
            std::cout << "versions"
                      << ",name:" << versions.getKey() << ",size:" << versions.getSize()
                      << ",etag:" << versions.getETag() << ",is latest:" << versions.isLatest()
                      << ",version id:" << versions.getVersionId() << ",hashCrc64:" << versions.getHashCrc64Ecma()
                      << ",lastmodify time:" << versions.getStringFormatLastModified()
                      << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            std::cout << "deleteMarkers"
                      << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                      << ",version id:" << deleteMarker.getVersionId()
                      << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListObjectsWithDelimiterVersions() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    input.setDelimiter("/");
    input.setMaxKeys(100);
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    std::string nextVersionIdMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        input.setVersionIdMarker(nextVersionIdMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        nextVersionIdMarker = output.result().getNextVersionIdMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListObjectVersions success." << std::endl;
        for (const auto& versions : output.result().getVersions()) {
            std::cout << "versions"
                      << ",name:" << versions.getKey() << ",size:" << versions.getSize()
                      << ",etag:" << versions.getETag() << ",is latest:" << versions.isLatest()
                      << ",version id:" << versions.getVersionId() << ",hashCrc64:" << versions.getHashCrc64Ecma()
                      << ",lastmodify time:" << versions.getStringFormatLastModified()
                      << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            std::cout << "deleteMarkers"
                      << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                      << ",version id:" << deleteMarker.getVersionId()
                      << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
        }
        for (const auto& prefix : output.result().getCommonPrefixes()) {
            std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListAllObjectsWithDelimiterVersions() {
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

    ListObjectVersionsV2Input input(bucketName);
    input.setMaxKeys(100);
    input.setDelimiter("/");
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    std::string nextVersionIdMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        input.setVersionIdMarker(nextVersionIdMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            nextKeyMarker = output.result().getNextKeyMarker();
            nextVersionIdMarker = output.result().getNextVersionIdMarker();
            isTruncated = output.result().isTruncated();

            std::cout << "ListObjectVersions success." << std::endl;
            for (const auto& versions : output.result().getVersions()) {
                std::cout << "versions"
                          << ",name:" << versions.getKey() << ",size:" << versions.getSize()
                          << ",etag:" << versions.getETag() << ",is latest:" << versions.isLatest()
                          << ",version id:" << versions.getVersionId() << ",hashCrc64:" << versions.getHashCrc64Ecma()
                          << ",lastmodify time:" << versions.getStringFormatLastModified()
                          << ",storage class:" << versions.getStringFormatStorageClass() << std::endl;
            }
            for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
                std::cout << "deleteMarkers"
                          << ",name:" << deleteMarker.getKey() << ",is latest:" << deleteMarker.isLatest()
                          << ",version id:" << deleteMarker.getVersionId()
                          << ",lastmodify time:" << deleteMarker.getStringFormatLastModified() << std::endl;
            }
            for (const auto& prefix : output.result().getCommonPrefixes()) {
                std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
            }
        } else {
            // 异常处理
            std::cout << "ListObjectVersions failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListMultipartUpload() {
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

    // 列举已上传分片，默认列举1000个分片
    ListMultipartUploadsV2Input input(bucketName);
    // 设置返回分片上传任务的最大数量
    input.setMaxUploads(10);
    auto output = client.listMultipartUploads(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "ListMultipartUploads success." << std::endl;
    for (const auto& part : output.result().getUploads()) {
        std::cout << "part"
                  << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                  << ",storageClasss:" << part.getStringFormatStorageClass()
                  << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListMultipartUploadWithEncodingType() {
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

    // 列举已上传分片，默认列举1000个分片
    ListMultipartUploadsV2Input input(bucketName);
    // 设置返回分片上传任务的最大数量
    input.setMaxUploads(10);
    input.setEncodingType("url");
    auto output = client.listMultipartUploads(input);
    if (!output.isSuccess()) {
        std::cout << "ListMultipartUploads success." << std::endl;
        for (const auto& part : output.result().getUploads()) {
            std::cout << "part"
                      << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                      << ",storageClasss:" << part.getStringFormatStorageClass()
                      << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int ListObjectSample::ListMultipartUploadWithPrefix() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀 Prefix。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);
    // 列举已上传分片
    ListMultipartUploadsV2Input input(bucketName);
    // 设置列举前缀
    input.setPrefix(prefix);
    // 一次最多列举100条
    input.setMaxUploads(100);
    // isTruncated用于控制是否列举完全，nextKeyMarker用于递归列举
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listMultipartUploads(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListMultipartUploads success." << std::endl;
        for (const auto& part : output.result().getUploads()) {
            std::cout << "part"
                      << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                      << ",storageClasss:" << part.getStringFormatStorageClass()
                      << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int ListObjectSample::ListMultipartUploadsAll() {
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
    // 列举已上传分片
    ListMultipartUploadsV2Input input(bucketName);
    // 一次最多列举10条
    input.setMaxUploads(10);
    // isTruncated用于控制是否列举完全，nextKeyMarker用于递归列举
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listMultipartUploads(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListMultipartUploads success." << std::endl;
        for (const auto& part : output.result().getUploads()) {
            std::cout << "part"
                      << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                      << ",storageClasss:" << part.getStringFormatStorageClass()
                      << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int ListObjectSample::ListMultipartUploadWithDelimiter() {
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

    // 列举已上传分片
    ListMultipartUploadsV2Input input(bucketName);
    // 设置分割符为"/"
    input.setDelimiter("/");
    // 一次最多列举100条
    input.setMaxUploads(100);
    auto output = client.listMultipartUploads(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "ListMultipartUploads success." << std::endl;
    for (const auto& part : output.result().getUploads()) {
        std::cout << "part"
                  << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                  << ",storageClasss:" << part.getStringFormatStorageClass()
                  << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
    }
    for (const auto& prefix : output.result().getCommonPrefixes()) {
        std::cout << "Sub Dir:" << prefix.getPrefix() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int ListObjectSample::ListMultipartUploadAllWithDelimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写列举的文件前缀（Prefix）。
    std::string prefix = "yourkeyPrefix";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 列举已上传分片
    ListMultipartUploadsV2Input input(bucketName);
    // 设置分割符为"/"
    input.setDelimiter("/");
    // 一次最多列举10条
    input.setMaxUploads(10);
    // isTruncated用于控制是否列举完全，nextKeyMarker用于递归列举
    bool isTruncated = true;
    std::string nextKeyMarker = "";
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listMultipartUploads(input);
        if (!output.isSuccess()) {
            nextKeyMarker = output.result().getNextKeyMarker();
            isTruncated = output.result().isTruncated();

            std::cout << "ListMultipartUploads success." << std::endl;
            for (const auto& part : output.result().getUploads()) {
                std::cout << "part"
                          << ",key:" << part.getKey() << ",uploadId:" << part.getUploadId()
                          << ",storageClasss:" << part.getStringFormatStorageClass()
                          << ",initiated time:" << part.getStringFormatInitiated() << std::endl;
            }
            for (const auto& prefix : output.result().getCommonPrefixes()) {
                std::cout << "Common Prefixes" << prefix.getPrefix() << std::endl;
            }
        } else {
            // 异常处理
            std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
