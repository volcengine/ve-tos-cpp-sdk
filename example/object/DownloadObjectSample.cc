#include <iostream>
#include "DownloadObjectSample.h"
#include <fstream>

using namespace VolcengineTos;

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

int DownloadObjectSample::GetObjectToBuffer() {
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

    GetObjectV2Input input(bucketName, objectName);
    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    auto stream = output.result().getContent();
    std::string ss;
    ;

    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        // 根据实际情况处理数据。
    }
    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::GetObjectToBufferVersions() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写对象的 versionId
    std::string versionId = "your object version";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    GetObjectV2Input input(bucketName, objectName);
    input.setVersionId(versionId);
    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectToFile() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    GetObjectToFileInput input(bucketName, objectName, filePath);
    auto output = client.getObjectToFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObjectToFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObjectToFile success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectWithRewriteResponseHeader() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 重写响应头
    input.setResponseCacheControl("no-cache");
    input.setResponseContentDisposition("attachment; filename=123.txt");
    input.setResponseContentEncoding("gzip");
    input.setResponseContentLanguage("en-US");
    input.setResponseContentType("text/plain");
    // 方式一：通过 transGMTFormatStringToTime 将 string 类型的时间转换为 time_t 类型的 expires
    //    time_t expires = TimeUtils::transGMTFormatStringToTime("Sat, 1 Jan 2022 00:00:00 GMT");
    //    if (expires == -1){
    //        std::cout << "Check expires, transfer string type to time_t failed." << std::endl;
    //    }
    // 方式二：通过修改 time_t 结构体，得到 expires
    //        time_t expirationTime = time(nullptr);
    //        tm* gmtmExpiration = gmtime(&expirationTime);
    //        gmtmExpiration->tm_min = 0;
    //        gmtmExpiration->tm_hour = 0;
    //        gmtmExpiration->tm_sec = 0;
    //        gmtmExpiration->tm_mday = 1;
    //        gmtmExpiration->tm_mon = 0;
    //        gmtmExpiration->tm_wday = 6;
    //        gmtmExpiration->tm_year = gmtmExpiration->tm_year;
    //        auto expires = timegm(gmtmExpiration);
    //
    //        input.setResponseExpires(expires);
    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectWithProcess() {
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
    GetObjectV2Input input(bucketName, objectName);

    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::GetObjectWithRateLimiter() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int GetObjectWithConditon() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 如果指定的时间早于实际修改时间，则正常传输文件，否则返回错误（304 Not modified）。86400 对应一天
    input.setIfModifiedSince(time(nullptr) - 86400);
    // 如果指定的时间等于或者晚于文件实际修改时间，则正常传输文件，否则返回错误（412 Precondition failed）。
    input.setIfUnmodifiedSince(time(nullptr));
    // 如果指定的ETag和Tos文件的ETag匹配，以文件的etag为8a36be0d764367db4eea2deb16b71543举例，则正常传输文件，否则返回错误（412
    // Precondition failed）。
    HeadObjectV2Input headInput(bucketName, objectName);
    auto headOutput = client.headObject(headInput);
    auto etag = headOutput.result().getETag();
    input.setIfMatch(etag);
    // 如果指定的ETag和Tos文件的ETag不匹配，则正常传输文件，否则返回错误（304 Not modified）。
    // input.setIfNoneMatch(etag)

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        if (output.error().getStatusCode() == 304) {
            std::cout << "StatusCode: 304" << output.error().String() << std::endl;
        }
        if (output.error().getStatusCode() == 412) {
            std::cout << "StatusCode: 412" << output.error().String() << std::endl;
        }
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectWithStartEnd() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 指定范围为 bytes=0-1000
    input.setRangeStart(0);
    input.setRangeEnd(1000);

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::GetObjectWithRange() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 指定范围为 bytes=0-1000
    input.setRange("bytes=0-1000");

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectWithStartEndWithRateLimiter() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 指定范围为 bytes=0-1000
    input.setRangeStart(0);
    input.setRangeEnd(1000);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int DownloadObjectSample::GetObjectWithStartEndWithProcess() {
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

    GetObjectV2Input input(bucketName, objectName);
    // 指定范围为 bytes=0-1000
    input.setRangeStart(0);
    input.setRangeEnd(1000);
    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);

    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
              << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::DownloadFile() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 记录断点记录文件的完整路径。只有当Object下载中断产生了断点记录文件后，如果需要继续下载该Object，才需要设置对应的断点记录文件。下载完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传下载
    DownloadFileInput input(bucketName, objectName);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = client.downloadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DownloadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DownloadFile success. object etag is: " << output.result().getHeadObjectV2Output().getETags()
              << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::DownloadFileWithProcess() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 记录断点记录文件的完整路径。只有当Object下载中断产生了断点记录文件后，如果需要继续下载该Object，才需要设置对应的断点记录文件。下载完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传下载
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);

    auto output = client.downloadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DownloadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DownloadFile success. object etag is: " << output.result().getHeadObjectV2Output().getETags()
              << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
static void DownloadCallBack(std::shared_ptr<DownloadEvent> event) {
    DownloadEventType type_ = event->type_;
    bool error_ = event->error_;
    std::string bucket_ = event->bucket_;
    std::string key_ = event->key_;
    std::string versionId_ = event->versionId_;
    std::string filePath_ = event->filePath_;
    std::string fileInfo = bucket_ + key_ + versionId_ + filePath_;
    std::shared_ptr<std::string> checkpointFile_ = event->checkpointFile_;
    std::shared_ptr<std::string> tempFilePath_ = event->tempFilePath_;
    std::shared_ptr<DownloadPartInfo> downloadPartInfo_ = event->downloadPartInfo_;
    std::string downloadPartInfo = "";
    if (downloadPartInfo_ != nullptr) {
        downloadPartInfo = std::to_string(downloadPartInfo_->rangeStart_) + "," +
                           std::to_string(downloadPartInfo_->rangeEnd_) + "," +
                           std::to_string(downloadPartInfo_->partNumber_);
    }

    std::cout << "type:" << type_
              << ","
                 "error:"
              << error_ << ","
              << "fileInfo:" << fileInfo << ","
              << "checkpointFile:" << *checkpointFile_ << ","
              << "tempFilePath:" << *tempFilePath_ << ","
              << "downloadPartInfo:" << downloadPartInfo << std::endl;
}
int DownloadObjectSample::DownloadFileWithEvent() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 记录断点记录文件的完整路径。只有当Object下载中断产生了断点记录文件后，如果需要继续下载该Object，才需要设置对应的断点记录文件。下载完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传下载
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 DownloadEvent
    DownloadEventListener downloadHandler = {DownloadCallBack};
    input.setDownloadEventListener(downloadHandler);

    auto output = client.downloadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DownloadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DownloadFile success. object etag is: " << output.result().getHeadObjectV2Output().getETags()
              << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
int DownloadObjectSample::DownloadFileWithRateLimiter() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 记录断点记录文件的完整路径。只有当Object下载中断产生了断点记录文件后，如果需要继续下载该Object，才需要设置对应的断点记录文件。下载完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传下载
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));
    input.setRateLimiter(RateLimiter);

    auto output = client.downloadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DownloadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DownloadFile success.object etag is: " << output.result().getHeadObjectV2Output().getETags()
              << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

static void threadFunction1(TosClientV2 client, DownloadFileInput input) {
    auto output = client.downloadFile(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DownloadFile failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
    }

    std::cout << "DownloadFile success. object etag is: " << output.result().getHeadObjectV2Output().getETags()
              << std::endl;
}

static void threadFunction2(std::shared_ptr<CancelHook> CancelHook) {
    // false 代表不清除 checkpoint 等信息。true 代表清除 checkpoint 等信息
    CancelHook->Cancel(false);
}

#include <thread>
int DownloadObjectSample::DownloadFileWithCancel() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 下载Object到本地文件examplefile.txt，并保存到指定的本地路径中，例如/localpath/examplefile.txt，如果指定的本地文件存在会覆盖，不存在则新建。
    std::string filePath = "/localpath/examplefile.txt";
    // 记录断点记录文件的完整路径。只有当Object下载中断产生了断点记录文件后，如果需要继续下载该Object，才需要设置对应的断点记录文件。下载完成后，该文件会被删除。
    // 如果未设置该值，默认与待上传的本地文件同路径。
    std::string CheckpointFilePath = "yourCheckpointFilepath";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 断点续传下载
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(10);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置 cancelHook
    std::shared_ptr<CancelHook> CancelHook(NewCancelHook());
    input.setCancelHook(CancelHook);

    // 线程1中执行断点续传下载接口
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
