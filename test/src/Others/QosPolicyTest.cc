#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class QosPolicyTest : public ::testing::Test {
protected:
    QosPolicyTest() {
    }

    ~QosPolicyTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.enableVerifySSL = TestConfig::enableVerifySSL;
        conf.endPoint = TestConfig::Endpoint;
        conf.controlEndPoint = TestConfig::ControlEndpoint;

        ClientConfig conf2;
        conf2.enableVerifySSL = TestConfig::enableVerifySSL;
        conf2.endPoint = TestConfig::Endpoint;

        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV2Invalid1 = std::make_shared<TosClientV2>("test-region", TestConfig::Ak, TestConfig::Sk, conf2);
        cliV2Invalid2 = std::make_shared<TosClientV2>("test-region", TestConfig::Ak, TestConfig::Sk);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        cliV2 = nullptr;
        cliV2Invalid1 = nullptr;
        cliV2Invalid2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::shared_ptr<TosClientV2> cliV2Invalid1;
    static std::shared_ptr<TosClientV2> cliV2Invalid2;
};

std::shared_ptr<TosClientV2> QosPolicyTest::cliV2 = nullptr;
std::shared_ptr<TosClientV2> QosPolicyTest::cliV2Invalid1 = nullptr;
std::shared_ptr<TosClientV2> QosPolicyTest::cliV2Invalid2 = nullptr;

TEST_F(QosPolicyTest, PutQosPolicyTest) {
    std::string bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);

    std::string accountID = TestConfig::AccountID;
    std::string qos_policy = "{\"Version\":\"2012-10-17\",\"Statement\":[{\"Sid\":\"statement1\",\"Quota\":{\"WritesQps\":\"100\",\"ReadsQps\":\"100\",\"ListQps\":\"100\",\"WritesRate\":\"10\",\"ReadsRate\":\"10\"},\"Resource\":[\"trn:tos:::examplebucket1/*\"],\"Principal\":[\"trn:iam::*\"],\"Condition\":{\"StringEquals\":{\"AccessPoint\":\"BktB.tos-cn-beijing.volces.com\",\"NetPlane\":\"public\"}}}]}";

    DeleteQosPolicyInput delete_qos_policy_input(accountID);
    auto delete_qos_policy_output = cliV2->deleteQosPolicy(delete_qos_policy_input);
    EXPECT_EQ(delete_qos_policy_output.isSuccess(), true);

    PutQosPolicyInput qos_policy_input(accountID, qos_policy);
    auto qos_policy_output = cliV2->putQosPolicy(qos_policy_input);
    EXPECT_EQ(qos_policy_output.isSuccess(), true);

    qos_policy_output = cliV2->putQosPolicy(qos_policy_input);
    EXPECT_EQ(qos_policy_output.isSuccess(), false);
    EXPECT_EQ(qos_policy_output.error().getCode(), "UpdateConflict");

    GetQosPolicyInput get_qos_policy_input(accountID);
    auto get_qos_policy_output = cliV2->getQosPolicy(get_qos_policy_input);
    EXPECT_EQ(get_qos_policy_output.isSuccess(), true);
    EXPECT_EQ(get_qos_policy_output.result().getPolicy().empty(), false);

    PutQosPolicyInput qos_policy_input2(accountID, get_qos_policy_output.result().getPolicy());
    auto qos_policy_output2 = cliV2->putQosPolicy(qos_policy_input2);
    EXPECT_EQ(qos_policy_output2.isSuccess(), true);

    delete_qos_policy_output = cliV2->deleteQosPolicy(delete_qos_policy_input);
    EXPECT_EQ(delete_qos_policy_output.isSuccess(), true);

    get_qos_policy_output = cliV2->getQosPolicy(get_qos_policy_input);
    EXPECT_EQ(get_qos_policy_output.isSuccess(), false);
    EXPECT_EQ(get_qos_policy_output.error().getCode(), "NoSuchQosPolicy");

    qos_policy_output2 = cliV2Invalid1->putQosPolicy(qos_policy_input2);
    EXPECT_EQ(qos_policy_output2.isSuccess(), false);
    EXPECT_EQ(qos_policy_output2.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");

    get_qos_policy_output = cliV2Invalid1->getQosPolicy(get_qos_policy_input);
    EXPECT_EQ(get_qos_policy_output.isSuccess(), false);
    EXPECT_EQ(get_qos_policy_output.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");

    delete_qos_policy_output = cliV2Invalid1->deleteQosPolicy(delete_qos_policy_input);
    EXPECT_EQ(delete_qos_policy_output.isSuccess(), false);
    EXPECT_EQ(delete_qos_policy_output.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");

    qos_policy_output2 = cliV2Invalid2->putQosPolicy(qos_policy_input2);
    EXPECT_EQ(qos_policy_output2.isSuccess(), false);
    EXPECT_EQ(qos_policy_output2.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");

    get_qos_policy_output = cliV2Invalid2->getQosPolicy(get_qos_policy_input);
    EXPECT_EQ(get_qos_policy_output.isSuccess(), false);
    EXPECT_EQ(get_qos_policy_output.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");

    delete_qos_policy_output = cliV2Invalid2->deleteQosPolicy(delete_qos_policy_input);
    EXPECT_EQ(delete_qos_policy_output.isSuccess(), false);
    EXPECT_EQ(delete_qos_policy_output.error().getMessage(), "invalid control endpoint, the control endpoint must be not empty");
}

}  // namespace VolcengineTos
