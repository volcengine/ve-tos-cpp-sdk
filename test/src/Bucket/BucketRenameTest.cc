#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketRenameTest : public ::testing::Test {
protected:
    BucketRenameTest() = default;

    ~BucketRenameTest() override = default;

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
    static std::string Policy;
};

std::shared_ptr<TosClientV2> BucketRenameTest::cliV2 = nullptr;
std::string BucketRenameTest::bucketName = "";
TEST_F(BucketRenameTest, BucketRenameTest) {
    PutBucketRenameInput putBucketRenameInput(bucketName, true);
    auto putOutput = cliV2->putBucketRename(putBucketRenameInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    TimeUtils::sleepSecondTimes(92);

    GetBucketRenameInput getBucketRenameInput(bucketName);
    auto getOutput = cliV2->getBucketRename(getBucketRenameInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    EXPECT_EQ(getOutput.result().isRenameEnable(), true);

    std::string data = TestUtils::GetRandomString(1024);
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectV2Input putObjectInput(bucketName, obj_key, ss);
    auto output_obj_put = cliV2->putObject(putObjectInput);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    RenameObjectInput renameObjectInput(bucketName, obj_key, "new");
    auto output = cliV2->renameObject(renameObjectInput);
    EXPECT_EQ(output.isSuccess(), true);

    GetObjectV2Input input(bucketName, obj_key);
    auto res = cliV2->getObject(input);
    EXPECT_EQ(res.isSuccess(), false);

    input.setKey("new");
    res = cliV2->getObject(input);
    EXPECT_EQ(res.isSuccess(), true);

    DeleteBucketRenameInput deleteBucketRenameInput(bucketName);
    auto deleteOutput = cliV2->deleteBucketRename(deleteBucketRenameInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
}
}  // namespace VolcengineTos
