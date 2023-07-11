#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include "model/object/DownloadFileInput.h"
#include <gtest/gtest.h>
#include <fstream>


namespace VolcengineTos {
class DownLoadFileTest : public ::testing::Test {
protected:
    DownLoadFileTest() {
    }

    ~DownLoadFileTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix) + "downloadfile";
        TestUtils::CreateBucket(cliV2, bucketName);
        workPath = FileUtils::getWorkPath();
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

std::shared_ptr<TosClientV2> DownLoadFileTest::cliV2 = nullptr;
std::string DownLoadFileTest::bucketName = "";
std::string DownLoadFileTest::workPath = "";

TEST_F(DownLoadFileTest, DownLoadFileWithoutCheckpointTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER +"downloadFile1";
    remove(filePath.c_str());
    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    std::fstream file;
//    file.open(filePath, std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
//    file.close();
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bucketName, objectName, ss);
    auto putOutput = cliV2->putObject(input_obj_put);
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(3);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = cliV2->downloadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    std::fstream file_;
    file_.open(filePath, std::ios_base::in | std::ios_base::binary);
    file_.seekg(0, file_.beg);
    std::ostringstream ssFromFile;
    ssFromFile << file_.rdbuf();
    file_.close();
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == ssFromFile.str());
    EXPECT_EQ(check_data, true);
    remove(filePath.c_str());
}
// 源数据发生变化的场景，带上进度条、限流、事件、cancel等
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
// 源数据不发生变化的场景
// 测试方式：可以先断点到 checkpoint dump 的位置，然后中断重续，之后重新运行程序
// 需要注意不要 putobject 把数据覆盖了，后续可以考虑使用白名单桶保存对象
// TEST_F(DownLoadFileTest, DownLoadFileWithCheckpointTest) {
//    std::string filePath = "/Users/bytedance/cpp_workspace/tos-sdk/ve-tos-cpp-sdk/testdir/testdirrrr/downloadfile";
//    std::string objectName = "testdownloadfile";
//    std::string bucketName = "testdownloadfile";
//    std::fstream file;
//    file.open(filePath, std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
//    auto ss = std::make_shared<std::stringstream>();
//    for (int i = 0; i < (11 << 20); ++i) {
//        *ss << 1;
//    }
//    //    TestUtils::CreateBucket(cliV2, bucketName);
//    //    PutObjectV2Input input_obj_put(bucketName, objectName, ss);
//    //    auto putOutput = cliV2->putObject(input_obj_put);
//    DownloadFileInput input;
//    // 对象名和桶名
//    HeadObjectV2Input headInput(bucketName, objectName);
//    input.setHeadObjectV2Input(headInput);
//    // 并发下载分片的线程数 1-1000
//    input.setTaskNum(1);
//    // 开启 checkpoint 会在本地生成断点续传记录文件
//    input.setEnableCheckpoint(true);
//    // 默认分片大小 20MB
//    input.setPartSize(5 * 1024 * 1024);
//    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
//    input.setFilePath(filePath);
//    // 设置进度条
//    DataTransferListener processHandler = {ProgressCallback, nullptr};
//    input.setDataTransferListener(processHandler);
//    auto output = cliV2->downloadFile(input);
//    EXPECT_EQ(output.isSuccess(), true);
//
//    std::fstream file_;
//    file_.open(filePath, std::ios_base::in | std::ios_base::binary);
//    file_.seekg(0, file_.beg);
//    std::ostringstream ssFromFile;
//    ssFromFile << file_.rdbuf();
//    std::string data = std::string((11 << 20), '1');
//    bool check_data = (data == ssFromFile.str());
//    EXPECT_EQ(check_data, true);
//    //    remove(filePath.c_str());
//}

// 源数据发生变化的场景
TEST_F(DownLoadFileTest, DownLoadFileWithCheckpointWithObjectChangedTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "downloadFile2";
    remove(filePath.c_str());
    std::string objectName = "Test-DownloadFile";
//    std::fstream file;
//    file.open(filePath, std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
//    file.close();
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bucketName, objectName, ss);
    auto putOutput = cliV2->putObject(input_obj_put);
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(3);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto output = cliV2->downloadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    std::fstream file_;
    file_.open(filePath, std::ios_base::in | std::ios_base::binary);
    file_.seekg(0, file_.beg);
    std::ostringstream ssFromFile;
    ssFromFile << file_.rdbuf();
    file_.close();
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == ssFromFile.str());
    EXPECT_EQ(check_data, true);
    remove(filePath.c_str());
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
TEST_F(DownLoadFileTest, DownLoadFileWithCheckpointWithProcessTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "downloadFile3";
    remove(filePath.c_str());
    std::string objectName = "Test-DownloadFile";
//    std::fstream file;
//    file.open(filePath, std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
//    file.close();
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bucketName, objectName, ss);
    auto putOutput = cliV2->putObject(input_obj_put);
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(5);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    // 设置进度条
    DataTransferListener processHandler = {ProgressCallback, nullptr};
    input.setDataTransferListener(processHandler);
    // 设置 rateLimiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
    input.setRateLimiter(RateLimiter);
    // 设置 DownloadEvent
    DownloadEventListener downloadHandler = {DownloadCallBack};
    input.setDownloadEventListener(downloadHandler);
    // 设置 cancelHook
    std::shared_ptr<CancelHook> CancelHook(NewCancelHook());
    input.setCancelHook(CancelHook);

    auto output = cliV2->downloadFile(input);
    EXPECT_EQ(output.isSuccess(), true);

    std::fstream file_;
    file_.open(filePath, std::ios_base::in | std::ios_base::binary);
    file_.seekg(0, file_.beg);
    std::ostringstream ssFromFile;
    ssFromFile << file_.rdbuf();
    file_.close();
    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == ssFromFile.str());
    EXPECT_EQ(check_data, true);
    remove(filePath.c_str());
}
TEST_F(DownLoadFileTest, DownLoadFileWithDirPathTest) {
//
//
//    std::string tempFilePath = workPath + "test/testdata/" + "t.temp";
//    std::fstream tempFile;
//    tempFile.open(tempFilePath, std::ios::app);
//    if (tempFile.is_open())
//    {
//        tempFile << "Hello, World!";
//        tempFile.close();
//    }
//

    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, "");
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(5);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);

    std::string tmpFile = FileUtils::getTempPath() + "cppsdk" + TOS_PATH_DELIMITER;
    input.setCheckpointFile(tmpFile);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径

    auto keyList = {"a/b.file","/a/b.file", "/a/b/c.file", "/a/d/e/f/g.file", "a/b/d.file", "/a/g/", "/a/f"};
    for (auto& key : keyList) {
        TestUtils::PutObject(cliV2, bucketName, key, TestUtils::GetRandomString(5 * 1024));
    }
    std::string baseFilePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER;
    std::string filePath = baseFilePath + TestUtils::GetRandomString(4);
    input.setFilePath(filePath);
    for (auto& key : keyList) {
        headInput.setKey(key);
        input.setHeadObjectV2Input(headInput);
        auto output = cliV2->downloadFile(input);
        EXPECT_EQ(output.isSuccess(), true);
        if (!output.isSuccess()) {
            std::cout << key << "  " << output.error().getMessage() << std::endl;
        }
    }
    filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + TestUtils::GetRandomString(4);
    input.setFilePath(filePath);
    FileUtils::CreateDir(filePath, false);
    for (auto& key : keyList) {
        headInput.setKey(key);
        input.setHeadObjectV2Input(headInput);
        auto output = cliV2->downloadFile(input);
        EXPECT_EQ(output.isSuccess(), true);
        if (!output.isSuccess()) {
            std::cout << key << "  " << output.error().getMessage() << std::endl;
        }
    }
}
TEST_F(DownLoadFileTest, DownLoadFileWithTrafficLimitTest) {
    std::string filePath = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "downloadFile1";
    remove(filePath.c_str());
    std::string objectName = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bucketName, objectName, ss);
    auto putOutput = cliV2->putObject(input_obj_put);
    DownloadFileInput input;
    // 对象名和桶名
    HeadObjectV2Input headInput(bucketName, objectName);
    input.setHeadObjectV2Input(headInput);
    // 并发下载分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(false);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 下载后文件的保存路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(filePath);
    auto startTime = std::chrono::high_resolution_clock::now();
    auto output = cliV2->downloadFile(input);
    EXPECT_EQ(output.isSuccess(), true);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto fp_ms = endTime - startTime;
    auto time1 = fp_ms.count() / 1000;
    remove(filePath.c_str());

    input.setTrafficLimit(1024 * 1024);
    startTime = std::chrono::high_resolution_clock::now();
    auto output2 = cliV2->downloadFile(input);
    EXPECT_EQ(output2.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time2 = fp_ms.count() / 1000;
    EXPECT_EQ(time2 > time1, true);
    remove(filePath.c_str());
}
}  // namespace VolcengineTos
