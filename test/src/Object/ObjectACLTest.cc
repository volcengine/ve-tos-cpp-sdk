#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectACLTest : public ::testing::Test {
protected:
    ObjectACLTest() {
    }

    ~ObjectACLTest() override {
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

std::shared_ptr<TosClientV2> ObjectACLTest::cliV2 = nullptr;
std::string ObjectACLTest::bkt_name = "";

TEST_F(ObjectACLTest, PutGetObjectAclWithHeadTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "1234567890abcd";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    PutObjectAclV2Input input_put_acl(bkt_name, obj_key);
    input_put_acl.setAcl(ACLType::PublicReadWrite);
    auto output_ = cliV2->putObjectAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);
}
TEST_F(ObjectACLTest, PutGetObjectAclWithBodyTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "1234567890abcd";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectAclV2Input input_get_acl(bkt_name, obj_key);
    auto output_obj_get_acl = cliV2->getObjectAcl(input_get_acl);
    bool check_id = (output_obj_get_acl.result().getOwner().getId() ==
                     output_obj_get_acl.result().getGrant()[0].getGrantee().getId());
    bool check_Grantee_type =
            (output_obj_get_acl.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission = (output_obj_get_acl.result().getGrant()[0].getPermission() == PermissionType::FullControl);
    EXPECT_EQ(check_id & check_Grantee_type & check_Permission, true);

    PutObjectAclV2Input input_put_acl(bkt_name, obj_key);
    Owner owner;
    owner.setId("accountId");
    GranteeV2 granteev2;
    granteev2.setType(GranteeType::CanonicalUser);
    std::string id = "test-cid";
    granteev2.setId(id);
    GrantV2 grantv2;
    grantv2.setGrantee(granteev2);
    grantv2.setPermission(PermissionType::WriteAcp);
    input_put_acl.setOwner(owner);
    input_put_acl.setGrants({grantv2});
    // input_put_acl.setAcl(ACLType::PublicReadWrite);
    auto output_ = cliV2->putObjectAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);
    GetObjectAclV2Input input_get_acl_(bkt_name, obj_key);
    auto output_obj_get_acl_ = cliV2->getObjectAcl(input_get_acl_);

    bool check_Grantee_type_ =
            (output_obj_get_acl_.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission_ = (output_obj_get_acl_.result().getGrant()[0].getPermission() == PermissionType::WriteAcp);
    EXPECT_EQ(check_Grantee_type_ & check_Permission_, true);
}

TEST_F(ObjectACLTest, PutGetObjectAclWithBody2Test) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "1234567890abcd";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectAclV2Input input_get_acl(bkt_name, obj_key);
    auto output_obj_get_acl = cliV2->getObjectAcl(input_get_acl);
    bool check_id = (output_obj_get_acl.result().getOwner().getId() ==
                     output_obj_get_acl.result().getGrant()[0].getGrantee().getId());
    bool check_Grantee_type =
            (output_obj_get_acl.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission = (output_obj_get_acl.result().getGrant()[0].getPermission() == PermissionType::FullControl);
    EXPECT_EQ(check_id & check_Grantee_type & check_Permission, true);

    PutObjectAclV2Input input_put_acl(bkt_name, obj_key);
    Owner owner;
    owner.setId("test-cid");
    GranteeV2 granteev2;
    granteev2.setType(GranteeType::Group);
    granteev2.setCanned(CannedType::AllUsers);
    GrantV2 grantv2;
    grantv2.setGrantee(granteev2);
    grantv2.setPermission(PermissionType::WriteAcp);
    input_put_acl.setOwner(owner);
    input_put_acl.setGrants({grantv2});
    auto output_ = cliV2->putObjectAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);
    GetObjectAclV2Input input_get_acl_(bkt_name, obj_key);
    auto output_obj_get_acl_ = cliV2->getObjectAcl(input_get_acl_);
    bool check_Grantee_type_ =
            (output_obj_get_acl_.result().getGrant()[0].getGrantee().getType() == GranteeType::Group);
    bool check_Permission_ = (output_obj_get_acl_.result().getGrant()[0].getPermission() == PermissionType::WriteAcp);
    EXPECT_EQ(check_Grantee_type_ & check_Permission_, true);
}

TEST_F(ObjectACLTest, PutObjectAclWithNonexistentNameTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string nonexistent_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectAclV2Input input_put_acl(nonexistent_bkt_name, nonexistent_obj_name);
    auto output = cliV2->putObjectAcl(input_put_acl);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
    input_put_acl.setBucket(bkt_name);
    Owner owner;
    owner.setId("test-cid");
    GranteeV2 granteev2;
    granteev2.setType(GranteeType::CanonicalUser);
    granteev2.setId("test-cid");
    GrantV2 grantv2;
    grantv2.setGrantee(granteev2);
    grantv2.setPermission(PermissionType::FullControl);
    input_put_acl.setOwner(owner);
    input_put_acl.setGrants({grantv2});
    auto output_ = cliV2->putObjectAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getMessage() == "The specified key does not exist.", true);
}
TEST_F(ObjectACLTest, GetObjectAclWithNonexistentNameTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string nonexistent_obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    GetObjectAclV2Input input_get_acl(nonexistent_bkt_name, nonexistent_obj_name);
    auto output = cliV2->getObjectAcl(input_get_acl);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
    input_get_acl.setBucket(bkt_name);
    auto output_ = cliV2->getObjectAcl(input_get_acl);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getMessage() == "The specified key does not exist.", true);
}

}  // namespace VolcengineTos
