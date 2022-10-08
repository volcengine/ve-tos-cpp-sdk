#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <sstream>
#include <iomanip>
namespace VolcengineTos {
class ObjectListAndListVersionTest : public ::testing::Test {
protected:
    ObjectListAndListVersionTest() {
    }

    ~ObjectListAndListVersionTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bkt_name);
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

std::shared_ptr<TosClientV2> ObjectListAndListVersionTest::cliV2 = nullptr;
std::string ObjectListAndListVersionTest::bkt_name = "";

// TEST_F(ObjectListAndListVersionTest, ListObjectTest) {
// }

TEST_F(ObjectListAndListVersionTest, ListObjectFromNonexistentBucketTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    ListObjectsV2Input input_list(nonexistent_bkt_name);
    auto output = cliV2->listObjects(input_list);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}
TEST_F(ObjectListAndListVersionTest, ListObjectVersionsFromNonexistentBucketTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    ListObjectVersionsV2Input input_list_version(nonexistent_bkt_name);
    auto output = cliV2->listObjectVersions(input_list_version);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}
TEST_F(ObjectListAndListVersionTest, ListObjectWith100ObjectsTest) {
    std::map<std::string, std::string> meta;
    for (int i = 0; i < 100; ++i) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << i;
        std::string idx_string;
        ss >> idx_string;

        meta["self-test"] = idx_string;
        TestUtils::PutObjectWithMeta(cliV2, bkt_name, idx_string, idx_string, meta);
    }
    ListObjectsV2Input input_list(bkt_name);
    input_list.setMaxKeys(10);
    std::string start_marker = "";
    std::string next_marker = "";
    int i = 0;
    for (int t = 0; t < 10; ++t) {
        next_marker = start_marker;
        input_list.setMarker(next_marker);
        auto output = cliV2->listObjects(input_list);
        EXPECT_EQ(output.isSuccess(), true);
        start_marker = output.result().getNextMarker();
        auto content = output.result().getContents();
        for (int p = 0; p < 10; ++p) {
            std::stringstream ss;
            ss << std::setw(3) << std::setfill('0') << i;
            std::string idx_string;
            ss >> idx_string;
            std::string obj_name_ = content[p].getKey();
            std::string tmp_string = TestUtils::GetObjectContent(cliV2, bkt_name, obj_name_);
            auto meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name_);
            bool check_data = (idx_string == tmp_string);
            bool check_meta = (meta["self-test"] == idx_string);
            EXPECT_EQ(check_data & check_meta, true);
            i++;
        }
    }
}

TEST_F(ObjectListAndListVersionTest, ListObjectWith3PrefixObjectsTest) {
    std::string obj_name_ = "11/22/33/4444";
    TestUtils::PutObject(cliV2, bkt_name, obj_name_, "111");
    std::string obj_name_2 = "11/22/34/4444";
    TestUtils::PutObject(cliV2, bkt_name, obj_name_2, "111");
    ListObjectsV2Input input_list(bkt_name);
    input_list.setPrefix("11/22/33/");
    input_list.setDelimiter("/");
    auto output = cliV2->listObjects(input_list);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(ObjectListAndListVersionTest, DeleteMultiObjectsTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    TestUtils::PutObject(cliV2, bkt_name_, "todelete-001", "1111");
    ListObjectsV2Input input_list(bkt_name_);
    auto output_1 = cliV2->listObjects(input_list);
    EXPECT_EQ(output_1.result().getContents().size(), 1);

    std::vector<ObjectTobeDeleted> otds(2);
    ObjectTobeDeleted otd1;
    otd1.setKey("todelete-001");
    otds[0] = otd1;
    ObjectTobeDeleted otd2;
    otd2.setKey("中文测试1644921884327");
    otds[1] = otd2;
    DeleteMultiObjectsInput input;
    input.setQuiet(false);
    input.setObjectTobeDeleteds(otds);
    input.setBucket(bkt_name_);
    auto output = cliV2->deleteMultiObjects(input);
    EXPECT_EQ(output.isSuccess(), true);

    auto output_2 = cliV2->listObjects(input_list);
    EXPECT_EQ(output_2.result().getContents().size(), 0);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectListAndListVersionTest, DeleteFromNonexistentBucketTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);

    std::vector<ObjectTobeDeleted> otds(2);
    ObjectTobeDeleted otd1;
    otd1.setKey("todelete-001");
    otds[0] = otd1;
    ObjectTobeDeleted otd2;
    otd2.setKey("中文测试1644921884327");
    otds[1] = otd2;
    DeleteMultiObjectsInput input;
    input.setQuiet(false);
    input.setObjectTobeDeleteds(otds);
    input.setBucket(bkt_name_);
    auto output = cliV2->deleteMultiObjects(input);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
}

}  // namespace VolcengineTos
