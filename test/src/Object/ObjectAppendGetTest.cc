#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectAppendGetTest : public ::testing::Test {
protected:
    ObjectAppendGetTest() {
    }

    ~ObjectAppendGetTest() override {
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

std::shared_ptr<TosClientV2> ObjectAppendGetTest::cliV2 = nullptr;
std::string ObjectAppendGetTest::bkt_name = "";

TEST_F(ObjectAppendGetTest, AppendWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_key, part0, 0);
    auto output = cliV2->appendObject(input_append);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input_append.setContent(part1);
    input_append.setOffset(output.result().getNextAppendOffset());
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output1.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_key);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = out_obj_get.result().getGetObjectBasicOutput();
    auto content_output = out_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    long ss = 256 << 11;
    char streamBuffer[ss];
    memset(streamBuffer, 0, ss);
    while (stream->good()) {
        stream->read(streamBuffer, ss);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    int data_size = (256 << 10) + (128 << 10);
    std::string data = std::string(data_size, '1');
    int tmp_string_size = tmp_string.size();
    bool length_compare = (data_size == tmp_string_size);
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectAppendGetTest, AppendObjectToNonexistentBucketTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    AppendObjectV2Input input_append(bkt_name_, obj_key, part0, 0);
    auto output = cliV2->appendObject(input_append);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectAppendGetTest, AppendWithAllParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_key, part0, 0);
    input_append.setContentType("text/plain");
    input_append.setAcl(ACLType::PublicReadWrite);
    // input_append.setGrantWriteAcp("id=123");
    std::map<std::string, std::string> meta_1{{"self-test", "yes"}};
    input_append.setMeta(meta_1);
    input_append.setWebsiteRedirectLocation("/anotherObjectName");
    input_append.setStorageClass(StorageClassType::STANDARD);
    auto output = cliV2->appendObject(input_append);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input_append.setContent(part1);
    input_append.setOffset(output.result().getNextAppendOffset());
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output1.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_key);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = out_obj_get.result().getGetObjectBasicOutput();
    auto content_output = out_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    long ss = 256 << 11;
    char streamBuffer[ss];
    memset(streamBuffer, 0, ss);
    while (stream->good()) {
        stream->read(streamBuffer, ss);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    int data_size = (256 << 10) + (128 << 10);
    std::string data = std::string(data_size, '1');
    int tmp_string_size = tmp_string.size();
    bool length_compare = (data_size == tmp_string_size);
    bool content_compare = (data == tmp_string);
    auto meta = basic_output.getMeta();
    bool check_meta = (meta["self-test"] == "yes");
    EXPECT_EQ(length_compare, true);
    EXPECT_EQ(content_compare, true);
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
//    auto output = cliV2->appendObject(input_append);
//    EXPECT_EQ(output.isSuccess(), false);
//}
}  // namespace VolcengineTos
