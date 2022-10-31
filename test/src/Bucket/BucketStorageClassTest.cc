#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketStorageClassTest : public ::testing::Test {
protected:
    BucketStorageClassTest() = default;

    ~BucketStorageClassTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix) + "storagetype";
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

std::shared_ptr<TosClientV2> BucketStorageClassTest::cliV2 = nullptr;
std::string BucketStorageClassTest::bucketName = "";

TEST_F(BucketStorageClassTest, BucketStorageClassWithoutParametersTest) {
    PutBucketStorageClassInput putBucketStorageClassInput(bucketName);
    auto putOutput = cliV2->putBucketStorageClass(putBucketStorageClassInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "The storage class you specified is not valid");
}

TEST_F(BucketStorageClassTest, BucketStorageClassWithALLParametersTest) {
    HeadBucketV2Input headBucketV2Input(bucketName);
    auto headOutput = cliV2->headBucket(headBucketV2Input);
    EXPECT_EQ(headOutput.isSuccess(), true);
    EXPECT_EQ(headOutput.result().getStorageClass() != StorageClassType::IA, true);

    PutBucketStorageClassInput putBucketStorageClassInput(bucketName, StorageClassType::IA);
    auto putOutput = cliV2->putBucketStorageClass(putBucketStorageClassInput);
    EXPECT_EQ(putOutput.isSuccess(), true);
    TimeUtils::sleepSecondTimes(10);
    auto headOutput2 = cliV2->headBucket(headBucketV2Input);
    EXPECT_EQ(headOutput2.isSuccess(), true);
    EXPECT_EQ(headOutput2.result().getStorageClass() == StorageClassType::IA, true);
}

TEST_F(BucketStorageClassTest, BucketStorageClassWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketStorageClassInput putBucketStorageClassInput(bucketName_, StorageClassType::STANDARD);
    auto putOutput = cliV2->putBucketStorageClass(putBucketStorageClassInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
