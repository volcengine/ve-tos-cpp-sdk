#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class RestoreObjectTest : public ::testing::Test {
protected:
    RestoreObjectTest() = default;

    ~RestoreObjectTest() override = default;

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
    static std::string host_;
};

std::shared_ptr<TosClientV2> RestoreObjectTest::cliV2 = nullptr;
std::string RestoreObjectTest::bucketName = "";
std::string RestoreObjectTest::host_ = "";

TEST_F(RestoreObjectTest, RestoreObjectTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data = TestUtils::GetRandomString(1024);
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input putObjectInput(bucketName, obj_key, ss);
    putObjectInput.setStorageClass(StorageClassType::COLD_ARCHIVE);
    auto output_obj_put = cliV2->putObject(putObjectInput);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    RestoreObjectInput input(bucketName, obj_key, 10, RestoreJobParameters(TierType::TierExpedited));
    auto output = cliV2->restoreObject(input);
    EXPECT_EQ(output.isSuccess(), true);
}

}  // namespace VolcengineTos
