#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketCORSTest : public ::testing::Test {
protected:
    BucketCORSTest() = default;

    ~BucketCORSTest() override = default;

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

std::shared_ptr<TosClientV2> BucketCORSTest::cliV2 = nullptr;
std::string BucketCORSTest::bucketName = "";

TEST_F(BucketCORSTest, BucketCORSWithoutParametersTest) {
    PutBucketCORSInput putBucketCorsInput(bucketName);
    auto putOutput = cliV2->putBucketCORS(putBucketCorsInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "invalid rules, the rules must be not empty");
}

TEST_F(BucketCORSTest, BucketCORSWithALLParametersTest) {
    PutBucketCORSInput putBucketCorsInput(bucketName);
    CORSRule rule1("1", {"*"}, {"GET", "DELETE", "PUT"}, {"Authorization"}, {"X-TOS-HEADER-1", "X-TOS-HEADER-2"}, 3600);
    CORSRule rule2("2", {"https://www.volcengine.com"}, {"PUT", "POST"}, {"Authorization"},
                   {"X-TOS-HEADER-1", "X-TOS-HEADER-2"}, 3600);
    putBucketCorsInput.setRules({rule1, rule2});
    auto putOutput = cliV2->putBucketCORS(putBucketCorsInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketCORSInput getBucketCorsInput(bucketName);
    auto getOutput = cliV2->getBucketCORS(getBucketCorsInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto resRule = getOutput.result().getRules();
    bool checkRes = (resRule[0].getAllowedMethods().size() == 3 && resRule[0].getExposeHeaders().size() == 2 &&
                     resRule.size() == 2 && resRule[0].getAllowedOrigins().size() == 1 &&
                     resRule[0].getAllowedHeaders().size() == 1 && (resRule[1].getAllowedMethods().size() == 2) &&
                     resRule[0].getMaxAgeSeconds() == 3600);
    EXPECT_EQ(checkRes, true);

    DeleteBucketCORSInput deleteBucketCorsInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketCORS(deleteBucketCorsInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getBucketCORS(getBucketCorsInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchCORSConfiguration");
}

TEST_F(BucketCORSTest, BucketCORSWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketCORSInput putBucketCorsInput(bucketName_);
    CORSRule rule1("1", {"*"}, {"GET", "DELETE", "PUT"}, {"Authorization"}, {"X-TOS-HEADER-1", "X-TOS-HEADER-2"}, 3600);
    CORSRule rule2("2", {"https://www.volcengine.com"}, {"PUT", "POST"}, {"Authorization"},
                   {"X-TOS-HEADER-1", "X-TOS-HEADER-2"}, 3600);
    putBucketCorsInput.setRules({rule1, rule2});
    auto putOutput = cliV2->putBucketCORS(putBucketCorsInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetBucketCORSInput getBucketCorsInput(bucketName_);
    auto getOutput = cliV2->getBucketCORS(getBucketCorsInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
    DeleteBucketCORSInput deleteBucketCorsInput(bucketName_);
    auto deleteOutput = cliV2->deleteBucketCORS(deleteBucketCorsInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
