#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#ifdef _WIN32
#define  timegm(a)  ::_mkgmtime64(a)
#endif  // _WIN32
namespace VolcengineTos {
class BucketLifecycleTest : public ::testing::Test {
protected:
    BucketLifecycleTest() = default;

    ~BucketLifecycleTest() override = default;

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

std::shared_ptr<TosClientV2> BucketLifecycleTest::cliV2 = nullptr;
std::string BucketLifecycleTest::bucketName = "";

TEST_F(BucketLifecycleTest, BucketLifecycleWithoutParametersTest) {
    PutBucketLifecycleInput putBucketLifecycleInput(bucketName);
    auto putOutput = cliV2->putBucketLifecycle(putBucketLifecycleInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "invalid rules, the rules must be not empty");
}

TEST_F(BucketLifecycleTest, BucketLifecycleWithALLParametersTest) {
    PutBucketLifecycleInput putBucketLifecycleInput(bucketName);
    time_t now = time(nullptr);
    tm* gmtm = gmtime(&now);
    gmtm->tm_min = 0;
    gmtm->tm_hour = 0;
    gmtm->tm_sec = 0;
    auto t_1 = timegm(gmtm);
    time_t expirationTime = time(nullptr);
    tm* gmtmExpiration = gmtime(&expirationTime);
    gmtmExpiration->tm_min = 0;
    gmtmExpiration->tm_hour = 0;
    gmtmExpiration->tm_sec = 0;
    gmtmExpiration->tm_year = gmtmExpiration->tm_year + 1;
    auto t_2 = timegm(gmtmExpiration);
    LifecycleRule rule1("1", "test", StatusType::StatusEnabled, {{3, StorageClassType::IA}},
                        std::make_shared<Expiration>(10), {{30, StorageClassType::IA}},
                        std::make_shared<NoncurrentVersionExpiration>(70), {{"1", "2"}}, nullptr);
    LifecycleRule rule2("2", "rule2", StatusType::StatusEnabled, {{t_1, StorageClassType::IA}},
                        std::make_shared<Expiration>(t_2), {{40, StorageClassType::IA}},
                        std::make_shared<NoncurrentVersionExpiration>(50), {{"3", "4"}, {"4", "5"}}, nullptr);
    putBucketLifecycleInput.setRules({rule1, rule2});
    auto putOutput = cliV2->putBucketLifecycle(putBucketLifecycleInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetBucketLifecycleInput getBucketLifecycleInput(bucketName);
    auto getOutput = cliV2->getBucketLifecycle(getBucketLifecycleInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto resRules = getOutput.result().getRules();
    auto resRule0 = resRules[0];
    auto resRule1 = resRules[1];

    bool checkRes = (resRules.size() == 2 && resRule0.getId() == "1" && resRule1.getPrefix() == "rule2" &&
                     resRule0.getTransitions()[0].getStorageClass() == StorageClassType::IA &&
                     resRule0.getTransitions()[0].getDays() == 3 &&
                     resRule1.getTransitions()[0].getStorageClass() == StorageClassType::IA &&
                     resRule1.getTransitions()[0].getDate() == t_1 && resRule0.getExpiratioon()->getDays() == 10 &&
                     resRule1.getExpiratioon()->getDate() == t_2 &&
                     resRule0.getNoncurrentVersionTransitions()[0].getStorageClass() == StorageClassType::IA &&
                     resRule0.getNoncurrentVersionTransitions()[0].getNoncurrentDays() == 30 &&
                     resRule0.getTags()[0].getKey() == "1" && resRule0.getTags()[0].getValue() == "2");
    EXPECT_EQ(checkRes, true);

    DeleteBucketLifecycleInput deleteBucketLifecycleInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketLifecycle(deleteBucketLifecycleInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getBucketLifecycle(getBucketLifecycleInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchLifecycleConfiguration");
}

TEST_F(BucketLifecycleTest, BucketLifecycleWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketLifecycleInput putBucketLifecycleInput(bucketName_);
    LifecycleRule rule1("1", "test", StatusType::StatusEnabled, {{3, StorageClassType::IA}},
                        std::make_shared<Expiration>(10), {{30, StorageClassType::IA}},
                        std::make_shared<NoncurrentVersionExpiration>(70), {{"1", "2"}}, nullptr);
    putBucketLifecycleInput.setRules({rule1});
    auto putOutput = cliV2->putBucketLifecycle(putBucketLifecycleInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetBucketLifecycleInput getBucketLifecycleInput(bucketName_);
    auto getOutput = cliV2->getBucketLifecycle(getBucketLifecycleInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
    DeleteBucketLifecycleInput deleteBucketLifecycleInput(bucketName_);
    auto deleteOutput = cliV2->deleteBucketLifecycle(deleteBucketLifecycleInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchBucket");
}

}  // namespace VolcengineTos
