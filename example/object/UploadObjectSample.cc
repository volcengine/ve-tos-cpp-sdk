#include <iostream>
#include "UploadObjectSample.h"
#include <fstream>
#include <thread>
using namespace VolcengineTos;
int UploadObjectSample::PutObjectFromBuffer() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);
    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObject success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int UploadObjectSample::PutObjectFromFileContent() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 fileStream 的形式上传
    auto content = std::make_shared<std::fstream>(filePath, std::ios::in | std::ios_base::binary);
    PutObjectV2Input input(bucketName, objectName, content);
    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectFromFileContent failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObjectFromFileContent success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int UploadObjectSample::PutObjectFromFile() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    PutObjectFromFileInput input(bucketName, objectName, filePath);
    auto output = client.putObjectFromFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectFromFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObjectFromFile success. the object etag:" << output.result().getPutObjectV2Output().getETag()
              << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int UploadObjectSample::PutObjectWithMetaData() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);
    // 设置 ACL 为 PublicRead
    input.setAcl(ACLType::PublicRead);
    // 设置 StorageClass 为 IA
    input.setStorageClass(StorageClassType::IA);
    // 设置对象元数据
    input.setMeta({{"self-test", "yes"}});
    // 设置ContentType
    input.setContentType("application/json");
    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectWithMetaData failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObjectWithMetaData success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

static void ProgressCallback(std::shared_ptr<DataTransferStatus> datatransferstatus) {
    int64_t consumedBytes = datatransferstatus->consumedBytes_;
    int64_t totalBytes = datatransferstatus->totalBytes_;
    int64_t rwOnceBytes = datatransferstatus->rwOnceBytes_;
    DataTransferType type = datatransferstatus->type_;
    int64_t rate = 100 * consumedBytes / totalBytes;
    std::cout << "rate:" << rate << ","
              << "ConsumedBytes:" << consumedBytes << ","
              << "totalBytes:" << totalBytes << ","
              << "rwOnceBytes:" << rwOnceBytes << ","
              << "DataTransferType:" << type << std::endl;
}

int UploadObjectSample::PutObjectWithProcess() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);

    // 设置进度条
    DataTransferListener datatransferlistener = {ProgressCallback, nullptr};
    input.setDataTransferListener(datatransferlistener);

    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectWithProcess failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObjectWithProcess success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int UploadObjectSample::PutObjectWithRateLimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);

    // 设置客户端限速
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectWithRateLimiter failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutObjectWithRateLimiter success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
#include <dirent.h>
static void GenerateFilePaths(const std::string& folderPath, std::vector<std::string>& filePaths) {
    struct dirent* dirp;
    DIR* dp = opendir(folderPath.c_str());
    if (dp == nullptr) {
        return;
    }
    while ((dirp = readdir(dp)) != nullptr) {
        // 文件
        if (dirp->d_type == 8) {
            std::string fileName = dirp->d_name;
            std::string filePath = folderPath + fileName;
            filePaths.push_back(filePath);
        }
        // 文件夹
        if (dirp->d_type == 4) {
            std::string folderName = dirp->d_name;
            if (folderName != "." && folderName != "..") {
                std::string NewfolderPath = folderPath + folderName + "/";
                GenerateFilePaths(NewfolderPath, filePaths);
            }
        }
    }
    closedir(dp);
}
int UploadObjectSample::PutObjectFromFolder() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件夹完整路径，例如/localpath/
    std::string folderPath = "/localpath/";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    std::vector<std::string> filePaths;
    GenerateFilePaths(folderPath, filePaths);
    PutObjectFromFileInput input(bucketName);
    for (auto& filePath : filePaths) {
        input.setKey(filePath);
        input.setFilePath(filePath);
        auto output = client.putObjectFromFile(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "PutObjectFromFile failed. the object name:" << filePath << std::endl;
            std::cout << "PutObjectFromFile failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
        std::cout << "PutObjectFromFile success. the object name:" << filePath << std::endl;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
};

int UploadObjectSample::AppendObjectFromBuffer() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 第一次追加的位置是0，返回值为下一次追加的位置。后续追加的位置是追加前文件的长度。
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input(bucketName, objectName, part0, 0);
    auto output = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output.result().getHashCrc64ecma() << std::endl;

    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input.setContent(part1);
    // 从第一次追加的返回结果中，获取后续追加的位置offset和用于计算Crc64的PreHashCrc64Ecma
    input.setOffset(output.result().getNextAppendOffset());
    input.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output1.result().getHashCrc64ecma() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int UploadObjectSample::AppendObjectWithProcess() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 第一次追加的位置是0，返回值为下一次追加的位置。后续追加的位置是追加前文件的长度。
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input(bucketName, objectName, part0, 0);

    // 设置进度条
    DataTransferListener datatransferlistener = {ProgressCallback, nullptr};
    input.setDataTransferListener(datatransferlistener);

    auto output = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output.result().getHashCrc64ecma() << std::endl;

    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input.setContent(part1);
    // 从第一次追加的返回结果中，获取后续追加的位置offset和用于计算Crc64的PreHashCrc64Ecma
    input.setOffset(output.result().getNextAppendOffset());
    input.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output1.result().getHashCrc64ecma() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int UploadObjectSample::AppendObjectWithRateLimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 第一次追加的位置是0，返回值为下一次追加的位置。后续追加的位置是追加前文件的长度。
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input(bucketName, objectName, part0, 0);

    // 设置客户端限速
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output.result().getHashCrc64ecma() << std::endl;

    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input.setContent(part1);
    // 从第一次追加的返回结果中，获取后续追加的位置offset和用于计算Crc64的PreHashCrc64Ecma
    input.setOffset(output.result().getNextAppendOffset());
    input.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = client.appendObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AppendObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AppendObject success." << std::endl;
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    std::cout << "Pre HashCrc64Ecma is: " << output1.result().getHashCrc64ecma() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

static int64_t getFileSize(const std::string& file) {
    std::fstream f(file, std::ios::in | std::ios::binary);
    f.seekg(0, f.end);
    int64_t size = f.tellg();
    f.close();
    return size;
}

int UploadObjectSample::MultipartUpload() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 初始化分片上传事件
    CreateMultipartUploadInput input(bucketName, objectName);
    // 可以指定 ACL，StorageClass，用户自定义元数据等
    input.setAcl(ACLType::PublicRead);
    input.setStorageClass(StorageClassType::IA);
    input.setMeta({{"self-test", "yes"}});
    auto upload = client.createMultipartUpload(input);
    if (!upload.isSuccess()) {
        // 异常处理
        std::cout << "createMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "createMultipartUpload success." << std::endl;

    // 准备上传的文件
    std::string fileToUpload = "yourLocalFilename";
    int64_t partSize = 100 * 1024;
    std::vector<UploadedPartV2> partResList;
    auto fileSize = getFileSize(fileToUpload);
    int partCount = static_cast<int>(fileSize / partSize);
    // 计算分片个数
    if (fileSize % partSize != 0) {
        partCount++;
    }

    // 对每一个分片进行上传
    auto uploadId = upload.result().getUploadId();
    for (int i = 1; i <= partCount; i++) {
        auto offset = partSize * (i - 1);
        auto size = (partSize < fileSize - offset) ? partSize : (fileSize - offset);
        std::shared_ptr<std::iostream> content =
                std::make_shared<std::fstream>(fileToUpload, std::ios::in | std::ios::binary);
        content->seekg(offset, std::ios::beg);

        UploadPartV2Input uploadPartInput(bucketName, objectName, uploadId, size, i, content);
        auto uploadPartOutput = client.uploadPart(uploadPartInput);
        if (!uploadPartOutput.isSuccess()) {
            std::cout << "uploadPart failed." << upload.error().String() << std::endl;
        }
        UploadedPartV2 part(i, uploadPartOutput.result().getETag());
        partResList.push_back(part);
    }

    // 完成分片上传
    CompleteMultipartUploadV2Input completeMultipartUploadInput(bucketName, objectName, uploadId, partResList);
    auto com = client.completeMultipartUpload(completeMultipartUploadInput);
    if (!com.isSuccess()) {
        // 异常处理
        std::cout << "CompleteMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "CompleteMultipartUpload success." << com.result().getRequestInfo().getRequestId() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int UploadObjectSample::ListParts() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region。
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket。
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写UploadId，例如 f93f6fc9da94371f321e1008。uploadId来自于CreateMultipartUploadInput返回的结果。
    std::string uploadId = "f93f6fc9da94371f321e1008";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 列举已上传分片，默认列举1000个分片
    ListPartsInput input(bucketName, objectName, uploadId);
    // 设置返回分片上传任务的最大数量。
    input.setMaxParts(100);
    // 指定分片号的起始位置，只列举分片号大于此值的段，以 1 为例子。
    input.setPartNumberMarker(1);
    auto output = client.listParts(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListParts failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "ListParts success." << std::endl;
    for (const auto& part : output.result().getParts()) {
        std::cout << "part"
                  << ",partNumber:" << part.getPartNumber() << ",eTag:" << part.getETag() << ",size:" << part.getSize()
                  << ",lastModified time:" << part.getStringFormatLastModified() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
int UploadObjectSample::AbortMultipartUpload() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region。
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket。
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写UploadId，例如 f93f6fc9da94371f321e1008。uploadId来自于CreateMultipartUploadInput返回的结果。
    std::string uploadId = "f93f6fc9da94371f321e1008";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 列举已上传分片，默认列举1000个分片
    AbortMultipartUploadInput input(bucketName, objectName, uploadId);
    auto output = client.abortMultipartUpload(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "AbortMultipartUpload failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "AbortMultipartUpload success." << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
// int UploadObjectSample::MultipartUploadWithProcess(){
//     // 初始化 TOS 账号信息
//     // Your Region 填写 Bucket 所在 Region
//     std::string region = "Your Region";
//     std::string accessKey = "Your Access Key";
//     std::string secretKey = "Your Secret Key";
//     // 填写 Bucket 名称，例如 examplebucket
//     std::string bucketName = "examplebucket";
//     // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
//     std::string objectName = "exampledir/exampleobject.txt";
//
//     // 初始化网络等资源
//     InitializeClient();
//     // 创建交互的 client
//     TosClientV2 client(region, accessKey, secretKey);
//
//     // 初始化分片上传事件
//     CreateMultipartUploadInput input(bucketName, objectName);
//     // 可以指定 ACL，StorageClass，用户自定义元数据等
//     input.setAcl(ACLType::PublicRead);
//     input.setStorageClass(StorageClassType::IA);
//     input.setMeta({{"self-test", "yes"}});
//     auto upload = client.createMultipartUpload(input);
//     if (!upload.isSuccess()) {
//         std::cout << "createMultipartUpload success." << std::endl;
//     } else {
//         // 异常处理
//         std::cout << "createMultipartUpload failed." << upload.error().String() << std::endl;
//         // 释放网络等资源
//         CloseClient();
//         return -1;
//     }
//
//     // 准备上传的文件
//     std::string fileToUpload = "yourLocalFilename";
//     int64_t partSize = 100 * 1024;
//     std::vector<UploadedPartV2> partResList;
//     auto fileSize = getFileSize(fileToUpload);
//     int partCount = static_cast<int>(fileSize / partSize);
//     // 计算分片个数
//     if (fileSize % partSize != 0) {
//         partCount++;
//     }
//
//     // 对每一个分片进行上传
//     auto uploadId = upload.result().getUploadId();
//     for (int i = 1; i <= partCount; i++) {
//         auto offset = partSize * (i - 1);
//         auto size = (partSize < fileSize - offset) ? partSize : (fileSize - offset);
//         std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(fileToUpload,
//         std::ios::in|std::ios::binary); content->seekg(offset, std::ios::beg);
//
//         UploadPartV2Input uploadPartInput(bucketName, objectName,uploadId, size,i,content);
//
//         // 设置进度条
//         auto basicInput = uploadPartInput.getUploadPartBasicInput();
//         DataTransferListener datatransferlistener = {ProgressCallback, nullptr};
//         basicInput.setDataTransferListener(datatransferlistener);
//         uploadPartInput.setUploadPartBasicInput(basicInput);
//
//         auto uploadPartOutput = client.uploadPart(uploadPartInput);
//         if (!uploadPartOutput.isSuccess()) {
//             UploadedPartV2 part(i, uploadPartOutput.result().getETag());
//             partResList.push_back(part);
//         }
//         else {
//             std::cout << "uploadPart failed." << upload.error().String() << std::endl;
//         }
//
//     }
//
//     // 完成分片上传
//     CompleteMultipartUploadV2Input completeMultipartUploadInput(bucketName, objectName, uploadId, partResList);
//     auto com = client.completeMultipartUpload(completeMultipartUploadInput);
//     if (!com.isSuccess()) {
//         std::cout << "CompleteMultipartUpload success." << com.result().getRequestInfo().getRequestId() << std::endl;
//     } else {
//         // 异常处理
//         std::cout << "CompleteMultipartUpload failed." << upload.error().String() << std::endl;
//         // 释放网络等资源
//         CloseClient();
//         return -1;
//     }
//     return 0;
// }
// int UploadObjectSample::MultipartUploadWithRateLimiter(){
//     // 初始化 TOS 账号信息
//     // Your Region 填写 Bucket 所在 Region
//     std::string region = "Your Region";
//     std::string accessKey = "Your Access Key";
//     std::string secretKey = "Your Secret Key";
//     // 填写 Bucket 名称，例如 examplebucket
//     std::string bucketName = "examplebucket";
//     // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
//     std::string objectName = "exampledir/exampleobject.txt";
//
//     // 初始化网络等资源
//     InitializeClient();
//     // 创建交互的 client
//     TosClientV2 client(region, accessKey, secretKey);
//
//     // 初始化分片上传事件
//     CreateMultipartUploadInput input(bucketName, objectName);
//     // 可以指定 ACL，StorageClass，用户自定义元数据等
//     input.setAcl(ACLType::PublicRead);
//     input.setStorageClass(StorageClassType::IA);
//     input.setMeta({{"self-test", "yes"}});
//     auto upload = client.createMultipartUpload(input);
//     if (!upload.isSuccess()) {
//         std::cout << "createMultipartUpload success." << std::endl;
//     } else {
//         // 异常处理
//         std::cout << "createMultipartUpload failed." << upload.error().String() << std::endl;
//         // 释放网络等资源
//         CloseClient();
//         return -1;
//     }
//
//     // 准备上传的文件
//     std::string fileToUpload = "yourLocalFilename";
//     int64_t partSize = 100 * 1024;
//     std::vector<UploadedPartV2> partResList;
//     auto fileSize = getFileSize(fileToUpload);
//     int partCount = static_cast<int>(fileSize / partSize);
//     // 计算分片个数
//     if (fileSize % partSize != 0) {
//         partCount++;
//     }
//
//     // 对每一个分片进行上传
//     auto uploadId = upload.result().getUploadId();
//     for (int i = 1; i <= partCount; i++) {
//         auto offset = partSize * (i - 1);
//         auto size = (partSize < fileSize - offset) ? partSize : (fileSize - offset);
//         std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(fileToUpload,
//         std::ios::in|std::ios::binary); content->seekg(offset, std::ios::beg);
//
//         UploadPartV2Input uploadPartInput(bucketName, objectName,uploadId, size,i,content);
//
//         // 设置客户端限速
//         auto basicInput = uploadPartInput.getUploadPartBasicInput();
//         std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
//         basicInput.setRateLimiter(RateLimiter);
//         uploadPartInput.setUploadPartBasicInput(basicInput);
//
//         auto uploadPartOutput = client.uploadPart(uploadPartInput);
//         if (!uploadPartOutput.isSuccess()) {
//             UploadedPartV2 part(i, uploadPartOutput.result().getETag());
//             partResList.push_back(part);
//         }
//         else {
//             std::cout << "uploadPart failed." << upload.error().String() << std::endl;
//         }
//
//     }
//
//     // 完成分片上传
//     CompleteMultipartUploadV2Input completeMultipartUploadInput(bucketName, objectName, uploadId, partResList);
//     auto com = client.completeMultipartUpload(completeMultipartUploadInput);
//     if (!com.isSuccess()) {
//         std::cout << "CompleteMultipartUpload success. request id: " << com.result().getRequestInfo().getRequestId()
//         <<
//                 std::endl;
//     } else {
//         // 异常处理
//         std::cout << "CompleteMultipartUpload failed." << upload.error().String() << std::endl;
//         // 释放网络等资源
//         CloseClient();
//         return -1;
//     }
//     return 0;
// }

int UploadObjectSample::UploadFile() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 记录本地分片上传结果的文件。上传过程中的进度信息会保存在该文件中，如果某一分片上传失败，再次上传时会根据文件中记录的点继续上传。上传完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传上传
    UploadFileV2Input input(bucketName, objectName);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = client.uploadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "UploadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "UploadFile success. object etag is: " << output.result().getETag() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
int UploadObjectSample::UploadFileWithProcess() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 记录本地分片上传结果的文件。上传过程中的进度信息会保存在该文件中，如果某一分片上传失败，再次上传时会根据文件中记录的点继续上传。上传完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传上传
    UploadFileV2Input input(bucketName, objectName);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);

    auto output = client.uploadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "UploadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "UploadFile success. object etag is: " << output.result().getETag() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
static void UploadCallBack(std::shared_ptr<UploadEvent> event) {
    UploadEventType type_ = event->type_;
    bool error_ = event->error_;
    std::string bucket_ = event->bucket_;
    std::string key_ = event->key_;
    std::string fileInfo = bucket_ + key_;
    std::shared_ptr<std::string> uploadId_ = event->uploadId_;
    std::shared_ptr<std::string> checkpointFile_ = event->checkpointFile_;
    std::shared_ptr<UploadPartInfo> uploadPartInfo_ = event->uploadPartInfo_;
    std::string uploadPartInfo = "";
    if (uploadPartInfo_ != nullptr) {
        uploadPartInfo = std::to_string(uploadPartInfo_->partSize_) + "," + std::to_string(uploadPartInfo_->offset_) +
                         "," + std::to_string(uploadPartInfo_->partNumber_) + "," + *(uploadPartInfo_->eTag_) + "," +
                         std::to_string(*(uploadPartInfo_->hashCrc64ecma_));
    }
    std::cout << "type:" << type_ << ","
              << "error:" << error_ << ","
              << "fileInfo:" << fileInfo << ","
              << "checkpointFile:" << *checkpointFile_ << ","
              << "uploadId:" << *uploadId_ << ","
              << "uploadPartInf:" << uploadPartInfo << std::endl;
}
int UploadObjectSample::UploadFileWithEvent() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 记录本地分片上传结果的文件。上传过程中的进度信息会保存在该文件中，如果某一分片上传失败，再次上传时会根据文件中记录的点继续上传。上传完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传上传
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 UploadEvent
    UploadEventListener uploadHandler = {UploadCallBack};
    input.setUploadEventListener(uploadHandler);

    auto output = client.uploadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "UploadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "UploadFile success. object etag is: " << output.result().getETag() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
int UploadObjectSample::UploadFileWithRateLimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 记录本地分片上传结果的文件。上传过程中的进度信息会保存在该文件中，如果某一分片上传失败，再次上传时会根据文件中记录的点继续上传。上传完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传上传
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.uploadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "UploadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "UploadFile success. object etag is: " << output.result().getETag() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

static void threadFunction1(TosClientV2 client, UploadFileV2Input input) {
    auto output = client.uploadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "UploadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
    }
    std::cout << "UploadFile success. object etag is: " << output.result().getETag() << std::endl;
}

static void threadFunction2(std::shared_ptr<CancelHook> CancelHook) {
    // false 代表不清除 checkpoint 等信息。true 代表清除 checkpoint 等信息
    CancelHook->Cancel(false);
}

int UploadObjectSample::UploadFileWithCancel() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写本地文件完整路径，例如/localpath/examplefile.txt，其中localpath为本地文件examplefile.txt所在本地路径
    std::string filePath = "/localpath/examplefile.txt";
    // 记录本地分片上传结果的文件。上传过程中的进度信息会保存在该文件中，如果某一分片上传失败，再次上传时会根据文件中记录的点继续上传。上传完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传上传
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 cancelHook
    std::shared_ptr<CancelHook> CancelHook(NewCancelHook());
    input.setCancelHook(CancelHook);

    // 线程1中执行断点续传上传接口
    std::thread thread1(threadFunction1, client, input);
    thread1.detach();
    TimeUtils::sleepSecondTimes(5);
    // 运行五秒后在线程2中中断该次请求
    std::thread thread2(threadFunction2, CancelHook);
    thread2.join();
    // 释放网络等资源
    CloseClient();
    return 0;
}
