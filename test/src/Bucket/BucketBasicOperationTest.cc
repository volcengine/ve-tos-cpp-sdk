#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketBasicOperationTest : public ::testing::Test {
protected:
    BucketBasicOperationTest() {
    }

    ~BucketBasicOperationTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV1 = std::make_shared<TosClient>(TestConfig::Endpoint, TestConfig::Region, TestConfig::Ak, TestConfig::Sk);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        cliV2 = nullptr;
        cliV1 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::shared_ptr<TosClient> cliV1;
};

std::shared_ptr<TosClientV2> BucketBasicOperationTest::cliV2 = nullptr;
std::shared_ptr<TosClient> BucketBasicOperationTest::cliV1 = nullptr;

TEST_F(BucketBasicOperationTest, CreateBucketTest) {
    std::string bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);

    CreateBucketV2Input input(bkt_name);
    auto output = cliV2->createBucket(input);
    EXPECT_EQ(output.isSuccess(), true);

    DeleteBucketInput input_delete(bkt_name);
    auto output_delete = cliV2->deleteBucket(input_delete);
    EXPECT_EQ(output_delete.isSuccess(), true);
}

}  // namespace VolcengineTos
