#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ResumableCopyObjectTest : public ::testing::Test {
protected:
    ResumableCopyObjectTest() {
    }

    ~ResumableCopyObjectTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
        srcBucketName = TestUtils::GetBucketName(TestConfig::TestPrefix) + "src";
        TestUtils::CreateBucket(cliV2, srcBucketName);
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
    static std::string srcBucketName;
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ResumableCopyObjectTest::cliV2 = nullptr;
std::string ResumableCopyObjectTest::bucketName = "";
std::string ResumableCopyObjectTest::srcBucketName = "";
std::string ResumableCopyObjectTest::workPath = "";

TEST_F(ResumableCopyObjectTest, ResumableCopyObjectWithoutCheckpointTest) {
    std::string filePath = workPath + "test/testdata/" + "uploadFile1";
    std::string srcObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "src";
    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    UploadFileV2Input uploadInput;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(srcBucketName, srcObjectName);
    uploadInput.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    uploadInput.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    uploadInput.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    uploadInput.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    uploadInput.setFilePath(filePath);
    auto fileOutput = cliV2->uploadFile(uploadInput);
    EXPECT_EQ(fileOutput.isSuccess(), true);

    std::string dstObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    ResumableCopyObjectInput input;
    // 对象名和桶名
    input.setBucket(bucketName);
    input.setKey(objectName);
    input.setSrcBucket(srcBucketName);
    input.setSrcKey(srcObjectName);

    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    auto output = cliV2->resumableCopyObject(input);
    EXPECT_EQ(output.isSuccess(), true);

    auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == temp);
    EXPECT_EQ(check_data, true);
}

// TEST_F(ResumableCopyObjectTest, UploadEmptyFileWithoutCheckpointTest) {
//     std::string filePath = workPath + "test/testdata/" + "uploadFile2";
//
//     std::string srcObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "src";
//     std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//
//     UploadFileV2Input uploadInput;
//     // 对象名和桶名
//     CreateMultipartUploadInput createMultiPartInput(srcBucketName, srcObjectName);
//     uploadInput.setCreateMultipartUploadInput(createMultiPartInput);
//     // 并发下载分片的线程数 1-1000
//     uploadInput.setTaskNum(1);
//     // 开启 checkpoint 会在本地生成断点续传记录文件
//     uploadInput.setEnableCheckpoint(false);
//     // 默认分片大小 20MB
//     uploadInput.setPartSize(5 * 1024 * 1024);
//     // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
//     uploadInput.setFilePath(filePath);
//     auto fileOutput = cliV2->uploadFile(uploadInput);
//     EXPECT_EQ(fileOutput.isSuccess(), true);
//
//     std::string dstObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//     ResumableCopyObjectInput input;
//     // 对象名和桶名
//     input.setBucket(bucketName);
//     input.setKey(objectName);
//     input.setSrcBucket(srcBucketName);
//     input.setSrcKey(srcObjectName);
//     // 并发下载分片的线程数 1-1000
//     input.setTaskNum(1);
//     // 开启 checkpoint 会在本地生成断点续传记录文件
//     input.setEnableCheckpoint(false);
//     // 默认分片大小 20MB
//     input.setPartSize(5 * 1024 * 1024);
//     auto output = cliV2->resumableCopyObject(input);
//     EXPECT_EQ(output.isSuccess(), true);
//     auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
//     EXPECT_EQ(temp.empty(), true);
// }

static void CopyCallBack(std::shared_ptr<CopyEvent> event) {
    CopyEventType type_ = event->type_;
    bool error_ = event->error_;
    std::string bucket_ = event->bucket_;
    std::string key_ = event->key_;
    std::string fileInfo = bucket_ + key_;
    std::shared_ptr<std::string> uploadId_ = event->uploadId_;
    std::shared_ptr<std::string> checkpointFile_ = event->checkpointFile_;
    std::shared_ptr<CopyPartInfo> uploadPartInfo_ = event->copyPartInfo_;
    std::string uploadPartInfo = "";
    if (uploadPartInfo_ != nullptr) {
        uploadPartInfo = std::to_string(uploadPartInfo_->partNumber_) + "," +
                         std::to_string(uploadPartInfo_->copySourceRangeStart_) + "," +
                         std::to_string(uploadPartInfo_->copySourceRangeEnd_) + "," + *(uploadPartInfo_->eTag_);
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

//// 源数据不发生变化的场景
//// 测试方式：可以先断点到 checkpoint dump 的位置，然后中断重续，之后重新运行程序
// TEST_F(ResumableCopyObjectTest, ResumableCopyObjectWithCheckpointTest) {
//     std::string filePath = workPath + "test/testdata/" + "uploadFile1";
//     std::string srcObjectName = "sdktest-object-1671764101929src";
//     std::string objectName = "sdktest-object-1671764101929";
//
////    UploadFileV2Input uploadInput;
////    // 对象名和桶名
////    CreateMultipartUploadInput createMultiPartInput(srcBucketName, srcObjectName);
////    uploadInput.setCreateMultipartUploadInput(createMultiPartInput);
////    // 并发下载分片的线程数 1-1000
////    uploadInput.setTaskNum(1);
////    // 开启 checkpoint 会在本地生成断点续传记录文件
////    uploadInput.setEnableCheckpoint(false);
////    // 默认分片大小 20MB
////    uploadInput.setPartSize(5 * 1024 * 1024);
////    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
////    uploadInput.setFilePath(filePath);
////    auto fileOutput = cliV2->uploadFile(uploadInput);
////    EXPECT_EQ(fileOutput.isSuccess(), true);
//
//    std::string dstObjectName = "sdktest-object-1671764105002";
//    ResumableCopyObjectInput input;
//    // 对象名和桶名
//    input.setBucket("sdktest-bucket-1671764101506");
//    input.setKey(objectName);
//    input.setSrcBucket("sdktest-bucket-1671764101506src");
//    input.setSrcKey(srcObjectName);
//    // 设置事件
//    CopyEventListener copyHandler = {CopyCallBack};
//    input.setCopyEventListener(copyHandler);
//    // 并发下载分片的线程数 1-1000
//    input.setTaskNum(1);
//    // 开启 checkpoint 会在本地生成断点续传记录文件
//    input.setEnableCheckpoint(true);
//    // 默认分片大小 20MB
//    input.setPartSize(5 * 1024 * 1024);
//    auto output = cliV2->resumableCopyObject(input);
//    EXPECT_EQ(output.isSuccess(), true);
//
//    auto temp = TestUtils::GetObjectContentByStream(cliV2, "sdktest-bucket-1671764101506", objectName);
//    std::string data = std::string((11 << 20), '1');
//    bool check_data = (data == temp);
//    EXPECT_EQ(check_data, true);
//}

TEST_F(ResumableCopyObjectTest, ResumableCopyObjectWithCheckpointWithProcessTest) {
    std::string filePath = workPath + "test/testdata/" + "uploadFile1";
    std::string srcObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "src";
    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    UploadFileV2Input uploadInput;
    // 对象名和桶名
    CreateMultipartUploadInput createMultiPartInput(srcBucketName, srcObjectName);
    uploadInput.setCreateMultipartUploadInput(createMultiPartInput);
    // 并发下载分片的线程数 1-1000
    uploadInput.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    uploadInput.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    uploadInput.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    uploadInput.setFilePath(filePath);
    auto fileOutput = cliV2->uploadFile(uploadInput);
    EXPECT_EQ(fileOutput.isSuccess(), true);

    std::string dstObjectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    ResumableCopyObjectInput input;
    // 对象名和桶名
    input.setBucket(bucketName);
    input.setKey(objectName);
    input.setSrcBucket(srcBucketName);
    input.setSrcKey(srcObjectName);

    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 设置 cancelHook
    std::shared_ptr<CancelHook> CancelHook(NewCancelHook());
    input.setCancelHook(CancelHook);
    // 设置事件
    CopyEventListener copyHandler = {CopyCallBack};
    input.setCopyEventListener(copyHandler);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    auto output = cliV2->resumableCopyObject(input);
    EXPECT_EQ(output.isSuccess(), true);

    auto temp = TestUtils::GetObjectContentByStream(cliV2, bucketName, objectName);
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == temp);
    EXPECT_EQ(check_data, true);
}
}  // namespace VolcengineTos
