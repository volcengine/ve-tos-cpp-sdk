#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketCreateHeadClientV1Test : public ::testing::Test {
protected:
    BucketCreateHeadClientV1Test() = default;

    ~BucketCreateHeadClientV1Test() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        cliV2 = nullptr;
    }

    Outcome<TosError, CreateBucketOutput> CreateBucketV1(CreateBucketInput& input) {
        auto output_v1 = cliV2->createBucket(input);
        if (!output_v1.isSuccess()) {
            std::cout << output_v1.error().getMessage() << std::endl;
        }
        return output_v1;
    }
    Outcome<TosError, DeleteBucketOutput> DeleteBucketV1(const std::string& bkt_name) {
        auto output_v1_delete = cliV2->deleteBucket(bkt_name);
        if (!output_v1_delete.isSuccess()) {
            std::cout << output_v1_delete.error().getMessage() << std::endl;
        }
        return output_v1_delete;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
};
std::shared_ptr<TosClientV2> BucketCreateHeadClientV1Test::cliV2 = nullptr;

// client V1 test
TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithBucketNameClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketInput input_v1;
    input_v1.setBucket(bkt);

    auto output_v1 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1.isSuccess(), true);

    auto output_v1_head = cliV2->headBucket(bkt);
    EXPECT_EQ(output_v1_head.isSuccess(), true);
    bool regionCheck = (output_v1_head.result().getRegion() == TestConfig::Region);
    bool storageClassCheck = (output_v1_head.result().getStorageClass() == "STANDARD");
    EXPECT_EQ(regionCheck, true);
    EXPECT_EQ(storageClassCheck, true);

    auto output_v1_delete = cliV2->deleteBucket(bkt);
    EXPECT_EQ(output_v1_delete.isSuccess(), true);
}
TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithParametersClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketInput input_v1;
    input_v1.setBucket(bkt);
    //    input_v2.setAcl(PublicRead);
    //    input_v2.setGrantFullControl("id=123");
    //    input_v2.setGrantRead("id=123");
    //    input_v2.setGrantReadAcp("id=123,id=456");
    //    input_v2.setGrantWrite("id=123,id=456");
    //    input_v2.setGrantWriteAcp("id=123,id=456");
    input_v1.setStorageClass("IA");

    auto output_v1 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1.isSuccess(), true);

    auto output_v1_head = cliV2->headBucket(bkt);
    EXPECT_EQ(output_v1_head.isSuccess(), true);
    bool regionCheck = (output_v1_head.result().getRegion() == TestConfig::Region);
    bool storageClassCheck = (output_v1_head.result().getStorageClass() == "IA");
    EXPECT_EQ(regionCheck, true);
    EXPECT_EQ(storageClassCheck, true);

    auto output_v1_delete = cliV2->deleteBucket(bkt);
    EXPECT_EQ(output_v1_delete.isSuccess(), true);
}
TEST_F(BucketCreateHeadClientV1Test, HeadNonExistentBucketClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    // 校验桶元数据
    auto output_v1_head = cliV2->headBucket(bkt);
    EXPECT_EQ(output_v1_head.isSuccess(), false);
    int statusCode = output_v1_head.error().getStatusCode();
    EXPECT_EQ(statusCode, 404);
}

TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithOrthogonalCaseClientV1Test) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix) + "v1";
    CreateBucketInput input_v1;

    input_v1.setBucket(bkt);

    input_v1.setAcl("public-read");
    input_v1.setStorageClass("IA");
    EXPECT_EQ(CreateBucketV1(input_v1).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt).isSuccess(), true);

    std::string bkt2 = TestUtils::GetBucketName(TestConfig::TestPrefix) + "v1";
    input_v1.setBucket(bkt2);
    input_v1.setAcl("public-read-write");
    input_v1.setStorageClass("IA");

    EXPECT_EQ(CreateBucketV1(input_v1).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt2).isSuccess(), true);

    std::string bkt3 = TestUtils::GetBucketName(TestConfig::TestPrefix) + "v1";
    input_v1.setBucket(bkt3);
    input_v1.setAcl("authenticated-read");
    input_v1.setStorageClass("IA");
    EXPECT_EQ(CreateBucketV1(input_v1).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt3).isSuccess(), true);

    std::string bkt4 = TestUtils::GetBucketName(TestConfig::TestPrefix) + "v1";
    input_v1.setBucket(bkt4);
    input_v1.setAcl("bucket-owner-read");
    input_v1.setStorageClass("IA");
    EXPECT_EQ(CreateBucketV1(input_v1).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt4).isSuccess(), true);

    std::string bkt5 = TestUtils::GetBucketName(TestConfig::TestPrefix) + "v1";
    input_v1.setBucket(bkt5);
    input_v1.setAcl("bucket-owner-full-control");
    input_v1.setStorageClass("IA");
    EXPECT_EQ(CreateBucketV1(input_v1).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt5).isSuccess(), true);
}
TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithErrorBucketNameLengthClientV1Test) {
    std::string error_message = "invalid bucket name, the length must be [3, 63]";

    auto randomStr = TestUtils::GetRandomString(20);
    std::string bkt_1 = "0123456789-0123456789-0123456789-01234567890-" + randomStr;

    CreateBucketInput input_v1;
    input_v1.setBucket(bkt_1);
    auto output_v1_1 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_1.isSuccess(), false);
    EXPECT_EQ(output_v1_1.error().getMessage() == error_message, true);
    std::string bkt_2 = "0123456789-0123456789-0123456789-01234567" + randomStr;
    input_v1.setBucket(bkt_2);
    auto output_v1_2 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_2.isSuccess(), true);

    std::string bkt_3 = "aa";
    input_v1.setBucket(bkt_3);
    auto output_v1_3 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_3.isSuccess(), false);
    EXPECT_EQ(output_v1_3.error().getMessage() == error_message, true);

    std::string bkt_4 = TestConfig::TestPrefix + "ttttt" + randomStr;
    input_v1.setBucket(bkt_4);
    auto output_v1_4 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_4.isSuccess(), true);

    EXPECT_EQ(DeleteBucketV1(bkt_2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV1(bkt_4).isSuccess(), true);
}
TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithDisallowedCharacterSetClientV1Test) {
    std::string error_message = "invalid bucket name, the character set is illegal";
    CreateBucketInput input_v1;
    std::string bkt_1 = "ASDC";
    input_v1.setBucket(bkt_1);
    auto output_v1_1 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_1.isSuccess(), false);
    EXPECT_EQ(output_v1_1.error().getMessage() == error_message, true);

    std::string bkt_2 = "aaAAdd";
    input_v1.setBucket(bkt_2);
    auto output_v1_2 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_2.isSuccess(), false);
    EXPECT_EQ(output_v1_2.error().getMessage() == error_message, true);

    std::string bkt_3 = "aa?d-d";
    input_v1.setBucket(bkt_3);
    auto output_v1_3 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_3.isSuccess(), false);
    EXPECT_EQ(output_v1_3.error().getMessage() == error_message, true);
}
TEST_F(BucketCreateHeadClientV1Test, CreateBucketWithSymbolAtBeginOrEndClientV1Test) {
    std::string error_message =
            "invalid bucket name, the bucket name can be neither starting with ' - ' nor ending with ' - '";
    CreateBucketInput input_v1;
    std::string bkt_1 = "-xx";
    input_v1.setBucket(bkt_1);
    auto output_v1_1 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_1.isSuccess(), false);
    EXPECT_EQ(output_v1_1.error().getMessage() == error_message, true);

    std::string bkt_2 = "xx-";
    input_v1.setBucket(bkt_2);
    auto output_v1_2 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_2.isSuccess(), false);
    EXPECT_EQ(output_v1_2.error().getMessage() == error_message, true);

    std::string bkt_3 = "-xx-";
    input_v1.setBucket(bkt_3);
    auto output_v1_3 = cliV2->createBucket(input_v1);
    EXPECT_EQ(output_v1_3.isSuccess(), false);
    EXPECT_EQ(output_v1_3.error().getMessage() == error_message, true);
}

}  // namespace VolcengineTos
