#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include "spdlog/spdlog.h"

namespace VolcengineTos {
class SupportProgressTest : public ::testing::Test {
protected:
    SupportProgressTest() {
    }

    ~SupportProgressTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        //        TestUtils::CleanAllBucket(cliV2);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bkt_name);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> SupportProgressTest::cliV2 = nullptr;
std::string SupportProgressTest::bkt_name = "";

static void ProgressCallback(std::shared_ptr<DataTransferStatus> dataTransferStatus) {
    int64_t consumedBytes = dataTransferStatus->consumedBytes_;
    int64_t totalBytes = dataTransferStatus->totalBytes_;
    int64_t rwOnceBytes = dataTransferStatus->rwOnceBytes_;
    DataTransferType type = dataTransferStatus->type_;
    int64_t rate = 100 * consumedBytes / totalBytes;
    std::cout << "rate:" << rate
              << ","
                 "ConsumedBytes:"
              << consumedBytes << ","
              << "totalBytes:" << totalBytes << ","
              << "rwOnceBytes:" << rwOnceBytes << ","
              << "DataTransferType:" << type << std::endl;
}

TEST_F(SupportProgressTest, PutGetObjectWithProgressCallbackTest) {
    spdlog::info("My info log");
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *ss << "1";
    }
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto basic = input_obj_put.getPutObjectBasicInput();
    // 设置回调
    DataTransferListener dataTransferListener = {ProgressCallback, nullptr};
    basic.setDataTransferListener(dataTransferListener);
    // 设置limiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
    basic.setRateLimiter(RateLimiter);
    input_obj_put.setPutObjectBasicInput(basic);
    auto out_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(out_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    input_obj_get.setDataTransferListener(dataTransferListener);
    input_obj_get.setRateLimiter(RateLimiter);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
}
// 进度条测试 + 限流测试 + crc64测试
TEST_F(SupportProgressTest, AppendObjectWithProgressCallbackTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_name, part0, 0);
    // 进度条测试
    DataTransferListener dataTransferListener = {ProgressCallback, nullptr};
    input_append.setDataTransferListener(dataTransferListener);

    auto output = cliV2->appendObject(input_append);
    EXPECT_EQ(output.isSuccess(), true);
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part1 << "2";
    }
    input_append.setContent(part1);
    input_append.setOffset(output.result().getNextAppendOffset());
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output1.isSuccess(), true);
}

TEST_F(SupportProgressTest, UploadPartWithProgressCallbackTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
    DataTransferListener dataTransferListener = {ProgressCallback, nullptr};
    // 测试数据
    int data_size_1 = (5 << 20);
    int data_size_2 = (4 << 20);
    std::string data_1 = std::string(data_size_1, '1');
    std::string data_2 = std::string(data_size_2, '2');
    std::string data = data_1 + data_2;

    auto upload = cliV2->createMultipartUpload(input_part_create);
    // part_1数据准备
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    // 校验Crc64
    // 设置limiter
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(20 * 1024 * 1024, 5 * 1024 * 1024));

    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto basic_1 = input_upload_part.getUploadPartBasicInput();
    basic_1.setDataTransferListener(dataTransferListener);
    basic_1.setRateLimiter(RateLimiter);
    input_upload_part.setUploadPartBasicInput(basic_1);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);
    // part_2数据准备
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }
    // 校验Md5，提供正确的Md5，校验成功
    UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto basic_input_2 = input_upload_part_2.getUploadPartBasicInput();
    std::string dataMd5 = CryptoUtils::md5Sum(data_2);
    basic_input_2.setContentMd5(dataMd5);
    basic_input_2.setDataTransferListener(dataTransferListener);
    input_upload_part_2.setUploadPartBasicInput(basic_input_2);
    auto part2 = cliV2->uploadPart(input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);
}

}  // namespace VolcengineTos
