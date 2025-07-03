#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

//#include <dirent.h>
namespace VolcengineTos {
class ObjectRenameTest : public ::testing::Test {
protected:
    ObjectRenameTest() {
    }

    ~ObjectRenameTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
        conf.maxRetryCount = 0;
        conf.userAgentProductName = "tos";
        conf.userAgentSoftName="cxxSdk";
        conf.userAgentSoftVersion = "2.6.16";
        conf.userAgentCustomizedKeyValues = {{"key1", "value1"}, {"key2", "value2"}};
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        bkt_name_hns = TestUtils::GetBucketName(TestConfig::TestPrefixHns);

        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
        TestUtils::CreateBucket(cliV2, bkt_name);
        TestUtils::CreateBucket(cliV2, bkt_name_hns, true);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
    static std::string bkt_name_hns;
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ObjectRenameTest::cliV2 = nullptr;
std::string ObjectRenameTest::bkt_name = "";
std::string ObjectRenameTest::bkt_name_hns = "";
std::string ObjectRenameTest::workPath = "";


TEST_F(ObjectRenameTest, RenameObjectTestHNS) {

    std::string obj_key_suffix = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string obj_key = "cxx/sdk/test" + obj_key_suffix;
    std::string obj_key2 = "cxx/sdk/test2" + obj_key_suffix;
    std::string obj_key_new = "cxx/sdk/new/test" + obj_key_suffix;

    std::string data = "123";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name_hns);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto ss2 = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put2;
    auto input_obj_put_basic2 = input_obj_put2.getPutObjectBasicInput();
    input_obj_put2.setContent(ss2);
    input_obj_put_basic2.setBucket(bkt_name_hns);
    input_obj_put_basic2.setKey(obj_key2);
    input_obj_put2.setPutObjectBasicInput(input_obj_put_basic2);
    auto output_obj_put2 = cliV2->putObject(input_obj_put2);
    EXPECT_EQ(output_obj_put2.isSuccess(), true);

    auto getFileStatusInput = GetFileStatusInput(bkt_name_hns, obj_key);
    auto getFileStatusOutput = cliV2->getFileStatus(getFileStatusInput);
    EXPECT_EQ(getFileStatusOutput.isSuccess(), true);

    auto renameObjectInput  = RenameObjectInput(bkt_name_hns, obj_key, obj_key_new);
    auto renameObjectOutput = cliV2->renameObject(renameObjectInput);
    EXPECT_EQ(renameObjectOutput.isSuccess(), false);

    renameObjectInput.setRecursiveMkdir(true);
    renameObjectOutput = cliV2->renameObject(renameObjectInput);
    EXPECT_EQ(renameObjectOutput.isSuccess(), true);

    auto getFileStatusInput2 = GetFileStatusInput(bkt_name_hns, obj_key_new);
    auto getFileStatusOutput2 = cliV2->getFileStatus(getFileStatusInput2);
    EXPECT_EQ(getFileStatusOutput2.isSuccess(), true);


    renameObjectInput  = RenameObjectInput(bkt_name_hns, obj_key2, obj_key_new);
    renameObjectInput.setForbidOverwrite(true);
    renameObjectInput.setRecursiveMkdir(true);
    renameObjectOutput = cliV2->renameObject(renameObjectInput);
    EXPECT_EQ(renameObjectOutput.isSuccess(), false);

    renameObjectInput.setForbidOverwrite(false);
    renameObjectInput  = RenameObjectInput(bkt_name_hns, obj_key2, obj_key_new);
    renameObjectOutput = cliV2->renameObject(renameObjectInput);
    EXPECT_EQ(renameObjectOutput.isSuccess(), true);

    getFileStatusOutput2 = cliV2->getFileStatus(getFileStatusInput2);
    EXPECT_EQ(getFileStatusOutput2.isSuccess(), true);
}

}  // namespace VolcengineTos
