#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketReplicationTest : public ::testing::Test {
protected:
    BucketReplicationTest() = default;

    ~BucketReplicationTest() override = default;

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

std::shared_ptr<TosClientV2> BucketReplicationTest::cliV2 = nullptr;
std::string BucketReplicationTest::bucketName = "";

TEST_F(BucketReplicationTest, BucketReplicationWithALLParametersTest) {
    PutBucketReplicationInput putBucketReplicationInput(bucketName);
    ReplicationRule rule1("1", StatusType::StatusEnabled, {"prefix1", "prefix2"},
                          Destination{"sdk-testcrr-01", TestConfig::RegionCrr, StorageClassType::IA,
                                      StorageClassInheritDirectiveType::SourceObject},
                          StatusType::StatusEnabled);
    ReplicationRule rule2("2", StatusType::StatusEnabled, {"prefix3", "prefix4"},
                          Destination{"sdk-testcrr-02", TestConfig::RegionCrr, StorageClassType::STANDARD,
                                      StorageClassInheritDirectiveType::SourceObject},
                          StatusType::StatusDisabled);
    putBucketReplicationInput.setRules({rule1, rule2});
    putBucketReplicationInput.setRole("ServiceRoleforReplicationAccessTOS");
    auto putOutput = cliV2->putBucketReplication(putBucketReplicationInput);
    EXPECT_EQ(putOutput.isSuccess(), true);
    TimeUtils::sleepSecondTimes(92);
    GetBucketReplicationInput getBucketReplicationInput(bucketName, "1");
    auto getOutput = cliV2->getBucketReplication(getBucketReplicationInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto resRule = getOutput.result().getRules();
    bool checkRes = (resRule[0].getId() == "1" && resRule[0].getPrefixSet().size() == 2 &&
                     resRule[0].getProgress() != nullptr && resRule[0].getStatus() == rule1.getStatus() &&
                     resRule[0].getHistoricalObjectReplication() == rule1.getHistoricalObjectReplication() &&
                     resRule[0].getDestination().getBucket() == rule1.getDestination().getBucket() &&
                     resRule[0].getDestination().getStorageClass() == rule1.getDestination().getStorageClass() &&
                     resRule[0].getDestination().getLocation() == rule1.getDestination().getLocation() &&
                     resRule[0].getDestination().getStorageClassInheritDirective() ==
                             rule1.getDestination().getStorageClassInheritDirective());
    EXPECT_EQ(checkRes, true);
    GetBucketReplicationInput getBucketReplicationInput2(bucketName, "2");
    getOutput = cliV2->getBucketReplication(getBucketReplicationInput2);
    resRule = getOutput.result().getRules();
    bool checkRes2 = (resRule[0].getId() == "2" && resRule[0].getPrefixSet().size() == 2 &&
                      resRule[0].getProgress() != nullptr && resRule[0].getStatus() == rule2.getStatus() &&
                      resRule[0].getHistoricalObjectReplication() == rule2.getHistoricalObjectReplication() &&
                      resRule[0].getDestination().getBucket() == rule2.getDestination().getBucket() &&
                      resRule[0].getDestination().getStorageClass() == rule2.getDestination().getStorageClass() &&
                      resRule[0].getDestination().getLocation() == rule2.getDestination().getLocation() &&
                      resRule[0].getDestination().getStorageClassInheritDirective() ==
                              rule2.getDestination().getStorageClassInheritDirective());
    EXPECT_EQ(checkRes2, true);

    DeleteBucketReplicationInput deleteBucketReplicationInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketReplication(deleteBucketReplicationInput);
    EXPECT_EQ(deleteOutput.isSuccess(), true);

    getOutput = cliV2->getBucketReplication(getBucketReplicationInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucketReplication");
}

}  // namespace VolcengineTos
