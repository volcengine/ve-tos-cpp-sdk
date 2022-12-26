#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketCustomDomainTest : public ::testing::Test {
protected:
    BucketCustomDomainTest() = default;

    ~BucketCustomDomainTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
        // TestUtils::CleanAllBucket(cliV2);

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

std::shared_ptr<TosClientV2> BucketCustomDomainTest::cliV2 = nullptr;
std::string BucketCustomDomainTest::bucketName = "";

TEST_F(BucketCustomDomainTest, BucketCustomDomainWithALLParametersTest) {
    PutBucketCustomDomainInput putBucketCustomDomainInput(bucketName);
    CustomDomainRule rule1("www.baidu.com");
    rule1.setCertStatus(CertStatusType::Bound);
    rule1.setForbidden(true);
    rule1.setForbiddenReason("only test");

    putBucketCustomDomainInput.setRules(rule1);
    auto putOutput = cliV2->putBucketCustomDomain(putBucketCustomDomainInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    ListBucketCustomDomainInput listBucketCustomDomainInput(bucketName);
    auto getOutput = cliV2->listBucketCustomDomain(listBucketCustomDomainInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    EXPECT_EQ(getOutput.result().getRules().size(), 1);

    DeleteBucketCustomDomainInput deleteBucketCustomDomainInput(bucketName, "www.baidu.com");
    auto deleteOutput = cliV2->deleteBucketCustomDomain(deleteBucketCustomDomainInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->listBucketCustomDomain(listBucketCustomDomainInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
}

}  // namespace VolcengineTos
