#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketCreateHeadTest : public ::testing::Test {
protected:
    BucketCreateHeadTest() {
    }

    ~BucketCreateHeadTest() override {
    }

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

    static Outcome<TosError, CreateBucketV2Output> CreateBucketV2(CreateBucketV2Input& input) {
        auto output_v2 = cliV2->createBucket(input);
        if (!output_v2.isSuccess()) {
            std::cout << output_v2.error().getMessage() << std::endl;
        }
        return output_v2;
    }

    static Outcome<TosError, DeleteBucketOutput> DeleteBucketV2(DeleteBucketInput& input_v2_delete) {
        auto output_v2_delete = cliV2->deleteBucket(input_v2_delete);
        if (!output_v2_delete.isSuccess()) {
            std::cout << output_v2_delete.error().getMessage() << std::endl;
        }
        return output_v2_delete;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
};
std::shared_ptr<TosClientV2> BucketCreateHeadTest::cliV2 = nullptr;

TEST_F(BucketCreateHeadTest, CreateBucketWithBucketNameTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketV2Input input_v2(bkt);

    auto output_v2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2.isSuccess(), true);

    // 校验桶元数据
    HeadBucketV2Input input_v2_head(bkt);
    auto output_v2_head = cliV2->headBucket(input_v2_head);
    EXPECT_EQ(output_v2_head.isSuccess(), true);
    bool regionCheck = (output_v2_head.result().getRegion() == TestConfig::Region);
    bool storageClassCheck = (output_v2_head.result().getStorageClass() == StorageClassType::STANDARD);
    EXPECT_EQ(regionCheck, true);
    EXPECT_EQ(storageClassCheck, true);

    DeleteBucketInput input_v2_delete(bkt);
    auto output_v2_delete = cliV2->deleteBucket(input_v2_delete);
    EXPECT_EQ(output_v2_delete.isSuccess(), true);
}
TEST_F(BucketCreateHeadTest, CreateBucketWithParametersTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketV2Input input_v2(bkt);
    input_v2.setStorageClass(StorageClassType::IA);

    //    input_v2.setAcl(PublicRead);
    //    input_v2.setGrantFullControl("id=123");
    //    input_v2.setGrantRead("id=123");
    //    input_v2.setGrantReadAcp("id=123,id=456");
    //    input_v2.setGrantWrite("id=123,id=456");
    //    input_v2.setGrantWriteAcp("id=123,id=456");

    auto output_v2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2.isSuccess(), true);

    // 校验桶元数据
    HeadBucketV2Input input_v2_head;
    input_v2_head.setBucket(bkt);
    auto output_v2_head = cliV2->headBucket(input_v2_head);
    EXPECT_EQ(output_v2_head.isSuccess(), true);
    bool regionCheck = (output_v2_head.result().getRegion() == TestConfig::Region);
    bool storageClassCheck = (output_v2_head.result().getStorageClass() == StorageClassType::IA);
    EXPECT_EQ(regionCheck, true);
    EXPECT_EQ(storageClassCheck, true);

    DeleteBucketInput input_v2_delete;
    input_v2_delete.setBucket(bkt);
    auto output_v2_delete = cliV2->deleteBucket(input_v2_delete);
    EXPECT_EQ(output_v2_delete.isSuccess(), true);
}
TEST_F(BucketCreateHeadTest, HeadNonExistentBucketTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    // 校验桶元数据
    HeadBucketV2Input input_v2_head;
    input_v2_head.setBucket(bkt);
    auto output_v2_head = cliV2->headBucket(input_v2_head);
    EXPECT_EQ(output_v2_head.isSuccess(), false);
    int statusCode = output_v2_head.error().getStatusCode();
    EXPECT_EQ(statusCode, 404);
}

TEST_F(BucketCreateHeadTest, CreateBucketWithOrthogonalCaseTest) {
    std::string bkt = TestUtils::GetBucketName(TestConfig::TestPrefix);
    CreateBucketV2Input input_v2(bkt);
    DeleteBucketInput input_v2_delete(bkt);

    std::string bkt2 = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_v2.setBucket(bkt2);
    input_v2_delete.setBucket(bkt2);
    input_v2.setAcl(ACLType::PublicRead);
    input_v2.setStorageClass(StorageClassType::IA);
    EXPECT_EQ(CreateBucketV2(input_v2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);

    std::string bkt3 = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_v2.setBucket(bkt3);
    input_v2_delete.setBucket(bkt3);
    input_v2.setAcl(ACLType::PublicReadWrite);
    EXPECT_EQ(CreateBucketV2(input_v2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);

    std::string bkt4 = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_v2.setBucket(bkt4);
    input_v2_delete.setBucket(bkt4);
    input_v2.setAcl(ACLType::AuthenticatedRead);
    EXPECT_EQ(CreateBucketV2(input_v2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);

    std::string bkt5 = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_v2.setBucket(bkt5);
    input_v2_delete.setBucket(bkt5);
    input_v2.setAcl(ACLType::BucketOwnerRead);
    EXPECT_EQ(CreateBucketV2(input_v2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);

    std::string bkt6 = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_v2.setBucket(bkt6);
    input_v2_delete.setBucket(bkt6);
    input_v2.setAcl(ACLType::BucketOwnerFullControl);
    EXPECT_EQ(CreateBucketV2(input_v2).isSuccess(), true);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);
}
TEST_F(BucketCreateHeadTest, CreateBucketWithErrorBucketNameLengthTest) {
    std::string error_message = "invalid bucket name, the length must be [3, 63]";
    CreateBucketV2Input input_v2;
    std::string bkt_1 = "0123456789-0123456789-0123456789-01234567890-01234567890-01234567890";
    input_v2.setBucket(bkt_1);
    auto output_v2_1 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_1.isSuccess(), false);
    EXPECT_EQ(output_v2_1.error().getMessage() == error_message, true);
    std::string bkt_2 = "0123456789-0123456789-0123456789-0123456789-0123456789-01234567";
    input_v2.setBucket(bkt_2);
    auto output_v2_2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_2.isSuccess(), true);

    std::string bkt_3 = "aa";
    input_v2.setBucket(bkt_3);
    auto output_v2_3 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_3.isSuccess(), false);
    EXPECT_EQ(output_v2_3.error().getMessage() == error_message, true);

    std::string bkt_4 = "ttttt";
    input_v2.setBucket(bkt_4);
    auto output_v2_4 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_4.isSuccess(), true);

    DeleteBucketInput input_v2_delete;
    input_v2_delete.setBucket(bkt_2);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);
    input_v2_delete.setBucket(bkt_4);
    EXPECT_EQ(DeleteBucketV2(input_v2_delete).isSuccess(), true);
}
TEST_F(BucketCreateHeadTest, CreateBucketWithDisallowedCharacterSetTest) {
    std::string error_message = "invalid bucket name, the character set is illegal";
    CreateBucketV2Input input_v2;
    std::string bkt_1 = "ASDC";
    input_v2.setBucket(bkt_1);
    auto output_v2_1 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_1.isSuccess(), false);
    EXPECT_EQ(output_v2_1.error().getMessage() == error_message, true);

    std::string bkt_2 = "aaAAdd";
    input_v2.setBucket(bkt_2);
    auto output_v2_2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_2.isSuccess(), false);
    EXPECT_EQ(output_v2_2.error().getMessage() == error_message, true);

    std::string bkt_3 = "aa?d-d";
    input_v2.setBucket(bkt_3);
    auto output_v2_3 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_3.isSuccess(), false);
    EXPECT_EQ(output_v2_3.error().getMessage() == error_message, true);
}
TEST_F(BucketCreateHeadTest, CreateBucketWithSymbolAtBeginOrEndTest) {
    std::string error_message =
            "invalid bucket name, the bucket name can be neither starting with ' - ' nor ending with ' - '";
    CreateBucketV2Input input_v2;
    std::string bkt_1 = "-xx";
    input_v2.setBucket(bkt_1);
    auto output_v2_1 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_1.isSuccess(), false);
    EXPECT_EQ(output_v2_1.error().getMessage() == error_message, true);

    std::string bkt_2 = "xx-";
    input_v2.setBucket(bkt_2);
    auto output_v2_2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_2.isSuccess(), false);
    EXPECT_EQ(output_v2_2.error().getMessage() == error_message, true);

    std::string bkt_3 = "-xx-";
    input_v2.setBucket(bkt_3);
    auto output_v2_3 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2_3.isSuccess(), false);
    EXPECT_EQ(output_v2_3.error().getMessage() == error_message, true);
}

}  // namespace VolcengineTos
