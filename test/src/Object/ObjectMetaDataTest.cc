#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectMetaDataTest : public ::testing::Test {
protected:
    ObjectMetaDataTest() {
    }

    ~ObjectMetaDataTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
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
    static std::string obj_name;
};

std::shared_ptr<TosClientV2> ObjectMetaDataTest::cliV2 = nullptr;
std::string ObjectMetaDataTest::bkt_name = "";
std::string ObjectMetaDataTest::obj_name = "";

TEST_F(ObjectMetaDataTest, PutHeadSetObjectMetaTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "12345678901111111111111";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto basic_input = input_obj_put.getPutObjectBasicInput();
    basic_input.setContentType("text/plain");
    basic_input.setCacheControl("no-cache");
    basic_input.setContentDisposition("111.txt");
    std::map<std::string, std::string> meta_1{{"self-test", "yes"}};
    basic_input.setMeta(meta_1);
    input_obj_put.setPutObjectBasicInput(basic_input);

    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    HeadObjectV2Input input_obj_head(bkt_name, obj_key);
    auto output_obj_head = cliV2->headObject(input_obj_head);
    EXPECT_EQ(output_obj_head.isSuccess(), true);
    bool check_content_type = (output_obj_head.result().getContentType() == "text/plain");
    bool check_cache_control = (output_obj_head.result().getCacheControl() == "no-cache");
    bool check_content_disposition = (output_obj_head.result().getContentDisposition() == "111.txt");
    auto meta = output_obj_head.result().getMeta();
    bool check_meta_data = (meta["self-test"] == "yes");
    EXPECT_EQ(check_content_type & check_cache_control & check_content_disposition & check_meta_data, true);

    SetObjectMetaInput input_obj_set_meta(bkt_name, obj_key);

    input_obj_set_meta.setContentType("application/json");
    input_obj_set_meta.setCacheControl("no-store");
    input_obj_set_meta.setContentDisposition("222.txt");
    std::map<std::string, std::string> meta_2{{"self-test", "no"}};
    input_obj_set_meta.setMeta(meta_2);
    auto output_obj_set_meta = cliV2->setObjectMeta(input_obj_set_meta);
    EXPECT_EQ(output_obj_set_meta.isSuccess(), true);

    auto output_obj_head_ = cliV2->headObject(input_obj_head);
    EXPECT_EQ(output_obj_head_.isSuccess(), true);
    bool check_content_type_ = (output_obj_head_.result().getContentType() == "application/json");
    bool check_cache_control_ = (output_obj_head_.result().getCacheControl() == "no-store");
    bool check_content_disposition_ = (output_obj_head_.result().getContentDisposition() == "222.txt");
    auto meta_ = output_obj_head_.result().getMeta();
    bool check_meta_data_ = (meta_["self-test"] == "no");
    EXPECT_EQ(check_content_type_ & check_cache_control_ & check_content_disposition_ & check_meta_data_, true);
}

TEST_F(ObjectMetaDataTest, HeadObjectMetaFromNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    HeadObjectV2Input input_obj_head(bkt_name_, obj_key_);
    auto output = cliV2->headObject(input_obj_head);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "not found", true);

    input_obj_head.setBucket(bkt_name);
    auto output_ = cliV2->headObject(input_obj_head);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getCode() == "not found", true);
}
TEST_F(ObjectMetaDataTest, SetObjectMetaFromNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    SetObjectMetaInput input_obj_set_meta(bkt_name_, obj_key_);
    auto output = cliV2->setObjectMeta(input_obj_set_meta);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "NoSuchBucket", true);

    input_obj_set_meta.setBucket(bkt_name);
    auto output_ = cliV2->setObjectMeta(input_obj_set_meta);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    //    EXPECT_EQ(output_.error().getCode() == "NotFound", true);
}
TEST_F(ObjectMetaDataTest, DeleteAndHeadObjectMetaTest) {
    TestUtils::PutObject(cliV2, bkt_name, obj_name, "111");
    DeleteObjectInput input_obj_delete(bkt_name, obj_name);
    auto output = cliV2->deleteObject(input_obj_delete);
    EXPECT_EQ(output.isSuccess(), true);
    SetObjectMetaInput input_obj_set_meta(bkt_name, obj_name);
    auto output_ = cliV2->setObjectMeta(input_obj_set_meta);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
}

TEST_F(ObjectMetaDataTest, DeleteNonexistentObjectTest) {
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    DeleteObjectInput input_obj_delete(bkt_name, obj_key_);
    auto output = cliV2->deleteObject(input_obj_delete);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(ObjectMetaDataTest, DeleteObjectFromNonexistentBucketTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    DeleteObjectInput input_obj_delete(bkt_name_, obj_key_);
    auto output = cliV2->deleteObject(input_obj_delete);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "NoSuchBucket", true);
}

TEST_F(ObjectMetaDataTest, URIEncodeFuncTest) {
    auto out = CryptoUtils::UrlEncodeChinese("!@#$%^&*()_+-=[]{}|;':\",./<>?中文测试abcd /\\");
    EXPECT_EQ(
            "%21%40%23%24%25%5E%26%2A%28%29_%2B-%3D%5B%5D%7B%7D%7C%3B%27%3A%22%2C.%2F%3C%3E%3F%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95abcd%20%2F%5C",
            out);
    auto out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("!@#$%^&*()_+-=[]{}|;':\",./<>?中文测试abcd /\\", out2);

    out = CryptoUtils::UrlEncodeChinese("23i23+___");
    EXPECT_EQ("23i23%2B___", out);
    out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("23i23+___", out2);

    out = CryptoUtils::UrlEncodeChinese("23i23 ___");
    EXPECT_EQ("23i23%20___", out);
    out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("23i23 ___", out2);

    out = CryptoUtils::UrlEncodeChinese("23i23 /___");
    EXPECT_EQ("23i23%20%2F___", out);
    out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("23i23 /___", out2);

    out = CryptoUtils::UrlEncodeChinese("23i23 /___");
    EXPECT_EQ("23i23%20%2F___", out);
    out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("23i23 /___", out2);

    out = CryptoUtils::UrlEncodeChinese("/中文测试/");
    EXPECT_EQ("%2F%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95%2F", out);
    out2 = CryptoUtils::UrlDecodeChinese(out);
    EXPECT_EQ("/中文测试/", out2);

    out2 = CryptoUtils::UrlDecodeChinese("23i23%%20%2F___");
    EXPECT_EQ("23i23%%20%2F___", out2);
}
}  // namespace VolcengineTos
