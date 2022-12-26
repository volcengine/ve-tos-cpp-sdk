#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectCopyTest : public ::testing::Test {
protected:
    ObjectCopyTest() {
    }

    ~ObjectCopyTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        //        TestUtils::CleanAllBucket(cliV2);
        src_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        src_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "/%2ss";
        data = "1234567890";
        TestUtils::CreateBucket(cliV2, src_bkt_name);

        std::map<std::string, std::string> meta{{"self-test", "yes"}};
        TestUtils::PutObjectWithMeta(cliV2, src_bkt_name, src_obj_name, data, meta);

        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "/%2ss";
        TestUtils::CreateBucket(cliV2, bkt_name);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, src_bkt_name);
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string src_bkt_name;
    static std::string src_obj_name;
    static std::string bkt_name;
    static std::string obj_name;
    static std::string data;
};

std::shared_ptr<TosClientV2> ObjectCopyTest::cliV2 = nullptr;
std::string ObjectCopyTest::src_bkt_name = "";
std::string ObjectCopyTest::src_obj_name = "";
std::string ObjectCopyTest::bkt_name = "";
std::string ObjectCopyTest::obj_name = "";
std::string ObjectCopyTest::data = "";

TEST_F(ObjectCopyTest, CopyObjectToOtherBucketTest) {
    CopyObjectV2Input input_object_copy(bkt_name, obj_name, src_bkt_name, src_obj_name);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
}

TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithSSECTest) {
    auto obj_name_ = obj_name + "ssec";
    auto src_obj_name_ = src_obj_name + "ssec";

    auto ss = std::make_shared<std::stringstream>();
    *ss << data;

    PutObjectV2Input input_obj_put(src_bkt_name, src_obj_name_, ss);
    std::string ssecKey = "hoxnu1jii3u4h1h7cezrst3hpd8xv465";
    std::string ssecKeyBase64 =
            CryptoUtils::base64Encode(reinterpret_cast<const unsigned char*>(ssecKey.c_str()), ssecKey.length());
    std::string ssecKeyMd5 = CryptoUtils::md5Sum(ssecKey);
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put_basic.setSsecAlgorithm("AES256");
    input_obj_put_basic.setSsecKey(ssecKeyBase64);  // key base64
    input_obj_put_basic.setSsecKeyMd5(ssecKeyMd5);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto srcPut = cliV2->putObject(input_obj_put);
    EXPECT_EQ(srcPut.isSuccess(), true);

    CopyObjectV2Input input_object_copy(bkt_name, obj_name_, src_bkt_name, src_obj_name_);
    input_object_copy.setCopySourceSsecAlgorithm("AES256");
    input_object_copy.setCopySourceSsecKey(ssecKeyBase64);
    input_object_copy.setCopySourceSsecKeyMd5(ssecKeyMd5);
    input_object_copy.setSsecAlgorithm("AES256");
    input_object_copy.setSsecKey(ssecKeyBase64);
    input_object_copy.setSsecKeyMd5(ssecKeyMd5);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name_);
    input_obj_get.setSsecKeyMd5(ssecKeyMd5);
    input_obj_get.setSsecAlgorithm("AES256");
    input_obj_get.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);

    std::ostringstream ss1;
    ss1 << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss1.str();
    bool check_data = (data == tmp_string);
    EXPECT_EQ(check_data, true);
}

TEST_F(ObjectCopyTest, CopyObjectToCurrentBucketTest) {
    CopyObjectV2Input input_object_copy(src_bkt_name, obj_name, src_bkt_name, src_obj_name);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, src_bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, src_bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}

TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithUnmodifiedSinceParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfUnmodifiedSince(TestUtils::GetTimeWithDelay(100));
    input_object_copy.setAcl(ACLType::PublicReadWrite);
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name_, obj_name_);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name_, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name_, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}

TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithIfMatchParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfMatch("\"e807f1fcf82d132f9bb018ca6738a19f\"");
    input_object_copy.setAcl(ACLType::PublicReadWrite);
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name_, obj_name_);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name_, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name_, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithIfNoneMatchParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfNoneMatch("\"e807f1fcf82d132f9bb018c111119f\"");
    input_object_copy.setAcl(ACLType::PublicReadWrite);
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name_, obj_name_);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name_, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name_, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCopyTest, CopyObjectToCurrentBucketWithUnmodifiedSinceParamTest) {
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(src_bkt_name, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfUnmodifiedSince(TestUtils::GetTimeWithDelay(100));
    input_object_copy.setAcl(ACLType::Private);
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, src_bkt_name, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, src_bkt_name, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}

TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithSinceParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix) + "111";
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfModifiedSince(TestUtils::GetTimeWithDelay(-100));
    input_object_copy.setGrantWriteAcp("id=1234");
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name_, obj_name_);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name_, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name_, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCopyTest, CopyObjectToCurrentBucketWithSinceParamTest) {
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(src_bkt_name, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfModifiedSince(TestUtils::GetTimeWithDelay(-100));
    input_object_copy.setGrantRead("id=123");
    input_object_copy.setGrantReadAcp("id=123");
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, src_bkt_name, obj_name_);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, src_bkt_name, obj_name_);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}
TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithDelaySinceParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfModifiedSince(TestUtils::GetTimeWithDelay(100));
    input_object_copy.setGrantWriteAcp("id=1234");
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), false);
    GetObjectV2Input input_obj_get(bkt_name_, obj_name);
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_get.isSuccess(), false);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCopyTest, CopyObjectToOtherBucketWithNoneIfMatchParamTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CopyObjectV2Input input_object_copy(bkt_name_, obj_name_, src_bkt_name, src_obj_name);
    input_object_copy.setCopySourceIfMatch("not match md5");
    input_object_copy.setGrantWriteAcp("id=1234");
    input_object_copy.setWebsiteRedirectLocation("/anotherObjectName");
    input_object_copy.setStorageClass(StorageClassType::IA);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), false);
    GetObjectV2Input input_obj_get(bkt_name_, obj_name);
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_get.isSuccess(), false);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCopyTest, CopyObjectWithCopyStrategyTest) {
    CopyObjectV2Input input_object_copy(bkt_name, obj_name, src_bkt_name, src_obj_name);
    input_object_copy.setMetadataDirective(COPY);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    bool check_func = (tmp_string == tmp_string_2);
    EXPECT_EQ(check_data & check_func & check_meta, true);
}
TEST_F(ObjectCopyTest, CopyObjectWithReplaceStrategyTest) {
    CopyObjectV2Input input_object_copy(bkt_name, obj_name, src_bkt_name, src_obj_name);
    input_object_copy.setMetadataDirective(REPLACE);
    std::map<std::string, std::string> meta_{{"self-test", "no"}};
    input_object_copy.setMeta(meta_);
    auto output_obj_copy = cliV2->copyObject(input_object_copy);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::string tmp_string_2 = TestUtils::GetObjectContentByStream(cliV2, bkt_name, obj_name);

    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_func = (tmp_string == tmp_string_2);
    bool check_meta = (meta["self-test"] == "no");
    EXPECT_EQ(check_data & check_func & check_meta, true);
}
TEST_F(ObjectCopyTest, CopyObjectWithNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string src_bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string src_obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    CopyObjectV2Input input_object_copy_0(bkt_name_, obj_name, src_bkt_name, src_obj_name);
    auto output_obj_copy_0 = cliV2->copyObject(input_object_copy_0);
    EXPECT_EQ(output_obj_copy_0.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_0.error().getStatusCode(), 404);

    CopyObjectV2Input input_object_copy_1(bkt_name, obj_name, src_bkt_name_, src_obj_name);
    auto output_obj_copy_1 = cliV2->copyObject(input_object_copy_1);
    EXPECT_EQ(output_obj_copy_1.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_1.error().getStatusCode(), 404);

    CopyObjectV2Input input_object_copy_2(bkt_name, obj_name, src_bkt_name, src_obj_name_);
    auto output_obj_copy_2 = cliV2->copyObject(input_object_copy_2);
    EXPECT_EQ(output_obj_copy_2.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_2.error().getStatusCode(), 404);
}
}  // namespace VolcengineTos
