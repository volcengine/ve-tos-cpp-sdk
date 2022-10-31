#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class BucketACLTest : public ::testing::Test {
protected:
    BucketACLTest() {
    }

    ~BucketACLTest() override {
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

std::shared_ptr<TosClientV2> BucketACLTest::cliV2 = nullptr;
std::string BucketACLTest::bkt_name = "";

TEST_F(BucketACLTest, PutGetBucketAclWithBodyTest) {
    GetBucketAclInput input_get_acl(bkt_name);
    auto output_obj_get_acl = cliV2->getBucketAcl(input_get_acl);
    bool check_id = (output_obj_get_acl.result().getOwner().getId() ==
                     output_obj_get_acl.result().getGrant()[0].getGrantee().getId());
    bool check_Grantee_type =
            (output_obj_get_acl.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission = (output_obj_get_acl.result().getGrant()[0].getPermission() == PermissionType::FullControl);
    EXPECT_EQ(check_id & check_Grantee_type & check_Permission, true);

    PutBucketAclInput input_put_acl(bkt_name);
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
    auto output_ = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);
    GetBucketAclInput input_get_acl_(bkt_name);
    auto output_obj_get_acl_ = cliV2->getBucketAcl(input_get_acl_);

    bool check_Grantee_type_ =
            (output_obj_get_acl_.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission_ = (output_obj_get_acl_.result().getGrant()[0].getPermission() == PermissionType::WriteAcp);
    EXPECT_EQ(check_Grantee_type_ & check_Permission_, true);
}

TEST_F(BucketACLTest, PutGetBucketAclWithBody2Test) {
    auto bkt_name_0 = bkt_name + "0";
    TestUtils::CreateBucket(cliV2, bkt_name_0);
    GetBucketAclInput input_get_acl(bkt_name_0);
    auto output_obj_get_acl = cliV2->getBucketAcl(input_get_acl);
    bool check_id = (output_obj_get_acl.result().getOwner().getId() ==
                     output_obj_get_acl.result().getGrant()[0].getGrantee().getId());
    bool check_Grantee_type =
            (output_obj_get_acl.result().getGrant()[0].getGrantee().getType() == GranteeType::CanonicalUser);
    bool check_Permission = (output_obj_get_acl.result().getGrant()[0].getPermission() == PermissionType::FullControl);
    EXPECT_EQ(check_id & check_Grantee_type & check_Permission, true);

    PutBucketAclInput input_put_acl(bkt_name_0);
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
    auto output_ = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), true);
    GetBucketAclInput input_get_acl_(bkt_name_0);
    auto output_obj_get_acl_ = cliV2->getBucketAcl(input_get_acl_);
    bool check_Grantee_type_ =
            (output_obj_get_acl_.result().getGrant()[0].getGrantee().getType() == GranteeType::Group);
    bool check_Permission_ = (output_obj_get_acl_.result().getGrant()[0].getPermission() == PermissionType::WriteAcp);
    EXPECT_EQ(check_Grantee_type_ & check_Permission_, true);
    TestUtils::CleanBucket(cliV2, bkt_name_0);
}

TEST_F(BucketACLTest, PutGetBucketAclWithHeadTest) {
    auto bkt_name_1 = bkt_name + "1";
    TestUtils::CreateBucket(cliV2, bkt_name_1);
    PutBucketAclInput input_put_acl(bkt_name_1);
    input_put_acl.setAcl(ACLType::Private);
    auto output = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output.isSuccess(), true);

    GetBucketAclInput input_get_acl_(bkt_name_1);
    auto output_1 = cliV2->getBucketAcl(input_get_acl_);
    EXPECT_EQ(output_1.isSuccess(), true);
    EXPECT_EQ(output_1.result().getGrant()[0].getPermission() == PermissionType::FullControl, true);

    input_put_acl.setAcl(ACLType::BucketOwnerFullControl);
    auto output_2 = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output_2.isSuccess(), true);

    auto output_3 = cliV2->getBucketAcl(input_get_acl_);
    EXPECT_EQ(output_3.isSuccess(), true);
    EXPECT_EQ(output_3.result().getGrant()[0].getPermission() == PermissionType::FullControl, true);

    TestUtils::CleanBucket(cliV2, bkt_name_1);
}

TEST_F(BucketACLTest, PutBucketAclWithNonexistentNameTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    PutBucketAclInput input_put_acl(nonexistent_bkt_name);
    auto output = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
    input_put_acl.setBucket(nonexistent_bkt_name);
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
    auto output_ = cliV2->putBucketAcl(input_put_acl);
    EXPECT_EQ(output_.isSuccess(), false);
    EXPECT_EQ(output_.error().getStatusCode(), 404);
    EXPECT_EQ(output_.error().getMessage() == "The specified bucket does not exist.", true);
}
}  // namespace VolcengineTos
