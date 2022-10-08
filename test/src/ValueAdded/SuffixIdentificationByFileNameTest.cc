#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class SuffixIdentificationByFileNameTest : public ::testing::Test {
protected:
    SuffixIdentificationByFileNameTest() {
    }

    ~SuffixIdentificationByFileNameTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
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

std::shared_ptr<TosClientV2> SuffixIdentificationByFileNameTest::cliV2 = nullptr;
std::string SuffixIdentificationByFileNameTest::bkt_name = "";

TEST_F(SuffixIdentificationByFileNameTest, PutObjectTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix) + ".mp4";
    std::string data = "123456789";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setPutObjectBasicInput(basic);
    auto out_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(out_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "video/mp4", true);
}
TEST_F(SuffixIdentificationByFileNameTest, PutObjectWithDefaultTypeTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "123456789";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setPutObjectBasicInput(basic);
    auto out_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(out_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "application/octet-stream", true);
}
TEST_F(SuffixIdentificationByFileNameTest, AppendObjectTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix) + ".css";
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_name, part0, 0);
    auto output = cliV2->appendObject(input_append);
    EXPECT_EQ(output.isSuccess(), true);
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input_append.setContent(part1);
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    input_append.setOffset(output.result().getNextAppendOffset());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    EXPECT_EQ(output1.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "text/css", true);
}
TEST_F(SuffixIdentificationByFileNameTest, AppendObjectWithDefaultTypeTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_name, part0, 0);
    auto output = cliV2->appendObject(input_append);
    EXPECT_EQ(output.isSuccess(), true);
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input_append.setContent(part1);
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    input_append.setOffset(output.result().getNextAppendOffset());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    EXPECT_EQ(output1.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "application/octet-stream", true);
}
TEST_F(SuffixIdentificationByFileNameTest, CreateMultipartUploadTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix) + ".json";
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);

    // 测试数据
    int data_size_1 = (5 << 20);
    int data_size_2 = (4 << 20);
    std::string data_1 = std::string(data_size_1, '1');
    std::string data_2 = std::string(data_size_2, '2');
    std::string data = data_1 + data_2;

    auto upload = cliV2->createMultipartUpload(input_part_create);
    EXPECT_EQ(upload.isSuccess(), true);
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    // 校验Crc64
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());

    std::vector<UploadedPartV2> vec = {part_1};
    CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
    auto com = cliV2->completeMultipartUpload(input_upload_complete);
    EXPECT_EQ(com.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "application/json", true);
}
TEST_F(SuffixIdentificationByFileNameTest, CreateMultipartUploadWithDefaultTypeTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);

    // 测试数据
    int data_size_1 = (5 << 20);
    int data_size_2 = (4 << 20);
    std::string data_1 = std::string(data_size_1, '1');
    std::string data_2 = std::string(data_size_2, '2');
    std::string data = data_1 + data_2;

    auto upload = cliV2->createMultipartUpload(input_part_create);
    EXPECT_EQ(upload.isSuccess(), true);
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    // 校验Crc64
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());

    std::vector<UploadedPartV2> vec = {part_1};
    CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
    auto com = cliV2->completeMultipartUpload(input_upload_complete);
    EXPECT_EQ(com.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
    EXPECT_EQ(out_obj_get.result().getGetObjectBasicOutput().getContentType() == "application/octet-stream", true);
}
}  // namespace VolcengineTos
