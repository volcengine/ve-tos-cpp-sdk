#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectUploadPartAndCompleteClientV1Test : public ::testing::Test {
protected:
    ObjectUploadPartAndCompleteClientV1Test() {
    }

    ~ObjectUploadPartAndCompleteClientV1Test() override {
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
        // TestUtils::CleanAllBucket(cliV2);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> ObjectUploadPartAndCompleteClientV1Test::cliV2 = nullptr;
std::string ObjectUploadPartAndCompleteClientV1Test::bkt_name = "";

TEST_F(ObjectUploadPartAndCompleteClientV1Test, CreateMultipartUploadTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto upload = cliV2->createMultipartUpload(bkt_name, obj_name);

    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartInput input_upload_part(obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(bkt_name, input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }

    UploadPartInput input_upload_part_2(obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto part2 = cliV2->uploadPart(bkt_name, input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);
    CompleteMultipartUploadInput input(obj_name, upload.result().getUploadId(), {part1.result(), part2.result()});
    auto com = cliV2->completeMultipartUpload(bkt_name, input);
    EXPECT_EQ(com.isSuccess(), true);
    auto got = cliV2->getObject(bkt_name, obj_name);
    //    std::ofstream outFile;
    //    outFile.open("/Users/bytedance/11");
    //    outFile << got.result().getContent()->rdbuf();
    //    outFile.close();

    std::ostringstream ss;
    ss << got.result().getContent()->rdbuf();
    std::string s = ss.str();
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);

    int data_size_1 = (5 << 20);
    int data_size_2 = (4 << 20);
    std::string data_1 = std::string(data_size_1, '1');
    std::string data_2 = std::string(data_size_2, '2');
    std::string data = data_1 + data_2;
    bool check_data = (s == data);
    EXPECT_EQ(check_data, true);
}

TEST_F(ObjectUploadPartAndCompleteClientV1Test, UploadpartToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "notexistent";
    std::string upload_id_ = "1234";
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }

    auto upload = cliV2->createMultipartUpload(bkt_name, obj_name);
    UploadPartInput input_upload_part(obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part = cliV2->uploadPart(bkt_name, input_upload_part);
    EXPECT_EQ(part.isSuccess(), true);

    UploadPartInput input_upload_part_1(obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(bkt_name_, input_upload_part_1);
    EXPECT_EQ(part1.isSuccess(), false);
    EXPECT_EQ(part1.error().getStatusCode(), 404);
    EXPECT_EQ(part1.error().getCode(), "NoSuchBucket");

    UploadPartInput input_upload_part_2(obj_name, upload_id_, ss1->tellg(), 1, ss1);
    auto part2 = cliV2->uploadPart(bkt_name, input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), false);
    EXPECT_EQ(part2.error().getStatusCode(), 404);
    EXPECT_EQ(part2.error().getCode(), "NoSuchUpload");

    UploadPartInput input_upload_part_3(obj_name_, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part3 = cliV2->uploadPart(bkt_name, input_upload_part_3);
    EXPECT_EQ(part3.isSuccess(), false);
    EXPECT_EQ(part3.error().getStatusCode(), 404);
    EXPECT_EQ(part3.error().getCode(), "NoSuchUpload");
}
TEST_F(ObjectUploadPartAndCompleteClientV1Test, listpartsToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "notexistent";
    std::string upload_id_ = "1234";
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }

    auto upload = cliV2->createMultipartUpload(bkt_name, obj_name);
    ListUploadedPartsInput input_part_list;
    input_part_list.setKey(obj_name);
    input_part_list.setUploadId(upload.result().getUploadId());

    auto part = cliV2->listUploadedParts(bkt_name, input_part_list);
    EXPECT_EQ(part.isSuccess(), true);

    auto part1 = cliV2->listUploadedParts(bkt_name_, input_part_list);
    EXPECT_EQ(part1.isSuccess(), false);
    EXPECT_EQ(part1.error().getStatusCode(), 404);
    EXPECT_EQ(part1.error().getCode(), "NoSuchBucket");

    input_part_list.setUploadId(upload_id_);
    auto part2 = cliV2->listUploadedParts(bkt_name, input_part_list);
    EXPECT_EQ(part2.isSuccess(), false);
    EXPECT_EQ(part2.error().getStatusCode(), 404);
    EXPECT_EQ(part2.error().getCode(), "NoSuchUpload");

    input_part_list.setKey(obj_name_);
    input_part_list.setUploadId(upload.result().getUploadId());
    auto part3 = cliV2->listUploadedParts(bkt_name, input_part_list);
    EXPECT_EQ(part3.isSuccess(), false);
    EXPECT_EQ(part3.error().getStatusCode(), 404);
    EXPECT_EQ(part3.error().getCode(), "NoSuchUpload");

    input_part_list.setKey(obj_name);
    auto part4 = cliV2->listUploadedParts(bkt_name, input_part_list);
    EXPECT_EQ(part4.isSuccess(), true);
}

TEST_F(ObjectUploadPartAndCompleteClientV1Test, CompleteToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto upload = cliV2->createMultipartUpload(bkt_name, obj_name);

    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartInput input_upload_part(obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(bkt_name, input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }

    UploadPartInput input_upload_part_2(obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto part2 = cliV2->uploadPart(bkt_name, input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);

    auto vec = {part1.result(), part2.result()};

    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string upload_id_ = "1234";

    CompleteMultipartUploadInput input_upload_complete(obj_name, upload_id_, vec);
    auto com = cliV2->completeMultipartUpload(bkt_name_, input_upload_complete);
    EXPECT_EQ(com.isSuccess(), false);
    EXPECT_EQ(com.error().getStatusCode(), 404);
    EXPECT_EQ(com.error().getCode(), "NoSuchBucket");

    CompleteMultipartUploadInput input_upload_complete_2(obj_name, upload_id_, vec);
    auto com_2 = cliV2->completeMultipartUpload(bkt_name, input_upload_complete_2);
    EXPECT_EQ(com_2.isSuccess(), false);
    EXPECT_EQ(com_2.error().getStatusCode(), 404);
    EXPECT_EQ(com_2.error().getCode(), "NoSuchUpload");

    CompleteMultipartUploadInput input_upload_complete_3(obj_name_, upload.result().getUploadId(), vec);
    auto com_3 = cliV2->completeMultipartUpload(bkt_name, input_upload_complete_3);
    EXPECT_EQ(com_3.isSuccess(), false);
    EXPECT_EQ(com_3.error().getStatusCode(), 404);
    EXPECT_EQ(com_3.error().getCode(), "NoSuchUpload");
}

}  // namespace VolcengineTos
