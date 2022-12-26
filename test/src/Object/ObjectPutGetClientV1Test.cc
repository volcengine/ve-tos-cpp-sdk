#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include "../../../sdk/src/auth/SignV4.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectPutGetClientV1Test : public ::testing::Test {
protected:
    ObjectPutGetClientV1Test() {
    }

    ~ObjectPutGetClientV1Test() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
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

std::shared_ptr<TosClientV2> ObjectPutGetClientV1Test::cliV2 = nullptr;
std::string ObjectPutGetClientV1Test::bkt_name = "";

TEST_F(ObjectPutGetClientV1Test, PutObjectWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./ :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);

    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss);

    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto output_obj_get = cliV2->getObject(bkt_name, obj_key);

    auto content_output = output_obj_get.result().getContent();
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
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectPutGetClientV1Test, PutZeroSizeObjectTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}
TEST_F(ObjectPutGetClientV1Test, PutObjectWithErrorNameTest) {
    std::string error_message = "invalid object name, the length must be [1, 696]";
    std::string obj_key = "";
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);

    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == error_message, true);
}
TEST_F(ObjectPutGetClientV1Test, PutObjectWithErrorMd5Test) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./ :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    std::string data2 = "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcde";
    std::string dataMd5 = CryptoUtils::md5Sum(data2);
    RequestOptionBuilder rob;
    rob.withContentMD5(dataMd5);
    auto ss = std::make_shared<std::stringstream>(data);
    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss, rob);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The Content-MD5 you specified did not match what we received.",
              true);
}

// acl设置和grant设置是冲突的
// TEST_F(ObjectPutGetClientV1Test, PutObjectWithSSECTest) {
//    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    std::string data = "hoxnu1jii3u4h1h7cezrst3hpd8xv465";
//    // 设置全部参数
//    RequestOptionBuilder rob;
//    std::string dataMd5 = CryptoUtils::md5Sum(data);
//    rob.withHeader(http::HEADER_CONTENT_MD5, dataMd5);
//    rob.withHeader(http::HEADER_CACHE_CONTROL, "no-cache");
//    rob.withHeader(http::HEADER_EXPIRES, "Mon, 04 Jul 2022 02:57:31 GMT");
//    rob.withHeader(http::HEADER_CONTENT_DISPOSITION, "attachment; filename=123.txt");
//    rob.withHeader(http::HEADER_CONTENT_ENCODING, "gzip");
//    rob.withHeader(http::HEADER_CONTENT_LANGUAGE, "en-US");
//    rob.withHeader(HEADER_ACL, "authenticated-read");
//
//    std::string ssecKey = "hoxnu1jii3u4h1h7cezrst3hpd8xv465";
//    std::string ssecKeyMd5 = CryptoUtils::md5Sum(ssecKey);
//    rob.withServerSideEncryptionCustomer("AES256", "aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=", ssecKeyMd5);
//
//    //  rob.withServerSideEncryption("AES256");
//    // rob.withStorageClass(StorageClassType::STANDARD);
//
//    auto ss = std::make_shared<std::stringstream>(data);
//    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss, rob);
//    EXPECT_EQ(output_obj_put.isSuccess(), true);
//
//    GetObjectV2Input input_obj_get(bkt_name, obj_key);
//    input_obj_get.setSsecKeyMd5(ssecKeyMd5);
//    input_obj_get.setSsecAlgorithm("AES256");
//    input_obj_get.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
//    auto output_obj_get = cliV2->getObject(input_obj_get);
//    std::ostringstream ss_;
//    ss_ << output_obj_get.result().getContent()->rdbuf();
//    std::string tmp_string = ss_.str();
//
//    bool check_data = (data == tmp_string);
//    auto meta = output_obj_get.result().getGetObjectBasicOutput().getMeta();
//    EXPECT_EQ(check_data, true);
//    EXPECT_EQ(output_obj_get.isSuccess(), true);
//}

TEST_F(ObjectPutGetClientV1Test, PutObjectToNoExistingBucketTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./ :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    auto output_obj_put = cliV2->putObject(bkt_name_, obj_key, ss);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getStatusCode(), 404);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectPutGetClientV1Test, GetObjectWithNoExistingNameTest) {
    std::string obj_key = "111";
    auto res_obj_get = cliV2->getObject(bkt_name, obj_key);
    EXPECT_EQ(res_obj_get.isSuccess(), false);
    EXPECT_EQ(res_obj_get.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get.error().getMessage() == "The specified key does not exist.", true);

    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    auto res_obj_get_ = cliV2->getObject(bkt_name_, obj_key);
    EXPECT_EQ(res_obj_get_.isSuccess(), false);
    EXPECT_EQ(res_obj_get_.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get_.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectPutGetClientV1Test, GetObjectWithRangeTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    RequestOptionBuilder rob_get;
    int64_t begin = 10;
    int64_t end = 119;
    int length_range = end - begin + 1;
    rob_get.withRange(begin, end);

    auto res_obj_get = cliV2->getObject(bkt_name, obj_key, rob_get);
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
    bool content_length_compare = (data.compare(begin, length_range, tmp_string) == 0);
    bool content_compare = (data.substr(begin, length_range) == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}

}  // namespace VolcengineTos
