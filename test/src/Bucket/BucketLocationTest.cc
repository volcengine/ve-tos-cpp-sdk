#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketLocationTest : public ::testing::Test {
protected:
    BucketLocationTest() = default;

    ~BucketLocationTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        region = TestConfig::Region;
        endPoint = TestConfig::Endpoint;
        conf.endPoint = endPoint;
        cliV2 = std::make_shared<TosClientV2>(region, TestConfig::Ak, TestConfig::Sk, conf);
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
    static std::string region;
    static std::string endPoint;
};

std::shared_ptr<TosClientV2> BucketLocationTest::cliV2 = nullptr;
std::string BucketLocationTest::bucketName = "";
std::string BucketLocationTest::region = "";
std::string BucketLocationTest::endPoint = "";

TEST_F(BucketLocationTest, BucketLocationWithALLParametersTest) {
    GetBucketLocationInput getBucketLocationInput(bucketName);
    auto getOutput = cliV2->getBucketLocation(getBucketLocationInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    EXPECT_EQ(getOutput.result().getRegion() == region, true);
    std::string host_ = "";
    if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTPS)) {
        host_ = endPoint.substr(std::strlen(http::SchemeHTTPS) + 3,
                                endPoint.length() - std::strlen(http::SchemeHTTPS) - 3);
    } else if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTP)) {
        host_ = endPoint.substr(std::strlen(http::SchemeHTTP) + 3,
                                endPoint.length() - std::strlen(http::SchemeHTTP) - 3);
    } else {
        host_ = endPoint;
    }
    EXPECT_EQ(getOutput.result().getExtranetEndpoint() == host_, true);
}

TEST_F(BucketLocationTest, BucketLocationWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix) + "111s";
    GetBucketLocationInput getBucketLocationInput(bucketName_);
    auto getOutput = cliV2->getBucketLocation(getBucketLocationInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
