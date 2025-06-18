#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <sstream>
#include <iomanip>
namespace VolcengineTos {
class ObjectDeleteHNSTest : public ::testing::Test {
protected:
    ObjectDeleteHNSTest() {
    }
    
    ~ObjectDeleteHNSTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);

        // todo need PutBucketTrash ahead
        // bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        bkt_name = "cpp-sdk-hns-delete";
        TestUtils::CreateBucket(cliV2, bkt_name,true);
        int length = 4;
        for (int i = 0; i < length; ++i) {
            for (int j = 0; j < length; ++j) {
                for (int k = 0; k < length; ++k) {
                    std::string idx_string = "test/" + std::to_string(i) + "/" + std::to_string(j) + "/" + std::to_string(k);
                    TestUtils::PutObject(cliV2, bkt_name, idx_string, idx_string);
                }
            }
        }
        TestUtils::PutObject(cliV2, bkt_name, "test/0/1", "0/1");
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

std::shared_ptr<TosClientV2> ObjectDeleteHNSTest::cliV2 = nullptr;
std::string ObjectDeleteHNSTest::bkt_name = "";

TEST_F(ObjectDeleteHNSTest, ObjectDeleteHNSWithRecursive) {
    HeadObjectV2Input headObjectInput(bkt_name, "test/0/");
    auto headObjectOutput =  cliV2->headObject(headObjectInput);
    EXPECT_EQ(headObjectOutput.isSuccess(), true);

    DeleteObjectInput deleteObjectInput(bkt_name, "test/0/");
    deleteObjectInput.setRecursive(true);
    auto output1 = cliV2->deleteObject(deleteObjectInput);
    EXPECT_EQ(output1.isSuccess(), true);
    EXPECT_EQ(output1.result().getTrashPath().empty(), false);

    headObjectOutput =  cliV2->headObject(headObjectInput);
    EXPECT_EQ(headObjectOutput.isSuccess(), false);

    ListObjectsType2Input listInput(bkt_name, "test/0/", 100);
    listInput.setDelimiter("/");
    auto output2 = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output2.isSuccess(), true);
    EXPECT_EQ(output2.result().getCommonPrefixes().empty(), true);
    EXPECT_EQ(output2.result().getContents().empty(), true);

    HeadObjectV2Input headObjectInput1(bkt_name, "test/1/");
    auto headObjectOutput1 =  cliV2->headObject(headObjectInput1);
    EXPECT_EQ(headObjectOutput1.isSuccess(), true);

    HeadObjectV2Input headObjectInput2(bkt_name, "test/2/");
    auto headObjectOutput2 =  cliV2->headObject(headObjectInput2);
    EXPECT_EQ(headObjectOutput2.isSuccess(), true);

    HeadObjectV2Input headObjectInput3(bkt_name, "test/3/");
    auto headObjectOutput3 =  cliV2->headObject(headObjectInput3);
    EXPECT_EQ(headObjectOutput3.isSuccess(), true);

    std::vector<ObjectTobeDeleted> otds(3);
    ObjectTobeDeleted otd1;
    otd1.setKey("test/1/");
    otds[0] = otd1;
    ObjectTobeDeleted otd2;
    otd2.setKey("test/2/");
    otds[1] = otd2;
    ObjectTobeDeleted otd3;
    otd3.setKey("test/3");
    otds[2] = otd3;
    DeleteMultiObjectsInput deleteMultiObjectsInput;
    deleteMultiObjectsInput.setRecursive(true);
    deleteMultiObjectsInput.setObjectTobeDeleteds(otds);
    auto output3 = cliV2->deleteMultiObjects(bkt_name,deleteMultiObjectsInput);
    EXPECT_EQ(output3.isSuccess(), true);
    EXPECT_EQ(output3.result().getErrors().empty(), true);
    EXPECT_EQ(output3.result().getDeleteds().empty(), false);
    EXPECT_EQ(output3.result().getDeleteds()[0].getTrashPath().empty(), false);
    EXPECT_EQ(output3.result().getDeleteds()[1].getTrashPath().empty(), false);
    EXPECT_EQ(output3.result().getDeleteds()[2].getTrashPath().empty(), false);

    headObjectOutput1 =  cliV2->headObject(headObjectInput1);
    EXPECT_EQ(headObjectOutput1.isSuccess(), false);

    headObjectOutput2 =  cliV2->headObject(headObjectInput2);
    EXPECT_EQ(headObjectOutput2.isSuccess(), false);

    headObjectOutput3 =  cliV2->headObject(headObjectInput3);
    EXPECT_EQ(headObjectOutput3.isSuccess(), false);

    ListObjectsType2Input listInput2(bkt_name, "test", 100);
    listInput2.setDelimiter("/");
    auto output4 = cliV2->listObjectsType2(listInput2);
    EXPECT_EQ(output4.isSuccess(), true);
    EXPECT_EQ(output4.result().getCommonPrefixes().size(), 1);
    EXPECT_EQ(output4.result().getContents().empty(), true);
}

TEST_F(ObjectDeleteHNSTest, ObjectDeleteHNSWithRecursiveAndQuite) {
    DeleteObjectInput deleteObjectInput(bkt_name, "test/0/");
    deleteObjectInput.setRecursive(true);
    auto output1 = cliV2->deleteObject(deleteObjectInput);
    EXPECT_EQ(output1.isSuccess(), true);
    EXPECT_EQ(output1.result().getTrashPath().empty(), false);

    ListObjectsType2Input listInput(bkt_name, "test/0/", 100);
    listInput.setDelimiter("/");
    auto output2 = cliV2->listObjectsType2(listInput);
    EXPECT_EQ(output2.isSuccess(), true);
    EXPECT_EQ(output2.result().getCommonPrefixes().empty(), true);

    std::vector<ObjectTobeDeleted> otds(3);
    ObjectTobeDeleted otd1;
    otd1.setKey("test/1/");
    otds[0] = otd1;
    ObjectTobeDeleted otd2;
    otd2.setKey("test/2/");
    otds[1] = otd2;
    ObjectTobeDeleted otd3;
    otd3.setKey("test/");
    otds[2] = otd3;
    DeleteMultiObjectsInput deleteMultiObjectsInput;
    deleteMultiObjectsInput.setQuiet(true);
    deleteMultiObjectsInput.setRecursive(true);
    deleteMultiObjectsInput.setObjectTobeDeleteds(otds);
    auto output3 = cliV2->deleteMultiObjects(bkt_name,deleteMultiObjectsInput);
    EXPECT_EQ(output3.isSuccess(), true);
    EXPECT_EQ(output3.result().getErrors().empty(), true);
    EXPECT_EQ(output3.result().getDeleteds().empty(), true);

    ListObjectsType2Input listInput2(bkt_name, "test", 100);
    listInput2.setDelimiter("/");
    auto output4 = cliV2->listObjectsType2(listInput2);
    EXPECT_EQ(output4.isSuccess(), true);
    EXPECT_EQ(output4.result().getCommonPrefixes().empty(), true);
    EXPECT_EQ(output4.result().getContents().empty(), true);
}
}