#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectCreateAndListMultipartUploadClientV1Test : public ::testing::Test {
protected:
    ObjectCreateAndListMultipartUploadClientV1Test() {
    }

    ~ObjectCreateAndListMultipartUploadClientV1Test() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        cliV1 = std::make_shared<TosClient>(TestConfig::Endpoint, TestConfig::Region, TestConfig::Ak, TestConfig::Sk);

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
    static std::shared_ptr<TosClient> cliV1;

    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> ObjectCreateAndListMultipartUploadClientV1Test::cliV2 = nullptr;
std::shared_ptr<TosClient> ObjectCreateAndListMultipartUploadClientV1Test::cliV1 = nullptr;
std::string ObjectCreateAndListMultipartUploadClientV1Test::bkt_name = "";

TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, CreateListMultipartUploadTest) {
    RequestOptionBuilder rob_create;
    for (int i = 0; i < 100; ++i) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << i;
        std::string idx_string;
        ss >> idx_string;
        rob_create.withMeta("self-test", idx_string);
        auto upload = cliV1->createMultipartUpload(bkt_name, idx_string, rob_create);
    }
    ListMultipartUploadsInput input_multipart_list;
    input_multipart_list.setMaxUploads(10);

    std::string start_marker = "";
    std::string next_marker = "";
    int i = 0;
    for (int t = 0; t < 10; ++t) {
        next_marker = start_marker;
        input_multipart_list.setKeyMarker(next_marker);
        auto output = cliV1->listMultipartUploads(bkt_name, input_multipart_list);
        EXPECT_EQ(output.isSuccess(), true);
        start_marker = output.result().getNextKeyMarker();
        auto uploads = output.result().getUpload();
        for (int p = 0; p < 10; ++p) {
            std::stringstream ss;
            ss << std::setw(3) << std::setfill('0') << i;
            std::string idx_string;
            ss >> idx_string;
            bool check_upload_part_name = (uploads[p].getKey() == idx_string);
            EXPECT_EQ(check_upload_part_name, true);
            i++;
        }
    }
}
TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, AbortMultipartUploadTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    auto upload_1 = cliV1->createMultipartUpload(bkt_name_, "test-obj-001");
    auto upload_2 = cliV1->createMultipartUpload(bkt_name_, "test-obj-002");

    AbortMultipartUploadInput input_1("test-obj-001", upload_1.result().getUploadId());
    AbortMultipartUploadInput input_2("test-obj-002", upload_2.result().getUploadId());
    ListMultipartUploadsInput input_multipart_list;
    input_multipart_list.setMaxUploads(100);
    auto output_list_1 = cliV1->listMultipartUploads(bkt_name_, input_multipart_list);

    EXPECT_EQ(output_list_1.isSuccess(), true);
    EXPECT_EQ(output_list_1.result().getUpload().size(), 2);

    auto output_abort_1 = cliV1->abortMultipartUpload(bkt_name_, input_1);
    EXPECT_EQ(output_abort_1.isSuccess(), true);
    auto output_abort_2 = cliV1->abortMultipartUpload(bkt_name_, input_2);
    EXPECT_EQ(output_abort_2.isSuccess(), true);

    auto output_list_2 = cliV1->listMultipartUploads(bkt_name_, input_multipart_list);
    EXPECT_EQ(output_list_2.isSuccess(), true);
    EXPECT_EQ(output_list_2.result().getUpload().size(), 0);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}

TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, ListObjectWith3PrefixObjectsTest) {
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    TestUtils::CreateBucket(cliV2, bkt_name_);
    std::string obj_name_1 = "11/22/33/4444";
    auto upload_1 = cliV1->createMultipartUpload(bkt_name_, obj_name_1);
    std::string obj_name_2 = "11/22/34/4444";
    auto upload_2 = cliV1->createMultipartUpload(bkt_name_, obj_name_2);

    ListMultipartUploadsInput input_multipart_list;
    input_multipart_list.setMaxUploads(10);
    input_multipart_list.setPrefix("11/22/");
    input_multipart_list.setDelimiter("/");
    auto output = cliV1->listMultipartUploads(bkt_name_, input_multipart_list);
    EXPECT_EQ(output.isSuccess(), true);
    EXPECT_EQ(output.result().getCommonPrefixes()[0].getPrefix() == "11/22/33/", true);
    EXPECT_EQ(output.result().getCommonPrefixes()[1].getPrefix() == "11/22/34/", true);
    TestUtils::CleanBucket(cliV2, bkt_name_);
}
TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, CreateUploadsFromNonexistentBucketTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);

    auto output = cliV1->createMultipartUpload(nonexistent_bkt_name, obj_name);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}
TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, ListUploadsFromNonexistentBucketTest) {
    std::string nonexistent_bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
    ListMultipartUploadsInput input_multipart_list;
    input_multipart_list.setMaxUploads(10);
    auto output = cliV1->listMultipartUploads(nonexistent_bkt_name, input_multipart_list);
    EXPECT_EQ(output.isSuccess(), false);
    EXPECT_EQ(output.error().getStatusCode(), 404);
    EXPECT_EQ(output.error().getMessage() == "The specified bucket does not exist.", true);
}
TEST_F(ObjectCreateAndListMultipartUploadClientV1Test, AbortUploadsFromNonexistentBucketTest) {
    std::string obj_name_create = "112233";

    auto upload_1 = cliV1->createMultipartUpload(bkt_name, obj_name_create);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string upload_id = "11edee11aeec61fd11d448f505be2ef3";

    AbortMultipartUploadInput input_multipart_abort(obj_name_, upload_id);
    auto output_1 = cliV1->abortMultipartUpload(bkt_name_, input_multipart_abort);
    EXPECT_EQ(output_1.isSuccess(), false);
    EXPECT_EQ(output_1.error().getStatusCode(), 404);
    EXPECT_EQ(output_1.error().getMessage() == "The specified bucket does not exist.", true);

    auto output_2 = cliV1->abortMultipartUpload(bkt_name_, input_multipart_abort);
    EXPECT_EQ(output_2.isSuccess(), false);
    EXPECT_EQ(output_2.error().getStatusCode(), 404);

    auto output_3 = cliV1->abortMultipartUpload(bkt_name_, input_multipart_abort);
    EXPECT_EQ(output_3.isSuccess(), false);
    EXPECT_EQ(output_3.error().getStatusCode(), 404);
}
}  // namespace VolcengineTos
