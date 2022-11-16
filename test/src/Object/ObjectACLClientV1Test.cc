#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectACLClientV1Test : public ::testing::Test {
protected:
    ObjectACLClientV1Test() = default;

    ~ObjectACLClientV1Test() override = default;

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

std::shared_ptr<TosClientV2> ObjectACLClientV1Test::cliV2 = nullptr;
std::string ObjectACLClientV1Test::bkt_name = "";

TEST_F(ObjectACLClientV1Test, PutGetObjectAclWithBodyTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "1234567890abcd";
    auto ss = std::make_shared<std::stringstream>(data);

    auto output_obj_put = cliV2->putObject(bkt_name, obj_key, ss);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto output_obj_get_acl = cliV2->getObjectAcl(bkt_name, obj_key);
    bool check_id = (output_obj_get_acl.result().getOwner().getId() ==
                     output_obj_get_acl.result().getGrant()[0].getGrantee().getId());
    bool check_Grantee_type = (output_obj_get_acl.result().getGrant()[0].getGrantee().getType() == "CanonicalUser");
    bool check_Permission = (output_obj_get_acl.result().getGrant()[0].getPermission() == "FULL_CONTROL");
    EXPECT_EQ(check_id & check_Grantee_type & check_Permission, true);
    
    PutObjectAclInput input_put_acl;
    input_put_acl.setKey(obj_key);
    ObjectAclGrant objectAclGrant;
    objectAclGrant.setAclPublicReadWrite();
    input_put_acl.setAclGrant(objectAclGrant);

    Owner owner;
    owner.setId("test-cid");
    Grantee grantee;
    grantee.setType("CanonicalUser");
    std::string id = "test-cid";
    grantee.setId(id);
    Grant grant;
    grant.setGrantee(grantee);
    grant.setPermission("FULL_CONTROL");
    ObjectAclRules objectAclRules;
    objectAclRules.setOwner(owner);
    objectAclRules.setGrants({grant});
    //    input_put_acl.setAclRules(objectAclRules);

    auto output_ = cliV2->putObjectAcl(bkt_name, input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);

    auto output_obj_get_acl_ = cliV2->getObjectAcl(bkt_name, obj_key);
    bool check_Permission_1 = (output_obj_get_acl_.result().getGrant()[0].getPermission() == "READ");
    bool check_Permission_2 = (output_obj_get_acl_.result().getGrant()[1].getPermission() == "WRITE");
    EXPECT_EQ(check_Permission_1 & check_Permission_2, true);
}

TEST_F(ObjectACLClientV1Test, PutObjectAclWithNonexistentNameTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string nonexistent_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectAclInput input_put_acl;
    input_put_acl.setKey(nonexistent_obj_name);
    auto output = cliV2->putObjectAcl(nonexistent_bkt_name, input_put_acl);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);

    input_put_acl.setKey(nonexistent_obj_name);
    ObjectAclGrant objectAclGrant;
    objectAclGrant.setAclPrivate();
    input_put_acl.setAclGrant(objectAclGrant);

    Owner owner;
    owner.setId("test-cid");
    Grantee grantee;
    grantee.setType("CanonicalUser");
    std::string id = "test-cid";
    grantee.setId(id);
    Grant grant;
    grant.setGrantee(grantee);
    grant.setPermission("WRITE_ACP");
    ObjectAclRules objectAclRules;
    objectAclRules.setOwner(owner);
    objectAclRules.setGrants({grant});
    // input_put_acl.setAclRules(objectAclRules);

    auto output_ = cliV2->putObjectAcl(bkt_name, input_put_acl);

    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getMessage() == "The specified key does not exist.", true);
}

TEST_F(ObjectACLClientV1Test, GetObjectAclWithNonexistentNameTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string nonexistent_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto output = cliV2->getObjectAcl(nonexistent_bkt_name, nonexistent_obj_name);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
    auto output_ = cliV2->getObjectAcl(bkt_name, nonexistent_obj_name);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getMessage() == "The specified key does not exist.", true);
}

}  // namespace VolcengineTos
