#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

//#include <dirent.h>
namespace VolcengineTos {
class ObjectStreamtGetTest : public ::testing::Test {
protected:
    ObjectStreamtGetTest() {
    }

    ~ObjectStreamtGetTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
        conf.maxRetryCount = 0;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);

        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
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
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ObjectStreamtGetTest::cliV2 = nullptr;
std::string ObjectStreamtGetTest::bkt_name = "";
std::string ObjectStreamtGetTest::workPath = "";

TEST_F(ObjectStreamtGetTest, PutObjectWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);

    std::string data2 =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss2 = std::make_shared<std::stringstream>(data2);
    input_obj_put.setContent(ss2);
    auto output_obj_put2 = cliV2->putObject(input_obj_put);

    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    std::shared_ptr<std::iostream> resContent;
    auto res_obj_get = cliV2->getObject(input_obj_get, resContent, nullptr);
    assert(res_obj_get.isSuccess());
}
}  // namespace VolcengineTos
