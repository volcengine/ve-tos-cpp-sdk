#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class InitClientTest : public ::testing::Test {
protected:
    InitClientTest() {
    }

    ~InitClientTest() override = default;

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        conf.enableCRC = true;
        cli = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bktName = TestUtils::GetBucketName(TestConfig::TestPrefix);
        //        TestUtils::CreateBucket(cli, bktName);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        //        TestUtils::CleanBucket(cli, bktName);
        cli = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cli;
    static std::string bktName;
};

std::shared_ptr<TosClientV2> InitClientTest::cli = nullptr;
std::string InitClientTest::bktName = "";

static EnvCredentials* NewEnvCredentials() {
    auto* envCredentials = new EnvCredentials();
    return envCredentials;
};

static EcsCredentials* NewEcsCredentials(const std::string& role, const std::string& url) {
    auto* ecsCredentials = new EcsCredentials(role, url);
    return ecsCredentials;
};

TEST_F(InitClientTest, InitClientTest) {
    ClientConfig conf;
    conf.endPoint = TestConfig::Endpoint;
    conf.enableCRC = true;
    // 不带 config，Env 方式
    std::shared_ptr<Credentials> envCredentialsProvider(NewEnvCredentials());

    auto cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, envCredentialsProvider, conf);

    // 带 config，Env 方式
    cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, envCredentialsProvider, conf);

    std::shared_ptr<Credentials> ecsCredentialsProvider(
            NewEcsCredentials("role1", "http://100.96.0.961/volcstack/latest/iam/security_credentials1/"));
    // 不带 config，Env 方式
    cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, ecsCredentialsProvider, conf);
    // 带 config，Env 方式
    cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, ecsCredentialsProvider, conf);
}

TEST_F(InitClientTest, InitClientV1Test) {
    ClientConfig conf;
    conf.endPoint = TestConfig::Endpoint;
    conf.enableCRC = true;
    std::shared_ptr<Credentials> envCredentialsProvider(NewEnvCredentials());
    std::shared_ptr<Credentials> ecsCredentialsProvider(
            NewEcsCredentials("role1", "http://100.96.0.961/volcstack/latest/iam/security_credentials1/"));

    // 生成数据
    std::string objKey = "obj";
    std::string data = "11111111";

    auto value = std::getenv("TOS_ACCESS_KEY");
    std::string ak(value);
    EXPECT_EQ(ak.empty(), false);
    value = std::getenv("TOS_SECRET_KEY");
    std::string sk(value);
    EXPECT_EQ(sk.empty(), false);

    // 带 config，Env 方式
    auto cliV1 = std::make_shared<TosClientV2>(TestConfig::Region, envCredentialsProvider, conf);

    auto ss = std::make_shared<std::stringstream>(data);
    //    auto res = cliV1->putObject(bktName, objKey, ss);
    //    EXPECT_EQ(res.isSuccess(), true);

    // BOE 环境该用例会失败，因为没有 Region 对应的 Endpoint
    //    cliV1 = std::make_shared<TosClientV2>(TestConfig::Region, envCred);
    //    ss = std::make_shared<std::stringstream>(data);
    //    res = cliV1->putObject(bktName, objKey, ss);
    //    EXPECT_EQ(res.isSuccess(), true);

    // 不带 config，Env 方式，BOE 环境该用例会失败，因为没有 Region 对应的 Endpoint
    cliV1 = std::make_shared<TosClientV2>(TestConfig::Region, ecsCredentialsProvider);
    ss = std::make_shared<std::stringstream>(data);
    auto res = cliV1->putObject(bktName, objKey, ss);
    EXPECT_EQ(res.isSuccess(), true);

    // 带 config，Env 方式
    cliV1 = std::make_shared<TosClientV2>(TestConfig::Region, ecsCredentialsProvider, conf);
    ss = std::make_shared<std::stringstream>(data);
    res = cliV1->putObject(bktName, objKey, ss);
    EXPECT_EQ(res.isSuccess(), true);
}

}  // namespace VolcengineTos
