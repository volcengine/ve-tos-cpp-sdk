#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectMetaDataClientV1Test : public ::testing::Test {
protected:
    ObjectMetaDataClientV1Test() {
    }

    ~ObjectMetaDataClientV1Test() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV1 = std::make_shared<TosClient>(TestConfig::Endpoint, TestConfig::Region, TestConfig::Ak, TestConfig::Sk);
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
    static std::shared_ptr<TosClient> cliV1;

    static std::string bkt_name;
    static std::string obj_name;
};

std::shared_ptr<TosClientV2> ObjectMetaDataClientV1Test::cliV2 = nullptr;
std::shared_ptr<TosClient> ObjectMetaDataClientV1Test::cliV1 = nullptr;

std::string ObjectMetaDataClientV1Test::bkt_name = "";
std::string ObjectMetaDataClientV1Test::obj_name = "";

TEST_F(ObjectMetaDataClientV1Test, PutHeadSetObjectMetaTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "1234567890";
    auto ss = std::make_shared<std::stringstream>(data);

    RequestOptionBuilder rob_put;
    rob_put.withContentType("text/plain");
    rob_put.withCacheControl("no-cache");
    rob_put.withContentDisposition("111.txt");
    rob_put.withMeta("self-test", "yes");
    auto output_obj_put = cliV1->putObject(bkt_name, obj_key, ss, rob_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto output_obj_head = cliV1->headObject(bkt_name, obj_key);
    EXPECT_EQ(output_obj_head.isSuccess(), true);
    bool check_content_type = (output_obj_head.result().getObjectMeta().getContentType() == "text/plain");
    bool check_cache_control = (output_obj_head.result().getObjectMeta().getCacheControl() == "no-cache");
    bool check_content_disposition = (output_obj_head.result().getObjectMeta().getContentDisposition() == "111.txt");
    auto meta = output_obj_head.result().getObjectMeta().getMetadata();
    bool check_meta_data = (meta["self-test"] == "yes");
    EXPECT_EQ(check_content_type & check_cache_control & check_content_disposition & check_meta_data, true);

    RequestOptionBuilder rob;
    rob.withContentType("application/json");
    rob.withCacheControl("no-store");
    rob.withContentDisposition("222.txt");
    rob.withMeta("self-test", "no");
    auto output_obj_set_meta = cliV1->setObjectMeta(bkt_name, obj_key, rob);
    EXPECT_EQ(output_obj_set_meta.isSuccess(), true);

    auto output_obj_head_ = cliV1->headObject(bkt_name, obj_key);
    EXPECT_EQ(output_obj_head_.isSuccess(), true);
    bool check_content_type_ = (output_obj_head_.result().getObjectMeta().getContentType() == "application/json");
    bool check_cache_control_ = (output_obj_head_.result().getObjectMeta().getCacheControl() == "no-store");
    bool check_content_disposition_ = (output_obj_head_.result().getObjectMeta().getContentDisposition() == "222.txt");
    auto meta_ = output_obj_head_.result().getObjectMeta().getMetadata();
    bool check_meta_data_ = (meta_["self-test"] == "no");
    EXPECT_EQ(check_content_type_ & check_cache_control_ & check_content_disposition_ & check_meta_data_, true);
}

TEST_F(ObjectMetaDataClientV1Test, HeadObjectMetaFromNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto output = cliV1->headObject(bkt_name_, obj_key_);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "not found", true);

    auto output_ = cliV1->headObject(bkt_name, obj_key_);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getCode() == "not found", true);
}
TEST_F(ObjectMetaDataClientV1Test, SetObjectMetaFromNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    RequestOptionBuilder rob;
    auto output = cliV1->setObjectMeta(bkt_name_, obj_key_, rob);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "NoSuchBucket", true);

    auto output_ = cliV1->setObjectMeta(bkt_name, obj_key_, rob);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    //    EXPECT_EQ(output_.error().getCode() == "NotFound", true);
}

TEST_F(ObjectMetaDataClientV1Test, DeleteAndHeadObjectMetaTest) {
    TestUtils::PutObject(cliV2, bkt_name, obj_name, "111");

    auto output = cliV1->deleteObject(bkt_name, obj_name);
    EXPECT_EQ(output.isSuccess(), true);
    RequestOptionBuilder rob;
    auto output_ = cliV1->setObjectMeta(bkt_name, obj_name, rob);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
}

TEST_F(ObjectMetaDataClientV1Test, DeleteNonexistentObjectTest) {
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto output = cliV1->deleteObject(bkt_name, obj_key_);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(ObjectMetaDataClientV1Test, DeleteObjectFromNonexistentBucketTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto output = cliV1->deleteObject(bkt_name_, obj_key_);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getCode() == "NoSuchBucket", true);
}
}  // namespace VolcengineTos
