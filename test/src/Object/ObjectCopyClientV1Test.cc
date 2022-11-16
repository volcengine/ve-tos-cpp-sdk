#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectCopyClientV1Test : public ::testing::Test {
protected:
    ObjectCopyClientV1Test() = default;

    ~ObjectCopyClientV1Test() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);

        src_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        src_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
        data = "1234567890";
        TestUtils::CreateBucket(cliV2, src_bkt_name);

        std::map<std::string, std::string> meta{{"self-test", "yes"}};
        TestUtils::PutObjectWithMeta(cliV2, src_bkt_name, src_obj_name, data, meta);

        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bkt_name);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        TestUtils::CleanBucket(cliV2, src_bkt_name);
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

std::shared_ptr<TosClientV2> ObjectCopyClientV1Test::cliV2 = nullptr;

std::string ObjectCopyClientV1Test::src_bkt_name = "";
std::string ObjectCopyClientV1Test::src_obj_name = "";
std::string ObjectCopyClientV1Test::bkt_name = "";
std::string ObjectCopyClientV1Test::obj_name = "";
std::string ObjectCopyClientV1Test::data = "";

TEST_F(ObjectCopyClientV1Test, CopyObjectToOtherBucketTest) {
    auto output_obj_copy = cliV2->copyObjectTo(src_bkt_name, bkt_name, obj_name, src_obj_name);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}

TEST_F(ObjectCopyClientV1Test, CopyObjectToCurrentBucketTest) {
    auto output_obj_copy = cliV2->copyObject(src_bkt_name, src_obj_name, obj_name);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, src_bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, src_bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}

TEST_F(ObjectCopyClientV1Test, CopyObjectWithCopyStrategyTest) {
    RequestOptionBuilder rob;
    rob.withMetadataDirective("COPY");

    auto output_obj_copy = cliV2->copyObjectTo(src_bkt_name, bkt_name, obj_name, src_obj_name, rob);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(check_data & check_meta, true);
}
TEST_F(ObjectCopyClientV1Test, CopyObjectWithReplaceStrategyTest) {
    RequestOptionBuilder rob;
    rob.withMetadataDirective("REPLACE");
    rob.withMeta("self-test", "no");
    auto output_obj_copy = cliV2->copyObjectTo(src_bkt_name, bkt_name, obj_name, src_obj_name, rob);
    EXPECT_EQ(output_obj_copy.isSuccess(), true);
    std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name);
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);
    bool check_data = (data == tmp_string);
    bool check_meta = (meta["self-test"] == "no");
    EXPECT_EQ(check_data & check_meta, true);
}
TEST_F(ObjectCopyClientV1Test, CopyObjectWithNonexistentNameTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string src_bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string src_obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto output_obj_copy_0 = cliV2->copyObjectTo(src_bkt_name, bkt_name_, obj_name, src_obj_name);
    EXPECT_EQ(output_obj_copy_0.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_0.error().getStatusCode(), 404);

    auto output_obj_copy_1 = cliV2->copyObjectTo(src_bkt_name_, bkt_name, obj_name, src_obj_name);
    EXPECT_EQ(output_obj_copy_1.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_1.error().getStatusCode(), 404);

    auto output_obj_copy_2 = cliV2->copyObjectTo(src_bkt_name, bkt_name, obj_name, src_obj_name_);
    EXPECT_EQ(output_obj_copy_2.isSuccess(), false);
    EXPECT_EQ(output_obj_copy_2.error().getStatusCode(), 404);
}
}  // namespace VolcengineTos
