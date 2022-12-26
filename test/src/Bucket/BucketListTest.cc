#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketListTest : public ::testing::Test {
protected:
    BucketListTest() {
    }

    ~BucketListTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;

        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        //        TestUtils::CleanAllBucket(cliV2);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
};

std::shared_ptr<TosClientV2> BucketListTest::cliV2 = nullptr;

TEST_F(BucketListTest, ListBucketTest) {
    std::string testPrefix = TestConfig::TestPrefix + "list-1";
    std::string bkt = TestUtils::GetBucketName(testPrefix);
    ListBucketsInput input_list;
    auto output_list_all_part1 = cliV2->listBuckets(input_list);
    EXPECT_EQ(output_list_all_part1.isSuccess(), true);
    auto buckets_part1 = output_list_all_part1.result().getBuckets();
    for (const auto& bkt_ : buckets_part1) {
        std::string bkt_name = bkt_.getName();
        if (bkt_name.rfind(testPrefix, 0) == 0 && bkt_.getLocation() == TestConfig::Region) {
            std::cout << "Delete bucket name:" << bkt_name << std::endl;
            TestUtils::CleanBucket(cliV2, bkt_name);
        }
    }

    // create a new bucket
    CreateBucketV2Input input_v2;
    input_v2.setBucket(bkt);
    auto output_v2 = cliV2->createBucket(input_v2);
    EXPECT_EQ(output_v2.isSuccess(), true);

    // list all again
    auto output_list_all_part2 = cliV2->listBuckets(input_list);
    EXPECT_EQ(output_list_all_part2.isSuccess(), true);
    auto buckets_part2 = output_list_all_part2.result().getBuckets();
    std::vector<ListedBucket> testBucket;
    for (auto bkt_ : buckets_part2) {
        std::string bkt_name = bkt_.getName();
        if (bkt_name.rfind(testPrefix, 0) == 0 && bkt_.getLocation() == TestConfig::Region) {
            testBucket.emplace_back(bkt_);
        }
    }
    EXPECT_EQ(testBucket.size(), 1);
    bool check_bucket_name = (bkt == testBucket[0].getName());

    bool check_location = (TestConfig::Region == testBucket[0].getLocation());
    EXPECT_EQ(check_bucket_name, true);
    EXPECT_EQ(check_location, true);
    // delete created bucket
    TestUtils::CleanBucket(cliV2, bkt);
}

// Client V1 Test
TEST_F(BucketListTest, ListBucketClientV1Test) {
    std::string testPrefix = TestConfig::TestPrefix + "list";
    std::string bkt = TestUtils::GetBucketName(testPrefix);

    // list all
    ListBucketsInput input_list;
    auto output_list_all_part1 = cliV2->listBuckets(input_list);
    EXPECT_EQ(output_list_all_part1.isSuccess(), true);
    auto buckets_part1 = output_list_all_part1.result().getBuckets();
    for (auto bkt_ : buckets_part1) {
        std::string bkt_name = bkt_.getName();
        if (bkt_name.rfind(testPrefix, 0) == 0 && bkt_.getLocation() == TestConfig::Region) {
            std::cout << "Delete bucket name:" << bkt_name << std::endl;
            TestUtils::CleanBucket(cliV2, bkt_name);
        }
    }

    // create a new bucket
    CreateBucketInput input;
    input.setBucket(bkt);
    auto output = cliV2->createBucket(input);
    EXPECT_EQ(output.isSuccess(), true);

    // list all again
    auto output_list_all_part2 = cliV2->listBuckets(input_list);
    EXPECT_EQ(output_list_all_part2.isSuccess(), true);
    auto buckets_part2 = output_list_all_part2.result().getBuckets();
    std::vector<ListedBucket> testBucket;
    for (auto bkt_ : buckets_part2) {
        std::string bkt_name = bkt_.getName();
        if (bkt_name.rfind(testPrefix, 0) == 0 && bkt_.getLocation() == TestConfig::Region) {
            testBucket.emplace_back(bkt_);
        }
    }
    EXPECT_EQ(testBucket.size(), 1);
    bool check_bucket_name = (bkt == testBucket[0].getName());

    bool check_location = (TestConfig::Region == testBucket[0].getLocation());
    EXPECT_EQ(check_bucket_name, true);
    EXPECT_EQ(check_location, true);
    // delete created bucket
    TestUtils::CleanBucket(cliV2, bkt);
}

}  // namespace VolcengineTos
