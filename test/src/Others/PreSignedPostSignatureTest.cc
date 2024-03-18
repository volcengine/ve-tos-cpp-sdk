#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <map>

namespace VolcengineTos {
class PreSignedPostSignatureTest : public ::testing::Test {
protected:
    PreSignedPostSignatureTest() = default;

    ~PreSignedPostSignatureTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        //        TestUtils::CleanAllBucket(cliV2);
        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
        objectKey = TestUtils::GetObjectKey(TestConfig::TestPrefix);
        TestUtils::CreateBucket(cliV2, bucketName);
        TestUtils::PutObject(cliV2, bucketName, objectKey, "123456");
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bucketName);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bucketName;
    static std::string objectKey;
};

std::shared_ptr<TosClientV2> PreSignedPostSignatureTest::cliV2 = nullptr;
std::string PreSignedPostSignatureTest::bucketName = "";
std::string PreSignedPostSignatureTest::objectKey = "";

TEST_F(PreSignedPostSignatureTest, PreSignedPostSignatureTest) {
    PreSignedPostSignatureInput preSignedPostSignatureInput("sdktest-bucket-1666673101587",
                                                            "sdktest-object-1666673101587", 3600);
    auto putOutput = cliV2->preSignedPostSignature(preSignedPostSignatureInput);
    EXPECT_EQ(putOutput.isSuccess(), true);
    auto res = putOutput.result();
    std::cout << "-----"
              << "\n"
              << res.getPolicy() << "\n"
              << res.getAlgorithm() << "\n"
              << res.getCredential() << "\n"
              << res.getDate() << "\n"
              << res.getOriginPolicy() << "\n"
              << res.getSignature() << "\n"
              << "-----" << std::endl;
    preSignedPostSignatureInput.setContentLengthRange(std::make_shared<ContentLengthRange>(50, 1025));
    auto putOutput2 = cliV2->preSignedPostSignature(preSignedPostSignatureInput);
    EXPECT_EQ(putOutput2.isSuccess(), true);
    res = putOutput2.result();
    std::cout << res.getPolicy() << "\n"
              << res.getAlgorithm() << "\n"
              << res.getCredential() << "\n"
              << res.getDate() << "\n"
              << res.getOriginPolicy() << "\n"
              << res.getSignature() << "\n"
              << "-----" << std::endl;
    preSignedPostSignatureInput.setExpires(1);
    auto putOutput3 = cliV2->preSignedPostSignature(preSignedPostSignatureInput);
    EXPECT_EQ(putOutput3.isSuccess(), true);
    res = putOutput3.result();
    std::cout << res.getPolicy() << "\n"
              << res.getAlgorithm() << "\n"
              << res.getCredential() << "\n"
              << res.getDate() << "\n"
              << res.getOriginPolicy() << "\n"
              << res.getSignature() << "\n"
              << "-----" << std::endl;

    PreSignedPostSignatureInput preSignedPostSignatureInput2(bucketName, objectKey, 3600);
    preSignedPostSignatureInput2.setConditions(
            {PostSignatureCondition("x-tos-acl", "public-read"),
             PostSignatureCondition("key", "sdktest", std::make_shared<std::string>("starts-with"))});
    auto putOutput4 = cliV2->preSignedPostSignature(preSignedPostSignatureInput2);
    EXPECT_EQ(putOutput4.isSuccess(), true);
    res = putOutput4.result();
    std::cout << res.getPolicy() << "\n"
              << res.getAlgorithm() << "\n"
              << res.getCredential() << "\n"
              << res.getDate() << "\n"
              << res.getOriginPolicy() << "\n"
              << res.getSignature() << "\n"
              << "-----" << std::endl;
}

TEST_F(PreSignedPostSignatureTest, PreSignedURLWithAlternativeEndpointTest) {
    PreSignedURLInput input(HttpMethodType::Get, "", objectKey, 86400);
    input.setAlternativeEndpoint("www.baidu.com");
    auto output = cliV2->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_NE(output.result().getSignUrl().find("https://www.baidu.com/" + objectKey), std::string::npos);

    input.setAlternativeEndpoint("http://www.baidu.com");
    output = cliV2->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_NE(output.result().getSignUrl().find("http://www.baidu.com/" + objectKey), std::string::npos);
}

TEST_F(PreSignedPostSignatureTest, PreSignedURLWithCustEndpointTest) {
    ClientConfig conf;
    conf.endPoint = "custom.domain";
    conf.isCustomDomain = true;
    auto cli = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
    PreSignedURLInput input(HttpMethodType::Get, bucketName, objectKey, 86400);
    auto output = cli->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_NE(output.result().getSignUrl().find("https://custom.domain/" + objectKey), std::string::npos);
}

TEST_F(PreSignedPostSignatureTest, PreSignedURLTest) {
    PreSignedURLInput input(HttpMethodType::Get, bucketName, objectKey, 86400);
    auto output = cliV2->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(PreSignedPostSignatureTest, PreSignedURLWithEncodeTest) {
    PreSignedURLInput input(HttpMethodType::Get, bucketName, objectKey, 86400);
    std::map<std::string, std::string> meta{{"中文/中文", "no/yes"}};
    input.setQuery(meta);
    auto output = cliV2->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
}

TEST_F(PreSignedPostSignatureTest, PreSignedURLWithPublicRegionTest) {
    auto cli = std::make_shared<TosClientV2>("ap-southeast-1", TestConfig::Ak, TestConfig::Sk);
    PreSignedURLInput input(HttpMethodType::Get, bucketName, objectKey, 86400);
    auto output = cli->preSignedURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_NE(output.result().getSignUrl().find("https://" + bucketName + ".tos-ap-southeast-1.volces.com" + "/" +
                                                objectKey),
              std::string::npos);
}

TEST_F(PreSignedPostSignatureTest, PreSignedPolicyURLTest) {
    auto condition1 = PolicySignatureCondition("key", "", "starts-with");
    std::vector<PolicySignatureCondition> conditions{condition1};
    PreSignedPolicyURLInput input(bucketName, 86400, conditions);
    auto output = cliV2->preSignedPolicyURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    auto getUrl = output.result().GetSignedURLForGetOrHead(objectKey, {});
    auto listUrl = output.result().GetSignedURLForGetOrHead("111", {});
}

TEST_F(PreSignedPostSignatureTest, PreSignedPolicyURLWithAlternativeEndpointTest) {
    auto condition1 = PolicySignatureCondition("key", "", "starts-with");
    std::vector<PolicySignatureCondition> conditions{condition1};
    PreSignedPolicyURLInput input(bucketName, 86400, conditions);
    input.setIsCustomDomain(true);
    input.setAlternativeEndpoint("www.baidu.com");
    auto output = cliV2->preSignedPolicyURL(input);
    EXPECT_EQ(output.isSuccess(), true);
    auto getUrl = output.result().GetSignedURLForGetOrHead(objectKey, {});
    auto listUrl = output.result().GetSignedURLForGetOrHead("111", {});
}

}  // namespace VolcengineTos
