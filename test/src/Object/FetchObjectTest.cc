//#include "../TestConfig.h"
//#include "../Utils.h"
//#include "TosClientV2.h"
//#include <gtest/gtest.h>
//
// namespace VolcengineTos {
// class FetchObjectTest : public ::testing::Test {
// protected:
//    FetchObjectTest() = default;
//
//    ~FetchObjectTest() override = default;
//
//    static void SetUpTestCase() {
//        ClientConfig conf;
//        conf.enableVerifySSL = TestConfig::enableVerifySSL;
//        conf.endPoint = TestConfig::Endpoint;
//        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
//        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
//
//        std::string endPoint = TestConfig::Endpoint;
//        if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTPS)) {
//            host_ = endPoint.substr(std::strlen(http::SchemeHTTPS) + 3,
//                                    endPoint.length() - std::strlen(http::SchemeHTTPS) - 3);
//        } else if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTP)) {
//            host_ = endPoint.substr(std::strlen(http::SchemeHTTP) + 3,
//                                    endPoint.length() - std::strlen(http::SchemeHTTP) - 3);
//        } else {
//            host_ = endPoint;
//        }
//        TestUtils::CreateBucket(cliV2, bucketName);
//    }
//
//    // Tears down the stuff shared by all tests in this test case.
//    static void TearDownTestCase() {
//        TestUtils::CleanBucket(cliV2, bucketName);
//        cliV2 = nullptr;
//    }
//
// public:
//    static std::shared_ptr<TosClientV2> cliV2;
//    static std::string bucketName;
//    static std::string host_;
//};
//
// std::shared_ptr<TosClientV2> FetchObjectTest::cliV2 = nullptr;
// std::string FetchObjectTest::bucketName = "";
// std::string FetchObjectTest::host_ = "";
//
// TEST_F(FetchObjectTest, FetchObjectWithALLParametersTest) {
//    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    std::string fetch_key = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "fetch";
//
//    std::string data = TestUtils::GetRandomString(1024);
//    auto ss = std::make_shared<std::stringstream>(data);
//    PutObjectV2Input putObjectInput(bucketName, obj_key, ss);
//    auto basicInput = putObjectInput.getPutObjectBasicInput();
//    basicInput.setAcl(ACLType::PublicRead);
//    putObjectInput.setPutObjectBasicInput(basicInput);
//    auto output_obj_put = cliV2->putObject(putObjectInput);
//    EXPECT_EQ(output_obj_put.isSuccess(), true);
//    std::map<std::string, std::string> meta_{{"test-key", "test-value"}};
//    std::string url_ = "http://" + bucketName + "." + host_ + "/" + obj_key;
//    FetchObjectInput FetchObjectInput(bucketName, fetch_key, ACLType::Private, StorageClassType::IA, meta_, url_);
//    auto output = cliV2->fetchObject(FetchObjectInput);
//    EXPECT_EQ(output.isSuccess(), true);
//}
//
// TEST_F(FetchObjectTest, PutFetchTaskWithALLParametersTest) {
//    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    std::string fetch_key = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "fetch";
//
//    std::string data = TestUtils::GetRandomString(1024);
//    auto ss = std::make_shared<std::stringstream>(data);
//    PutObjectV2Input putObjectInput(bucketName, obj_key, ss);
//    auto basicInput = putObjectInput.getPutObjectBasicInput();
//    basicInput.setAcl(ACLType::PublicRead);
//    putObjectInput.setPutObjectBasicInput(basicInput);
//    auto output_obj_put = cliV2->putObject(putObjectInput);
//    EXPECT_EQ(output_obj_put.isSuccess(), true);
//    std::map<std::string, std::string> meta_{{"test-key", "test-value"}};
//    std::string url_ = "http://" + bucketName + "." + host_ + "/" + obj_key;
//    PutFetchTaskInput putFetchTaskInput(bucketName, fetch_key, ACLType::Private, StorageClassType::IA, meta_, url_);
//    auto output = cliV2->putFetchTask(putFetchTaskInput);
//    EXPECT_EQ(output.isSuccess(), true);
//
//    HeadObjectV2Input input(bucketName, fetch_key);
//    auto headOutput = cliV2->headObject(input);
//    EXPECT_EQ(headOutput.isSuccess(), true);
//    auto resMeta = headOutput.result().getMeta();
//    EXPECT_EQ(resMeta["test-key"] == "test-value", true);
//    EXPECT_EQ(headOutput.result().getContentLength() == data.length(), true);
//}
//
//}  // namespace VolcengineTos
