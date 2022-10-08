#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class Md5Crc64DataVerifyTest : public ::testing::Test {
protected:
    Md5Crc64DataVerifyTest() {
    }

    ~Md5Crc64DataVerifyTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
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

std::shared_ptr<TosClientV2> Md5Crc64DataVerifyTest::cliV2 = nullptr;
std::string Md5Crc64DataVerifyTest::bkt_name = "";

TEST_F(Md5Crc64DataVerifyTest, PutObjectMd5Crc64VerifyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectV2Input input_obj_put(bkt_name, obj_name);
    // 校验Crc64
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    auto ss1 = std::make_shared<std::stringstream>(data);
    input_obj_put.setContent(ss);
    auto out_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(out_obj_put.isSuccess(), true);

    // 校验Md5，提供正确的Md5，校验成功
    auto basic_input = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss1);
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    basic_input.setContentMd5(dataMd5);
    input_obj_put.setPutObjectBasicInput(basic_input);
    auto out_obj_put_1 = cliV2->putObject(input_obj_put);
    EXPECT_EQ(out_obj_put_1.isSuccess(), true);
}
TEST_F(Md5Crc64DataVerifyTest, PutObjectWithErrorMd5Test) {
    // 校验Md5，提供错误的Md5，校验失败
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    std::string data2 = "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcde";
    std::string dataMd5 = CryptoUtils::md5Sum(data2);

    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);

    input_obj_put_basic.setContentMd5(dataMd5);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The Content-MD5 you specified did not match what we received.",
              true);
}

TEST_F(Md5Crc64DataVerifyTest, AppendObjectMd5Crc64VerifyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_name, part0, 0);

    // 校验Crc64
    auto output = cliV2->appendObject(input_append);
    EXPECT_EQ(output.isSuccess(), true);
    //    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "2";
    }
    input_append.setContent(part1);
    input_append.setOffset(output.result().getNextAppendOffset());
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    //    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output1.isSuccess(), true);
}
TEST_F(Md5Crc64DataVerifyTest, UploadPartMd5Crc64VerifyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);

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
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
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
    input_upload_part_2.setUploadPartBasicInput(basic_input_2);
    auto part2 = cliV2->uploadPart(input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);

    // part_3数据准备
    auto ss3 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss3 << 2;
    }
    // 校验Md5，提供错误的Md5，校验失败
    UploadPartV2Input input_upload_part_3(bkt_name, obj_name, upload.result().getUploadId(), ss3->tellg(), 3, ss3);
    auto basic_input_3 = input_upload_part_3.getUploadPartBasicInput();
    std::string dataMd5_ = CryptoUtils::md5Sum(data_2 + " 1");
    basic_input_3.setContentMd5(dataMd5_);
    input_upload_part_3.setUploadPartBasicInput(basic_input_3);
    auto part3 = cliV2->uploadPart(input_upload_part_3);
    EXPECT_EQ(part3.isSuccess(), false);
}
TEST_F(Md5Crc64DataVerifyTest, GetObjectCrc64VerifyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    TestUtils::PutObject(cliV2, bkt_name, obj_name, "111111111111");

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto output = cliV2->getObject(input_obj_get);
    auto content = output.result().getContent();
    content->seekg(0, content->end);
    int64_t size = content->tellg();
    EXPECT_EQ(output.result().getGetObjectBasicOutput().getContentLength() == size, true);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(Md5Crc64DataVerifyTest, GetObjectCutOffVerifyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    TestUtils::PutObject(cliV2, bkt_name, obj_name, data);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    int64_t begin = 10;
    int64_t end = 119;
    int length_range = end - begin + 1;

    input_obj_get.setRangeStart(begin);
    input_obj_get.setRangeEnd(end);

    auto output = cliV2->getObject(input_obj_get);
    auto content = output.result().getContent();
    content->seekg(0, content->end);
    int64_t size = content->tellg();
    EXPECT_EQ(output.result().getGetObjectBasicOutput().getContentLength() == size, true);
    EXPECT_EQ(output.isSuccess(), true);
}

}  // namespace VolcengineTos
