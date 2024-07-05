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
        bkt_name_hns = TestUtils::GetBucketName(TestConfig::TestPrefixHns);
        TestUtils::CreateBucket(cliV2, bkt_name);
        TestUtils::CreateBucket(cliV2, bkt_name_hns, true);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        TestUtils::CleanBucket(cliV2,bkt_name_hns);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
    static std::string bkt_name_hns;
};

std::shared_ptr<TosClientV2> ObjectAppendGetTest::cliV2 = nullptr;
std::string ObjectAppendGetTest::bkt_name = "";
std::string ObjectAppendGetTest::bkt_name_hns = "";

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
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_get.isSuccess(), true);
    auto basic_output = output_obj_get.result().getGetObjectBasicOutput();
    auto content_output = output_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    std::ostringstream ss;
    ss << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss.str();

    int data_size = (256 << 10) + (128 << 10);
    std::string data = std::string(data_size, '1');
    int tmp_string_size = tmp_string.size();
    bool length_compare = (data_size == tmp_string_size);
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectAppendGetTest, PutAppendGetHnsTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefixHns);
    auto part0 = std::make_shared<std::stringstream>();
    auto part0Size = 128;
    for (int i = 0; i < part0Size; ++i) {
        *part0 << "0";
    }
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(part0);
    input_obj_put_basic.setBucket(bkt_name_hns);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto part1Size = 128;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < part1Size; ++i) {
        *part1 << "1";
    }
    AppendObjectV2Input inputAppend(bkt_name_hns, obj_key, part1, part1Size);
    inputAppend.setContentLength(part1Size);
    auto output = cliV2->appendObject(inputAppend);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output.isSuccess(), true);

    auto part2Size = 128;
    auto part2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < part2Size; ++i) {
        *part2 << "2";
    }
    inputAppend.setContent(part2);
    inputAppend.setOffset(output.result().getNextAppendOffset());
    inputAppend.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    output = cliV2->appendObject(inputAppend);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    EXPECT_EQ(output.isSuccess(), true);

    GetObjectV2Input inputObjGet(bkt_name_hns, obj_key);
    auto outputObjGet = cliV2->getObject(inputObjGet);
    EXPECT_EQ(outputObjGet.isSuccess(), true);
    auto basicOutput = outputObjGet.result().getGetObjectBasicOutput();
    auto contentOutput = outputObjGet.result().getContent();
    std::string ss_;

    auto stream = contentOutput.get();

    std::ostringstream ss;
    ss << outputObjGet.result().getContent()->rdbuf();
    std::string tmp_string = ss.str();

    int data_size = part0Size+part1Size+part2Size;
    std::string data = std::string(part0Size, '0');
    data.append(std::string(part1Size, '1'));
    data.append(std::string(part2Size, '2'));
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
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output1.isSuccess(), true);
    auto basic_output = output_obj_get.result().getGetObjectBasicOutput();
    auto content_output = output_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    std::ostringstream ss;
    ss << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss.str();

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

TEST_F(ObjectAppendGetTest, AppendWithoutParametersWithTrafficLimitTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (10 << 20); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input_append(bkt_name, obj_key, part0, 0);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto output = cliV2->appendObject(input_append);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    EXPECT_EQ(output.isSuccess(), true);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endTime - startTime;
    auto time1 = fp_ms.count() / 1000;

    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (10 << 20); ++i) {
        *part1 << "1";
    }
    input_append.setContent(part1);
    input_append.setOffset(output.result().getNextAppendOffset());
    input_append.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    input_append.setTrafficLimit(1024 * 1024);
    startTime = std::chrono::high_resolution_clock::now();
    auto output1 = cliV2->appendObject(input_append);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    EXPECT_EQ(output1.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time2 = fp_ms.count() / 1000;
    EXPECT_EQ(time2 > time1, true);
}

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
