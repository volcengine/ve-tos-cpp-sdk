#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include "../../../sdk/src/auth/SignV4.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectPutGetTest : public ::testing::Test {
protected:
    ObjectPutGetTest() {
    }

    ~ObjectPutGetTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
        conf.maxRetryCount = 0;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);

        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
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
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ObjectPutGetTest::cliV2 = nullptr;
std::string ObjectPutGetTest::bkt_name = "";
std::string ObjectPutGetTest::workPath = "";

TEST_F(ObjectPutGetTest, PutObjectWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);

    std::string data2 =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss2 = std::make_shared<std::stringstream>(data2);
    input_obj_put.setContent(ss2);
    auto output_obj_put2 = cliV2->putObject(input_obj_put);

    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = res_obj_get.result().getGetObjectBasicOutput();
    auto content_output = res_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    bool length_compare = (tmp_string.size() == data.size());
    bool content_length_compare = (basic_output.getContentLength() == data.length());
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectPutGetTest, PutZeroSizeObjectTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}
TEST_F(ObjectPutGetTest, PutObjectWithErrorNameTest) {
    std::string error_message = "invalid object name, the length must be [1, 696]";
    std::string obj_key = "";
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == error_message, true);
}
TEST_F(ObjectPutGetTest, PutObjectWithErrorMd5Test) {
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

// todo:boe4服务端加密会超时
TEST_F(ObjectPutGetTest, PutObjectWithSSECTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";

    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key);
    input_obj_put.setContent(ss);
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();

    // 设置全部参数
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    input_obj_put_basic.setContentMd5(dataMd5);
    input_obj_put_basic.setCacheControl("no-cache");
    std::time_t expires = TestUtils::GetTimeWithDelay(10);
    input_obj_put_basic.setExpires(expires);
    input_obj_put_basic.setContentDisposition("attachment;filename=\" 中文.txt\"");
    input_obj_put_basic.setContentEncoding("gzip");
    input_obj_put_basic.setContentLanguage("en-US");
    //    input_obj_put_basic.setAcl(ACLType::Private);
    input_obj_put_basic.setGrantFullControl("id=123");
    input_obj_put_basic.setGrantRead("id=123");
    input_obj_put_basic.setGrantReadAcp("id=123");
    input_obj_put_basic.setGrantWriteAcp("id=123");
    std::map<std::string, std::string> meta_{{"中文key1中文", ""}, {"中文key2中文", "中文value2中文"}};
    input_obj_put_basic.setMeta(meta_);

    input_obj_put_basic.setSsecAlgorithm("AES256");
    std::string ssecKey = "hoxnu1jii3u4h1h7cezrst3hpd8xv465";
    std::string ssecKeyMd5 = CryptoUtils::md5Sum(ssecKey);
    input_obj_put_basic.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
    input_obj_put_basic.setSsecKeyMd5(ssecKeyMd5);

    // todo:time_out
    //    input_obj_put_basic.setServerSideEncryption("AES256");

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_key);
    input_obj_get.setSsecKeyMd5(ssecKeyMd5);
    input_obj_get.setSsecAlgorithm("AES256");
    input_obj_get.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
    auto output_obj_get = cliV2->getObject(input_obj_get);
    std::ostringstream ss_;
    ss_ << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss_.str();

    bool check_data = (data == tmp_string);
    auto meta = output_obj_get.result().getGetObjectBasicOutput().getMeta();
    bool check_meta = ((meta["中文key1中文"] == "") && (meta["中文key2中文"] == "中文value2中文"));
    EXPECT_EQ(check_data & check_meta, true);
    EXPECT_EQ(output_obj_get.isSuccess(), true);
}

TEST_F(ObjectPutGetTest, PutObjectToNoExistingBucketTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name_);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getStatusCode(), 404);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectPutGetTest, GetObjectWithNoExistingNameTest) {
    std::string obj_key = "111";
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get.isSuccess(), false);
    EXPECT_EQ(res_obj_get.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get.error().getMessage() == "The specified key does not exist.", true);

    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_obj_get.setBucket(bkt_name_);
    auto res_obj_get_ = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get_.isSuccess(), false);
    EXPECT_EQ(res_obj_get_.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get_.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectPutGetTest, GetObjectWithRangeTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    int64_t begin = 10;
    int64_t end = 119;
    int length_range = end - begin + 1;

    input_obj_get.setRangeStart(begin);
    input_obj_get.setRangeEnd(end);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = res_obj_get.result().getGetObjectBasicOutput();
    auto content_output = res_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();
    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    bool length_compare = (tmp_string.size() == length_range);
    bool content_length_compare = data.compare(begin, length_range - 1, tmp_string);
    bool content_compare = (data.substr(begin, length_range) == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}

TEST_F(ObjectPutGetTest, PutGetObjectFromFileTest) {
    auto workPath = FileUtils::getWorkPath();
    std::cout << workPath << std::endl;
    std::string filePath1 = workPath + "test/testdata/" + "PutObjectTest.txt";
    std::string filePath2 = workPath + "test/testdata/" + "GetObjectTest.txt";

    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectFromFileInput input_obj_put(bkt_name, obj_key, filePath1);
    auto output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectToFileInput input_obj_get(bkt_name, obj_key, filePath2);
    auto out_obj_get = cliV2->getObjectToFile(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
}

// TEST_F(ObjectPutGetTest, AppendGetObjectWithPartNumberTest) {
//     std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//     CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
//
//     auto upload = cliV2->createMultipartUpload(input_part_create);
//
//     // generate some data..
//     auto ss1 = std::make_shared<std::stringstream>();
//     for (int i = 0; i < (5 << 20); ++i) {
//         *ss1 << 1;
//     }
//     UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
//     auto part1 = cliV2->uploadPart(input_upload_part);
//     EXPECT_EQ(part1.isSuccess(), true);
//
//     // generate some data..
//     auto ss2 = std::make_shared<std::stringstream>();
//     for (int i = 0; i < (4 << 20); ++i) {
//         *ss2 << 2;
//     }
//     UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
//     auto part2 = cliV2->uploadPart(input_upload_part_2);
//     EXPECT_EQ(part2.isSuccess(), true);
//     auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
//     auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
//     std::vector<UploadedPartV2> vec = {part_1, part_2};
//     CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
//     auto com = cliV2->completeMultipartUpload(input_upload_complete);
//     EXPECT_EQ(com.isSuccess(), true);
//
//     int data_size_1 = (4 << 20);
//     int data_size_2 = (5 << 20);
//     std::string data_1 = std::string(data_size_1, '2');
//     std::string data_2 = std::string(data_size_2, '1');
//
//     GetObjectV2Input input_obj_get(bkt_name, obj_name);
//     input_obj_get.setPartNumber(2);
//     auto out_obj_get = cliV2->getObject(input_obj_get);
//     std::ostringstream ssOutput1;
//     ssOutput1 << out_obj_get.result().getContent()->rdbuf();
//     std::string tmpString1 = ssOutput1.str();
//     bool lengthCompare = (data_size_1 == tmpString1.size());
//     bool contentCompare = (data_1 == tmpString1);
//     EXPECT_EQ(lengthCompare, true);
//     EXPECT_EQ(contentCompare, true);
//
//     input_obj_get.setPartNumber(1);
//     auto getObjectOutput = cliV2->getObject(input_obj_get);
//     auto getObjectBasicOutput = getObjectOutput.result().getGetObjectBasicOutput();
//     auto contentOutput = getObjectOutput.result().getContent();
//
//     std::ostringstream ssOutput2;
//     ssOutput2 << getObjectOutput.result().getContent()->rdbuf();
//     std::string tmpString2 = ssOutput2.str();
//
//     bool lengthCompare2 = (data_size_2 == tmpString2.size());
//     bool contentCompare2 = (data_2 == tmpString2);
//     EXPECT_EQ(lengthCompare2, true);
//     EXPECT_EQ(contentCompare2, true);
// }

}  // namespace VolcengineTos
