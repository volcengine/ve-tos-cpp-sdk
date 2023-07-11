#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class UploadFileTest : public ::testing::Test {
protected:
    UploadFileTest() {
    }

    ~UploadFileTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bucketName);
        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bucketName);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bucketName;
    static std::string workPath;
};

std::shared_ptr<TosClientV2> UploadFileTest::cliV2 = nullptr;
std::string UploadFileTest::bucketName = "";
std::string UploadFileTest::workPath = "";

TEST_F(UploadFileTest, UploadFileWithoutCheckpointTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "uploadFile1";

    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = cliV2->uploadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == temp);
    EXPECT_EQ(check_data, true);
}
TEST_F(UploadFileTest, UploadEmptyFileWithoutCheckpointTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "uploadFile2";

    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = cliV2->uploadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    GetObjectV2Input input_obj_get(bucketName, objectName);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getContentLength(), 0);
}
//  带上进度条、限流、事件、cancel等
static void ProgressCallback(std::shared_ptr<DataTransferStatus> datatransferstatus) {
    int64_t consumedBytes = datatransferstatus->consumedBytes_;
    int64_t totalBytes = datatransferstatus->totalBytes_;
    int64_t rwOnceBytes = datatransferstatus->rwOnceBytes_;
    DataTransferType type = datatransferstatus->type_;
    int64_t rate = 100 * consumedBytes / totalBytes;
    std::cout << "rate:" << rate
              << ","
                 "ConsumedBytes:"
              << consumedBytes << ","
              << "totalBytes:" << totalBytes << ","
              << "rwOnceBytes:" << rwOnceBytes << ","
              << "DataTransferType:" << type << std::endl;
}

//// 源数据不发生变化的场景
//// 测试方式：可以先断点到 checkpoint dump 的位置，然后中断重续，之后重新运行程序
// TEST_F(UploadFileTest, UploadFileWithCheckpointTest) {
//     std::string filePath = workPath + "test/testdata/" + "uploadFile1";
//
//     std::string objectName = "testuploadfile";
//     std::string bucketName = "testuploadfile";
//     TestUtils::CreateBucket(cliV2, bucketName);
//     UploadFileV2Input input;
//     // 对象名和桶名
//     CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
//     input.setCreateMultipartUploadInput(createMultiPartInput);
//     // 并发下载分片的线程数 1-1000
//     input.setTaskNum(1);
//     // 开启 checkpoint 会在本地生成断点续传记录文件
//     input.setEnableCheckpoint(true);
//     // 设置进度条
//     DataTransferListener processHandler = {ProgressCallback, nullptr};
//     input.setDataTransferListener(processHandler);
//     // 默认分片大小 20MB
//     input.setPartSize(5 * 1024 * 1024);
//     // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
//     input.setFilePath(filePath);
//     auto output = cliV2->uploadFile(input);
//     EXPECT_EQ(output.isSuccess(), true);
//
//     auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
//     std::string data = std::string((11 << 20), '1');
//     bool check_data = (data == temp);
//     EXPECT_EQ(check_data, true);
// }

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
    std::cout << "type:" << type_
              << ","
                 "error:"
              << error_ << ","
              << "fileInfo:" << fileInfo << ","
              << "checkpointFile:" << *checkpointFile_ << ","
              << "uploadId:" << *uploadId_ << ","
              << "uploadPartInf:" << uploadPartInfo << std::endl;
}
TEST_F(UploadFileTest, UploadFileWithCheckpointWithProcessTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "uploadFile1";

    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(3);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);

    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));
    input.setRateLimiter(RateLimiter);
    // 设置 DownloadEvent
    UploadEventListener uploadHandler = {UploadCallBack};
    input.setUploadEventListener(uploadHandler);
    // 设置 cancelHook
    std::shared_ptr<CancelHook> CancelHook(NewCancelHook());
    input.setCancelHook(CancelHook);

    auto output = cliV2->uploadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == temp);
    EXPECT_EQ(check_data, true);
}
TEST_F(UploadFileTest, UploadFileWithTrafficLimitTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "uploadFile1";

    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    UploadFileV2Input input;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(bucketName, objectName);
    input.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto startTime = std::chrono::high_resolution_clock::now();
    auto output = cliV2->uploadFile(input);
    EXPECT_EQ(output.isSuccess(), true);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto fp_ms = endTime - startTime;
    auto time1 = fp_ms.count() / 1000;

    input.setTrafficLimit(1024 * 1024);
    startTime = std::chrono::high_resolution_clock::now();
    auto output2 = cliV2->uploadFile(input);
    EXPECT_EQ(output2.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time2 = fp_ms.count() / 1000;
    EXPECT_EQ(time2 > time1, true);
}
}  // namespace VolcengineTos
