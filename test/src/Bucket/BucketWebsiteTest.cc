#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketWebsiteTest : public ::testing::Test {
protected:
    BucketWebsiteTest() = default;

    ~BucketWebsiteTest() override = default;

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

std::shared_ptr<TosClientV2> BucketWebsiteTest::cliV2 = nullptr;
std::string BucketWebsiteTest::bucketName = "";

TEST_F(BucketWebsiteTest, BucketWebsiteWithALLParametersTest) {
    PutBucketWebsiteInput putBucketWebsiteInput(bucketName);
    RedirectAllRequestsTo redirectAllRequestsTo("www.volcengine.com", "https");
    IndexDocument indexDocument("index.html", true);
    ErrorDocument errorDocument("error.html");

    RoutingRuleCondition condition("website", 404);
    RoutingRuleRedirect redirect(ProtocolType::Http, "www.volcengine.com", "voc", 301);
    RoutingRule rule1(condition, redirect);

    RoutingRuleCondition condition2("website", 404);
    RoutingRuleRedirect redirect2(ProtocolType::Https, "www.volcengine.com", "voc", 301);
    RoutingRule rule2(condition2, redirect2);

    putBucketWebsiteInput.setRedirectAllRequestsTo(redirectAllRequestsTo);
    auto putOutput = cliV2->putBucketWebsite(putBucketWebsiteInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketWebsiteInput getBucketWebsiteInput(bucketName);
    auto getOutput = cliV2->getBucketWebsite(getBucketWebsiteInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto res = getOutput.result();
    EXPECT_EQ(res.getErrorDocument() == nullptr, true);
    EXPECT_EQ(res.getIndexDocument() == nullptr, true);
    EXPECT_EQ(res.getRoutingRules().empty(), true);
    EXPECT_EQ(res.getRedirectAllRequestsTo()->getProtocol() == "https" &&
                      res.getRedirectAllRequestsTo()->getHostName() == "www.volcengine.com",
              true);
    DeleteBucketWebsiteInput deleteBucketWebsiteInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketWebsite(deleteBucketWebsiteInput);
    EXPECT_EQ(deleteOutput.isSuccess(), true);

    getOutput = cliV2->getBucketWebsite(getBucketWebsiteInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchWebsiteConfiguration");

    PutBucketWebsiteInput putBucketWebsiteInput2(bucketName);
    putBucketWebsiteInput2.setIndexDocument(indexDocument);
    putBucketWebsiteInput2.setErrorDocument(errorDocument);
    putBucketWebsiteInput2.setRoutingRules({rule1, rule2});
    auto putOutput2 = cliV2->putBucketWebsite(putBucketWebsiteInput2);
    EXPECT_EQ(putOutput.isSuccess(), true);

    auto getOutput2 = cliV2->getBucketWebsite(getBucketWebsiteInput);
    EXPECT_EQ(getOutput2.isSuccess(), true);
    res = getOutput2.result();
    EXPECT_EQ(res.getErrorDocument()->getKey() == "error.html", true);
    EXPECT_EQ(
            res.getIndexDocument()->getSuffix() == "index.html" && res.getIndexDocument()->isForbiddenSubDir() == true,
            true);
    auto resRule = res.getRoutingRules();
    bool check = (resRule[0].getRedirect().getHostName() == "www.volcengine.com") &&
                 (resRule[1].getRedirect().getHostName() == "www.volcengine.com") &&
                 (resRule[0].getRedirect().getReplaceKeyPrefixWith() == "voc") &&
                 (resRule[1].getRedirect().getReplaceKeyPrefixWith() == "voc") &&
                 (resRule[0].getRedirect().getHttpRedirectCode() == 301) &&
                 (resRule[1].getRedirect().getHttpRedirectCode() == 301) &&
                 (resRule[0].getRedirect().getProtocolType() == ProtocolType::Http) &&
                 (resRule[1].getRedirect().getProtocolType() == ProtocolType::Https) &&
                 (resRule[0].getCondition().getHttpErrorCodeReturnedEquals() == 404) &&
                 (resRule[1].getCondition().getHttpErrorCodeReturnedEquals() == 404) &&
                 (resRule[0].getCondition().getKeyPrefixEquals() == "website") &&
                 (resRule[1].getCondition().getKeyPrefixEquals() == "website");
    EXPECT_EQ(check, true);
    EXPECT_EQ(res.getRedirectAllRequestsTo() == nullptr, true);

    auto deleteOutput2 = cliV2->deleteBucketWebsite(deleteBucketWebsiteInput);
    EXPECT_EQ(deleteOutput2.isSuccess(), true);
}

}  // namespace VolcengineTos
