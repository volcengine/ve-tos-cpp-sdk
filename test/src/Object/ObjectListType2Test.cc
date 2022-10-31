#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <sstream>
#include <iomanip>
namespace VolcengineTos {
class ObjectListType2Test : public ::testing::Test {
protected:
    ObjectListType2Test() {
    }

    ~ObjectListType2Test() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bkt_name);
        int length = 3;
        for (int i = 0; i < length; ++i) {
            for (int j = 0; j < length; ++j) {
                for (int k = 0; k < length; ++k) {
                    std::string idx_string = std::to_string(i) + "/" + std::to_string(j) + "/" + std::to_string(k);
                    TestUtils::PutObject(cliV2, bkt_name, idx_string, idx_string);
                }
            }
        }
        TestUtils::PutObject(cliV2, bkt_name, "0/1", "0/1");
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> ObjectListType2Test::cliV2 = nullptr;
std::string ObjectListType2Test::bkt_name = "";

TEST_F(ObjectListType2Test, ListObjectsType2ListOnce1Test) {
    ListObjectsType2Input listInput(bkt_name, "0/1", "0/1", 4);
    auto output = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_EQ(output.result().getContents().size(), 3);
}

TEST_F(ObjectListType2Test, ListObjectsType2WithTokenTest) {
    ListObjectsType2Input listInput(bkt_name, "0/1", "0/1", 2);
    auto output = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output.isSuccess(), true);

    auto output2 = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output2.isSuccess(), true);
    EXPECT_EQ(output2.result().getContents()[0].getKey() == "0/1/0", true);
    EXPECT_EQ(output2.result().getContents()[1].getKey() == "0/1/1", true);
    EXPECT_EQ(output2.result().getContents().size() == 2, true);

    listInput.setContinuationToken(output.result().getNextContinuationToken());
    auto output3 = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output3.isSuccess(), true);
    EXPECT_EQ(output3.result().getContents()[0].getKey() == "0/1/2", true);
    EXPECT_EQ(output3.result().getContents().size() == 1, true);
}

TEST_F(ObjectListType2Test, ListObjectsType2WithDelimiterTest) {
    ListObjectsType2Input listInput(bkt_name, "0/", 3);
    listInput.setDelimiter("/");
    auto output = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_EQ(output.result().getCommonPrefixes().size() == 2, true);
    EXPECT_EQ(output.result().getContents().size() == 1, true);
    listInput.setPrefix(output.result().getCommonPrefixes()[0].getPrefix());
    auto output2 = cliV2->listObjectsType2(listInput);

    EXPECT_EQ(output2.isSuccess(), true);
    EXPECT_EQ(output2.result().getContents()[0].getKey() == "0/0/0", true);
    EXPECT_EQ(output2.result().getContents()[1].getKey() == "0/0/1", true);
    EXPECT_EQ(output2.result().getContents()[2].getKey() == "0/0/2", true);
    EXPECT_EQ(output2.result().getContents().size() == 3, true);
}

TEST_F(ObjectListType2Test, ListObjectsType2WithDelimiter2Test) {
    ListObjectsType2Input listInput(bkt_name, "0/", 5);
    listInput.setDelimiter("/");
    auto output = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_EQ(output.result().getCommonPrefixes().size() == 3, true);
    EXPECT_EQ(output.result().getContents().size() == 1, true);
    EXPECT_EQ(output.result().getContents()[0].getKey() == "0/1", true);

    listInput.setPrefix(output.result().getCommonPrefixes()[0].getPrefix());
    auto output2 = cliV2->listObjectsType2(listInput);

    EXPECT_EQ(output2.isSuccess(), true);
    EXPECT_EQ(output2.result().getContents()[0].getKey() == "0/0/0", true);
    EXPECT_EQ(output2.result().getContents()[1].getKey() == "0/0/1", true);
    EXPECT_EQ(output2.result().getContents()[2].getKey() == "0/0/2", true);
    EXPECT_EQ(output2.result().getContents().size() == 3, true);
}

TEST_F(ObjectListType2Test, ListObjectsType2WithoutMaxKeyTest) {
    ListObjectsType2Input listInput(bkt_name);
    auto output = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_EQ(output.result().getContents().size(), 28);
}
}  // namespace VolcengineTos
