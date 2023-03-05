#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketVersioningTest : public ::testing::Test {
protected:
    BucketVersioningTest() = default;

    ~BucketVersioningTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
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
};

std::shared_ptr<TosClientV2> BucketVersioningTest::cliV2 = nullptr;
std::string BucketVersioningTest::bucketName = "";

TEST_F(BucketVersioningTest, BucketVersioningWithALLParametersTest) {
    PutBucketVersioningInput putBucketVersioningInput(bucketName, VersioningStatusType::Enabled);
    auto putOutput = cliV2->putBucketVersioning(putBucketVersioningInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    TimeUtils::sleepSecondTimes(30);
    GetBucketVersioningInput getBucketVersioningInput(bucketName);
    auto getOutput = cliV2->getBucketVersioning(getBucketVersioningInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    EXPECT_EQ(getOutput.result().getStatus(), VersioningStatusType::Enabled);

    putBucketVersioningInput.setStatus(VersioningStatusType::Suspended);
    auto putOutput1 = cliV2->putBucketVersioning(putBucketVersioningInput);
    EXPECT_EQ(putOutput1.isSuccess(), true);

    TimeUtils::sleepSecondTimes(30);
    auto getOutput1 = cliV2->getBucketVersioning(getBucketVersioningInput);
    EXPECT_EQ(getOutput1.isSuccess(), true);
    EXPECT_EQ(getOutput1.result().getStatus(), VersioningStatusType::Suspended);
}

TEST_F(BucketVersioningTest, BucketVersioningWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketVersioningInput putBucketVersioningInput(bucketName_, VersioningStatusType::Enabled);
    auto putOutput = cliV2->putBucketVersioning(putBucketVersioningInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetBucketVersioningInput getBucketVersioningInput(bucketName_);
    auto getOutput = cliV2->getBucketVersioning(getBucketVersioningInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
}

// TEST_F(BucketVersioningTest, asasdasd) {
//
//     GetBucketVersioningInput getBucketVersioningInput("sdk-testcrr-01");
//     auto getOutput = cliV2->getBucketVersioning(getBucketVersioningInput);
//     EXPECT_EQ(getOutput.isSuccess(), true);
// }

}  // namespace VolcengineTos
