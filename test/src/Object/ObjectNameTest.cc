#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <iostream>

namespace VolcengineTos {
class ObjectNameTest : public ::testing::Test {
protected:
    ObjectNameTest() {
    }

    ~ObjectNameTest() override {
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

std::shared_ptr<TosClientV2> ObjectNameTest::cliV2 = nullptr;
std::string ObjectNameTest::bkt_name = "";

TEST_F(ObjectNameTest, NameFrom32To127Test) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    int false_count = 0;
    // 46 .  47 /  92 \  127 delete
    for (int t = 32; t <= 127; ++t) {
        if (t == 46 || t == 47 || t == 92 || t == 127) {
        } else {
            auto i = char(t);
            std::cout << i << std::endl;

            obj_key = i;
            // 一定要注意，由于流 ss
            // 已经走到了末尾，无法从流中读取数据会导致卡死。所以可以两种写法，一种是重置流，另一种是设置 contentLength
            // = 0
            ss->seekg(0, ss->beg);
            input_obj_put.setContent(ss);
            input_obj_put_basic.setKey(obj_key);
            input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
            auto output_obj_put = cliV2->putObject(input_obj_put);
            if (!output_obj_put.isSuccess()) {
                false_count = false_count + 1;
            }
            input_obj_get.setKey(obj_key);
            auto output_obj_get = cliV2->getObject(input_obj_get);
            if (!output_obj_get.isSuccess()) {
                false_count = false_count + 1;
            }
        }
    }
    EXPECT_EQ(false_count, 0);
}

TEST_F(ObjectNameTest, NameTo31AndFrom128Test) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    int false_count = 0;
    for (int t = 0; t <= 31; ++t) {
        auto i = char(t);
        obj_key = i;
        input_obj_put_basic.setKey(obj_key);
        input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
        auto output_obj_put = cliV2->putObject(input_obj_put);
        EXPECT_EQ(output_obj_put.error().getMessage() == "invalid object name, the character set is illegal", true);
        if (output_obj_put.isSuccess()) {
            false_count = false_count + 1;
        }
    }
    for (int t = 128; t <= 255; ++t) {
        auto i = char(t);
        obj_key = i;
        input_obj_put_basic.setKey(obj_key);
        input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
        auto output_obj_put = cliV2->putObject(input_obj_put);
        EXPECT_EQ(output_obj_put.error().getMessage() == "invalid object name, the character set is illegal", true);
        if (output_obj_put.isSuccess()) {
            false_count = false_count + 1;
        }
    }
    EXPECT_EQ(false_count, 0);
}
TEST_F(ObjectNameTest, NameWithTooLargeSizeTest) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key(697, '1');
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == "invalid object name, the length must be [1, 696]", true);

    std::string obj_key_(696, '1');
    input_obj_put_basic.setKey(obj_key_);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put_ = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put_.isSuccess(), true);
}
TEST_F(ObjectNameTest, NameASCII4792Test) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    std::vector<int> t_vector{47, 92};
    for (auto t : t_vector) {
        auto i = char(t);
        obj_key = i;
        input_obj_put_basic.setKey(obj_key);
        input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
        auto output_obj_put = cliV2->putObject(input_obj_put);
        EXPECT_EQ(output_obj_put.isSuccess(), false);
        EXPECT_EQ(output_obj_put.error().getMessage() ==
                          "invalid object name, the object name can not start with '/' or '\\'",
                  true);
    }
}
TEST_F(ObjectNameTest, NameASCII46Test) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    std::vector<int> t_vector{46};
    for (auto t : t_vector) {
        auto i = char(t);
        obj_key = i;
        input_obj_put_basic.setKey(obj_key);
        input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
        auto output_obj_put = cliV2->putObject(input_obj_put);
        EXPECT_EQ(output_obj_put.isSuccess(), false);
        EXPECT_EQ(output_obj_put.error().getMessage() == "invalid object name, the object name can not use '.'", true);
    }
}
TEST_F(ObjectNameTest, NameWithChineseLanguageTest) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);

    obj_key = "测试桶";
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_get.isSuccess(), true);
}
TEST_F(ObjectNameTest, NameWithNormalLanguageTest) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);

    obj_key = "にほんごΨφ";
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto output_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(output_obj_get.isSuccess(), true);
}
TEST_F(ObjectNameTest, NameSpecialSysbolTest) {
    std::string data = "1234567890abc";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string obj_key;
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);

    obj_key = ".（!-_.*()/&$@=;:+ ,?\\{^}%`]>[~<#|'\"）.";
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}
}  // namespace VolcengineTos
