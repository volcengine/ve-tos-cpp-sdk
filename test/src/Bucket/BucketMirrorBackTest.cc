#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketMirrorBackTest : public ::testing::Test {
protected:
    BucketMirrorBackTest() = default;

    ~BucketMirrorBackTest() override = default;

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

std::shared_ptr<TosClientV2> BucketMirrorBackTest::cliV2 = nullptr;
std::string BucketMirrorBackTest::bucketName = "";

TEST_F(BucketMirrorBackTest, BucketMirrorBackWithoutParametersTest) {
    PutBucketMirrorBackInput putBucketMirrorBackInput(bucketName);
    auto putOutput = cliV2->putBucketMirrorBack(putBucketMirrorBackInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "invalid rules, the rules must be not empty");
}

TEST_F(BucketMirrorBackTest, BucketMirrorBackWithParametersTest) {
    PutBucketMirrorBackInput putBucketMirrorBackInput(bucketName);
    Condition condition{404};
    MirrorHeader mirrorHeader{true, {"aa", "bb"}, {"xxx"}};
    PublicSource publicSource{{{"http://www.volcengine.com/obj/tostest/"}, {"http://www.volcengine.com/obj/tostest/"}}};
    Redirect redirect{RedirectType::RedirectMirror, true, true, true, mirrorHeader, publicSource};
    MirrorBackRule rule1("1", condition, redirect);
    // 当前仅支持 1 个规则
    putBucketMirrorBackInput.setRules({rule1});
    auto putOutput = cliV2->putBucketMirrorBack(putBucketMirrorBackInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketMirrorBackInput getBucketMirrorBackInput(bucketName);
    auto getOutput = cliV2->getBucketMirrorBack(getBucketMirrorBackInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto res = getOutput.result().getRules()[0];
    auto resRedirect = res.getRedirect();
    bool resCheck = (res.getId() == "1" && res.getCondition().getHttpCode() == 404 && resRedirect.isFollowRedirect() &&
                     resRedirect.getRedirectType() == RedirectType::RedirectMirror &&
                     resRedirect.getMirrorHeader().getPass()[0] == "aa" &&
                     resRedirect.getPublicSource().getSourceEndpoint().getPrimary()[0] ==
                             "http://www.volcengine.com/obj/tostest/");
    EXPECT_EQ(resCheck, true);
    DeleteBucketMirrorBackInput deleteBucketMirrorBackInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketMirrorBack(deleteBucketMirrorBackInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getBucketMirrorBack(getBucketMirrorBackInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchMirrorConfiguration");
}

TEST_F(BucketMirrorBackTest, BucketMirrorBackWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketMirrorBackInput putBucketMirrorBackInput(bucketName_);
    Condition condition{404};
    MirrorHeader mirrorHeader{true, {"aa", "bb"}, {"xxx"}};
    PublicSource publicSource{{{"http://www.volcengine.com/obj/tostest/"}, {"http://www.volcengine.com/obj/tostest/"}}};
    Redirect redirect{RedirectType::RedirectMirror, true, true, true, mirrorHeader, publicSource};
    MirrorBackRule rule1("1", condition, redirect);

    putBucketMirrorBackInput.setRules({rule1});
    auto putOutput = cliV2->putBucketMirrorBack(putBucketMirrorBackInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetBucketMirrorBackInput getBucketMirrorBackInput(bucketName_);
    auto getOutput = cliV2->getBucketMirrorBack(getBucketMirrorBackInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
    DeleteBucketMirrorBackInput deleteBucketMirrorBackInput(bucketName_);
    auto deleteOutput = cliV2->deleteBucketMirrorBack(deleteBucketMirrorBackInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
