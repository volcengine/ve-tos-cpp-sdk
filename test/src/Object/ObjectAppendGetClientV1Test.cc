#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectAppendGetClientV1Test : public ::testing::Test {
protected:
    ObjectAppendGetClientV1Test() {
    }

    ~ObjectAppendGetClientV1Test() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV1 = std::make_shared<TosClient>(TestConfig::Endpoint, TestConfig::Region, TestConfig::Ak, TestConfig::Sk);

        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bkt_name);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
        cliV1 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::shared_ptr<TosClient> cliV1;
    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> ObjectAppendGetClientV1Test::cliV2 = nullptr;
std::shared_ptr<TosClient> ObjectAppendGetClientV1Test::cliV1 = nullptr;
std::string ObjectAppendGetClientV1Test::bkt_name = "";

TEST_F(ObjectAppendGetClientV1Test, AppendWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    auto output = cliV1->appendObject(bkt_name, obj_key, part0, 0);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    auto output1 = cliV1->appendObject(bkt_name, obj_key, part1, output.result().getNextAppendOffset());
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output1.isSuccess(), true);

    std::string tmp_string = TestUtils::GetObjectContentByStream(cliV2, bkt_name, obj_key);
    int data_size = (256 << 10) + (128 << 10);
    std::string data = std::string(data_size, '1');
    int tmp_string_size = tmp_string.size();
    bool length_compare = (data_size == tmp_string_size);
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectAppendGetClientV1Test, AppendObjectToNonexistentBucketTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);

    auto output = cliV1->appendObject(bkt_name_, obj_key, part0, 0);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}

// todo:没有md5这个选项，如果是头部带md5，则返回true，显然TosApi侧没有做处理
// TEST_F(ObjectAppendGetTest, AppendObjectWithNotMatchMd5Test) {
//    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    auto part0 = std::make_shared<std::stringstream>();
//    for (int i = 0; i < (128 << 10); ++i) {
//        *part0 << "1";
//    }
//    std::string data = std::string(128 << 9, '1');
//    std::string dataMd5 = CryptoUtils::md5Sum(data);
//    RequestOptionBuilder rob;
//    rob.withContentMD5(dataMd5);
//    AppendObjectV2Input input_append(bkt_name, obj_key, 0, part0);
//    auto output = cliV1->appendObject(input_append);
//    EXPECT_EQ(output.isSuccess(), false);
//}
}  // namespace VolcengineTos
