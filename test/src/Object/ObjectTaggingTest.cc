#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectTaggingTest : public ::testing::Test {
protected:
    ObjectTaggingTest() = default;

    ~ObjectTaggingTest() override = default;

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

std::shared_ptr<TosClientV2> ObjectTaggingTest::cliV2 = nullptr;
std::string ObjectTaggingTest::bucketName = "";

TEST_F(ObjectTaggingTest, ObjectTaggingWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectTaggingInput putObjectTaggingInput(bucketName, obj_key, {});
    auto putOutput = cliV2->putObjectTagging(putObjectTaggingInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getMessage(), "invalid tagSet, the tagSet must be not empty.");
}

TEST_F(ObjectTaggingTest, ObjectTaggingWithALLParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    TestUtils::PutObject(cliV2, bucketName, obj_key, "123");
    Tag tag1{"1", "2"};
    Tag tag2{"3", "4"};
    std::vector<Tag> tags = {tag1, tag2};
    TagSet tagSet(tags);
    PutObjectTaggingInput putObjectTaggingInput(bucketName, obj_key, tagSet);
    auto putOutput = cliV2->putObjectTagging(putObjectTaggingInput);
    EXPECT_EQ(putOutput.isSuccess(), true);

    GetObjectTaggingInput getObjectTaggingInput(bucketName, obj_key);
    auto getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    auto tag = getOutput.result().getTagSet().getTags();
    auto res =
            (tag[0].getKey() == "1" && tag[0].getValue() == "2" && tag[1].getKey() == "3" && tag[1].getValue() == "4");
    EXPECT_EQ(res, true);

    DeleteObjectTaggingInput deleteObjectTaggingInput(bucketName, obj_key);
    auto deleteOutput = cliV2->deleteObjectTagging(deleteObjectTaggingInput);
    EXPECT_EQ(getOutput.isSuccess(), true);

    getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
    EXPECT_EQ(getOutput.isSuccess(), true);
    res = getOutput.result().getTagSet().getTags().empty();
    EXPECT_EQ(res, true);
}

// TEST_F(ObjectTaggingTest, ObjectTaggingVersionsWithALLParametersTest) {
//     std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//     TestUtils::PutObject(cliV2, bucketName, obj_key, "123");
//     Tag tag1{"1", "2"};
//     Tag tag2{"3", "4"};
//     std::vector<Tag> tags = {tag1, tag2};
//     TagSet tagSet(tags);
//     PutObjectTaggingInput putObjectTaggingInput(bucketName, obj_key, tagSet);
//     auto putOutput = cliV2->putObjectTagging(putObjectTaggingInput);
//     EXPECT_EQ(putOutput.isSuccess(), true);
//
//     GetObjectTaggingInput getObjectTaggingInput(bucketName, obj_key);
//     auto getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
//     EXPECT_EQ(getOutput.isSuccess(), true);
//     auto tag = getOutput.result().getTagSet().getTags();
//     auto res =
//             (tag[0].getKey() == "1" && tag[0].getValue() == "2" && tag[1].getKey() == "3" && tag[1].getValue() ==
//             "4");
//     EXPECT_EQ(res, true);
//
//     DeleteObjectTaggingInput deleteObjectTaggingInput(bucketName, obj_key);
//     auto deleteOutput = cliV2->deleteObjectTagging(deleteObjectTaggingInput);
//     EXPECT_EQ(getOutput.isSuccess(), true);
//
//     getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
//     EXPECT_EQ(getOutput.isSuccess(), true);
//     res = getOutput.result().getTagSet().getTags().empty();
//     EXPECT_EQ(res, true);
// }

TEST_F(ObjectTaggingTest, ObjectTaggingWithNonExistentBucketNameTest) {
    auto bucketName_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    Tag tag1{"1", "2"};
    Tag tag2{"3", "4"};
    std::vector<Tag> tags = {tag1, tag2};
    TagSet tagSet(tags);
    PutObjectTaggingInput putObjectTaggingInput(bucketName_, obj_key, tagSet);
    auto putOutput = cliV2->putObjectTagging(putObjectTaggingInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchBucket");
    GetObjectTaggingInput getObjectTaggingInput(bucketName_, obj_key);
    auto getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchBucket");
    DeleteObjectTaggingInput deleteObjectTaggingInput(bucketName_, obj_key);
    auto deleteOutput = cliV2->deleteObjectTagging(deleteObjectTaggingInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchBucket");
}

TEST_F(ObjectTaggingTest, ObjectTaggingWithNonExistentObjectNameTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    Tag tag1{"1", "2"};
    Tag tag2{"3", "4"};
    std::vector<Tag> tags = {tag1, tag2};
    TagSet tagSet(tags);
    PutObjectTaggingInput putObjectTaggingInput(bucketName, obj_key, tagSet);
    auto putOutput = cliV2->putObjectTagging(putObjectTaggingInput);
    EXPECT_EQ(putOutput.isSuccess(), false);
    EXPECT_EQ(putOutput.error().getCode(), "NoSuchKey");
    GetObjectTaggingInput getObjectTaggingInput(bucketName, obj_key);
    auto getOutput = cliV2->getObjectTagging(getObjectTaggingInput);
    EXPECT_EQ(getOutput.isSuccess(), false);
    EXPECT_EQ(getOutput.error().getCode(), "NoSuchKey");
    DeleteObjectTaggingInput deleteObjectTaggingInput(bucketName, obj_key);
    auto deleteOutput = cliV2->deleteObjectTagging(deleteObjectTaggingInput);
    EXPECT_EQ(deleteOutput.isSuccess(), false);
    EXPECT_EQ(deleteOutput.error().getCode(), "NoSuchKey");
}
}  // namespace VolcengineTos
