#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketPolicyTest : public ::testing::Test {
protected:
    BucketPolicyTest() = default;

    ~BucketPolicyTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = "sdktestpolicybucket";
        TestUtils::CreateBucket(cliV2, bucketName);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bucketName);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bucketName;
    static std::string Policy;
};

std::shared_ptr<TosClientV2> BucketPolicyTest::cliV2 = nullptr;
std::string BucketPolicyTest::bucketName = "";
std::string BucketPolicyTest::Policy =
        "{\"Statement\":[{\"Action\":[\"*\"],\"Effect\":\"Allow\",\"Principal\":\"*\","
        "\"Resource\":[\"trn:tos:::sdktestpolicybucket/*\",\"trn:tos:::sdktestpolicybucket\"],\"Sid\":\"internal public\"}]}";
TEST_F(BucketPolicyTest, BucketPolicyWithoutParametersTest) {
    PutBucketPolicyInput putBucketPolicyInput(bucketName);
    auto putOutput = cliV2->putBucketPolicy(putBucketPolicyInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "invalid policy, the policy must be not empty");
}

TEST_F(BucketPolicyTest, BucketPolicyWithALLParametersTest) {
    PutBucketPolicyInput putBucketPolicyInput(bucketName);
    putBucketPolicyInput.setPolicy(Policy);
    auto putOutput = cliV2->putBucketPolicy(putBucketPolicyInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketPolicyInput getBucketPolicyInput(bucketName);
    auto getOutput = cliV2->getBucketPolicy(getBucketPolicyInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    EXPECT_EQ(getOutput.result().getPolicy().empty(), false);

    DeleteBucketPolicyInput deleteBucketPolicyInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketPolicy(deleteBucketPolicyInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getBucketPolicy(getBucketPolicyInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucketPolicy");
}

TEST_F(BucketPolicyTest, BucketPolicyWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketPolicyInput putBucketPolicyInput(bucketName_, Policy);
    auto putOutput = cliV2->putBucketPolicy(putBucketPolicyInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetBucketPolicyInput getBucketPolicyInput(bucketName_);
    auto getOutput = cliV2->getBucketPolicy(getBucketPolicyInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
    DeleteBucketPolicyInput deleteBucketPolicyInput(bucketName_);
    auto deleteOutput = cliV2->deleteBucketPolicy(deleteBucketPolicyInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
