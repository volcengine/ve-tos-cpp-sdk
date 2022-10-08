#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketDeleteTest : public ::testing::Test {
protected:
    BucketDeleteTest() {
    }

    ~BucketDeleteTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;

        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV1 = std::make_shared<TosClient>(TestConfig::Endpoint, TestConfig::Region, TestConfig::Ak, TestConfig::Sk);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        cliV2 = nullptr;
        cliV1 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::shared_ptr<TosClient> cliV1;
};
std::shared_ptr<TosClientV2> BucketDeleteTest::cliV2 = nullptr;
std::shared_ptr<TosClient> BucketDeleteTest::cliV1 = nullptr;

TEST_F(BucketDeleteTest, DeleteExistentBucketTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketV2Input input_v2;
    input_v2.setBucket(bkt);

    auto output_v2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2.isSuccess(), true);

    DeleteBucketInput input_v2_delete;
    input_v2_delete.setBucket(bkt);
    auto output_v2_delete = cliV2->deleteBucket(input_v2_delete);
    EXPECT_EQ(output_v2.isSuccess(), true);
}
TEST_F(BucketDeleteTest, DeleteNonExistentBucketTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    HeadBucketV2Input input_v2_head;
    input_v2_head.setBucket(bkt);
    auto output_v2_head = cliV2->headBucket(input_v2_head);
    EXPECT_EQ(output_v2_head.isSuccess(), false);

    DeleteBucketInput input_v2_delete;
    input_v2_delete.setBucket(bkt);
    auto output_v2_delete = cliV2->deleteBucket(input_v2_delete);
    EXPECT_EQ(output_v2_delete.isSuccess(), false);
}

// Client V1 Test
TEST_F(BucketDeleteTest, DeleteExistentBucketClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketInput input_v1;
    input_v1.setBucket(bkt);

    auto output_v1 = cliV1->createBucket(input_v1);
    EXPECT_EQ(output_v1.isSuccess(), true);

    auto output_v1_delete = cliV1->deleteBucket(bkt);
    EXPECT_EQ(output_v1.isSuccess(), true);
}
TEST_F(BucketDeleteTest, DeleteNonExistentBucketClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    auto output_v1_head = cliV1->headBucket(bkt);
    EXPECT_EQ(output_v1_head.isSuccess(), false);

    auto output_v1_delete = cliV1->deleteBucket(bkt);
    EXPECT_EQ(output_v1_delete.isSuccess(), false);
}

}  // namespace VolcengineTos
