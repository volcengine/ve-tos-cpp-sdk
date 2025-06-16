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

// TEST_F(ObjectNameTest, NameFrom32To127Test) {
//     std::string data = "1234567890abc";
//     auto ss = std::make_shared<std::stringstream>(data);
//     std::string obj_key;
//     PutObjectV2Input input_obj_put;
//     auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
//     input_obj_put.setContent(ss);
//     input_obj_put_basic.setBucket(bkt_name);
//     input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
//     GetObjectV2Input input_obj_get;
//     input_obj_get.setBucket(bkt_name);
//     int false_count = 0;
//     // 46 .  47 /  92 \  127 delete
//     for (int t = 0; t <= 255; ++t) {
//         if (t == 0){
//
//         } else {
//             auto i = char(t);
//             std::cout << i << std::endl;
//
//             obj_key = i;
//             // 一定要注意，由于流 ss
//             // 已经走到了末尾，无法从流中读取数据会导致卡死。所以可以两种写法，一种是重置流，另一种是设置
//             contentLength
//             // = 0
//             ss->seekg(0, ss->beg);
//             input_obj_put.setContent(ss);
//             input_obj_put_basic.setKey(obj_key);
//             input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
//             auto output_obj_put = cliV2->putObject(input_obj_put);
//             if (!output_obj_put.isSuccess()) {
//                 false_count = false_count + 1;
//             }
//             input_obj_get.setKey(obj_key);
//             auto output_obj_get = cliV2->getObject(input_obj_get);
//             if (!output_obj_get.isSuccess()) {
//                 false_count = false_count + 1;
//             }
//         }
//     }
//     EXPECT_EQ(false_count, 0);
// }

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
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    std::string obj_key_long(1200, 'a');
    auto headObjectInput = HeadObjectV2Input(bkt_name, obj_key_long);
    auto headObjectOutput = cliV2->headObject(headObjectInput);
    EXPECT_EQ(headObjectOutput.error().getEc(), "0017-00000002");
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

#ifdef _WIN32
    obj_key = u8"测试桶";
#else
    obj_key = "测试桶";
#endif

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

#ifdef _WIN32
    obj_key = u8"にほんごΨφ";
#else
    obj_key = "にほんごΨφ";
#endif

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
#ifdef _WIN32
    obj_key = u8".（!-_.*()/&$@=;:+ ,?\\{^}%`]>[~<#|'\"）.";
#else
    obj_key = ".（!-_.*()/&$@=;:+ ,?\\{^}%`]>[~<#|'\"）.";
#endif
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}
}  // namespace VolcengineTos
