#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketNotificationTest : public ::testing::Test {
protected:
    BucketNotificationTest() = default;

    ~BucketNotificationTest() override = default;

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

std::shared_ptr<TosClientV2> BucketNotificationTest::cliV2 = nullptr;
std::string BucketNotificationTest::bucketName = "";

TEST_F(BucketNotificationTest, BucketNotificationWithALLParametersTest) {
    FilterKey filterKey({{"prefix", "object"}, {"suffix", "object1"}});
    Filter filter(filterKey);
    CloudFunctionConfiguration configuration("test-id", {"tos:ObjectCreated:Put"}, filter, "p66zhthn");
    PutBucketNotificationInput putBucketNotificationInput(bucketName, {configuration});
    auto putOutput = cliV2->putBucketNotification(putBucketNotificationInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketNotificationInput getBucketNotificationInput(bucketName);
    auto getOutput = cliV2->getBucketNotification(getBucketNotificationInput);

    EXPECT_EQ(getOutput.isSuccess(), true);
    auto res = getOutput.result().getCloudFunctionConfigurations();
    bool check = res[0].getId() == configuration.getId() &&
                 res[0].getCloudFunction() == configuration.getCloudFunction() &&
                 res[0].getEvents() == configuration.getEvents() &&
                 res[0].getFilter().getKey().getRules()[0].getName() == filterKey.getRules()[0].getName() &&
                 res[0].getFilter().getKey().getRules()[0].getValue() == filterKey.getRules()[0].getValue() &&
                 res[0].getFilter().getKey().getRules()[1].getName() == filterKey.getRules()[1].getName() &&
                 res[0].getFilter().getKey().getRules()[1].getValue() == filterKey.getRules()[1].getValue();
    EXPECT_EQ(check, true);
}

}  // namespace VolcengineTos
