#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketRealTimeLogTest : public ::testing::Test {
protected:
    BucketRealTimeLogTest() = default;

    ~BucketRealTimeLogTest() override = default;

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

std::shared_ptr<TosClientV2> BucketRealTimeLogTest::cliV2 = nullptr;
std::string BucketRealTimeLogTest::bucketName = "";

TEST_F(BucketRealTimeLogTest, BucketRealTimeLogWithALLParametersTest) {
    AccessLogConfiguration accessLogConfiguration;
    accessLogConfiguration.setUseServiceTopic(true);
    RealTimeLogConfiguration realTimeLogConfiguration("TOSLogArchiveTLSRole", accessLogConfiguration);

    PutBucketRealTimeLogInput putBucketRealTimeLogInput(bucketName, realTimeLogConfiguration);

    auto putOutput = cliV2->putBucketRealTimeLog(putBucketRealTimeLogInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketRealTimeLogInput getBucketRealTimeLogInput(bucketName);
    auto getOutput = cliV2->getBucketRealTimeLog(getBucketRealTimeLogInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto config = getOutput.result().getConfiguration();
    EXPECT_EQ(config.getRole() == "TOSLogArchiveTLSRole", true);
    EXPECT_EQ(config.getConfiguration().isUseServiceTopic(), true);
    EXPECT_EQ(config.getConfiguration().getTlsTopicId().empty(), false);
    EXPECT_EQ(config.getConfiguration().getTlsProjectId().empty(), false);

    DeleteBucketRealTimeLogInput deleteBucketRealTimeLogInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketRealTimeLog(deleteBucketRealTimeLogInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getBucketRealTimeLog(getBucketRealTimeLogInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucketRealTimeLog");
}

}  // namespace VolcengineTos
