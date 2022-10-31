#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectUploadPartAndCompleteTest : public ::testing::Test {
protected:
    ObjectUploadPartAndCompleteTest() {
    }

    ~ObjectUploadPartAndCompleteTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::Endpoint;
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
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
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ObjectUploadPartAndCompleteTest::cliV2 = nullptr;
std::string ObjectUploadPartAndCompleteTest::bkt_name = "";
std::string ObjectUploadPartAndCompleteTest::workPath = "";

TEST_F(ObjectUploadPartAndCompleteTest, CreateMultipartUploadTest) {
    std::string filePath = workPath + "test/testdata/" + "AppendTest.txt";

    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);

    auto upload = cliV2->createMultipartUpload(input_part_create);

    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }
    UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto part2 = cliV2->uploadPart(input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);
    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
    auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
    std::vector<UploadedPartV2> vec = {part_1, part_2};
    CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
    auto com = cliV2->completeMultipartUpload(input_upload_complete);
    EXPECT_EQ(com.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto got = cliV2->getObject(input_obj_get);

    GetObjectToFileInput input_obj_get_tofile(bkt_name, obj_name, filePath);
    auto out_obj_get = cliV2->getObjectToFile(input_obj_get_tofile);

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

TEST_F(ObjectUploadPartAndCompleteTest, CreateMultipartUploadFromFileTest) {
    std::string filePath = workPath + "test/testdata/" + "AppendTest2.txt";
    std::string filePath2 = workPath + "test/testdata/" + "AppendDownTest.txt";

    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);

    auto upload = cliV2->createMultipartUpload(input_part_create);
    int64_t offset = 4;
    int64_t partsize = (5 << 20) + 1;
    UploadPartFromFileInput input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), 1, filePath, offset,
                                              partsize);
    auto part1 = cliV2->uploadPartFromFile(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);

    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss2 << 2;
    }
    UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto part2 = cliV2->uploadPart(input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);

    offset += partsize;
    partsize = (4 << 20);
    UploadPartFromFileInput input_upload_part_3(bkt_name, obj_name, upload.result().getUploadId(), 3, filePath, offset,
                                                partsize);
    auto part3 = cliV2->uploadPartFromFile(input_upload_part_3);

    auto part1_basic = part1.result().getUploadPartV2Output();
    auto part3_basic = part3.result().getUploadPartV2Output();
    auto part_1 = UploadedPartV2(part1_basic.getPartNumber(), part1_basic.getETag());
    auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
    auto part_3 = UploadedPartV2(part3_basic.getPartNumber(), part3_basic.getETag());

    std::vector<UploadedPartV2> vec = {part_1, part_2, part_3};
    CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
    auto com = cliV2->completeMultipartUpload(input_upload_complete);
    EXPECT_EQ(com.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_name);
    auto got = cliV2->getObject(input_obj_get);

    GetObjectToFileInput input_obj_get_tofile(bkt_name, obj_name, filePath2);
    auto out_obj_get = cliV2->getObjectToFile(input_obj_get_tofile);

    std::ostringstream ss;
    ss << got.result().getContent()->rdbuf();
    std::string s = ss.str();
    std::map<std::string, std::string> meta = TestUtils::GetObjectMeta(cliV2, bkt_name, obj_name);

    int data_size_1 = (5 << 20);
    int data_size_2 = (5 << 20);
    int data_size_3 = (4 << 20);
    std::string data_1 = std::string(data_size_1, '1');
    std::string data_2 = std::string(data_size_2, '2');
    std::string data_3 = std::string(data_size_3, '2');
    std::string data = '2' + data_1 + data_2 + data_3;
    bool check_data = (s == data);
    auto s_md5 = CryptoUtils::md5Sum(s);
    auto data_md5 = CryptoUtils::md5Sum(data);
    bool check_md5 = (s_md5 == data_md5);
    EXPECT_EQ(check_data & check_md5, true);
    EXPECT_EQ(check_data, true);
}

TEST_F(ObjectUploadPartAndCompleteTest, UploadpartToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "notexistent";
    std::string upload_id_ = "1234";
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
    auto upload = cliV2->createMultipartUpload(input_part_create);
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part.isSuccess(), true);

    auto basic = input_upload_part.getUploadPartBasicInput();
    basic.setBucket(bkt_name_);
    input_upload_part.setUploadPartBasicInput(basic);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), false);
    EXPECT_EQ(part1.error().getStatusCode(), 404);
    EXPECT_EQ(part1.error().getCode(), "NoSuchBucket");

    basic.setBucket(bkt_name);
    basic.setKey(obj_name_);
    input_upload_part.setUploadPartBasicInput(basic);
    auto part2 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part2.isSuccess(), false);
    EXPECT_EQ(part2.error().getStatusCode(), 404);
    EXPECT_EQ(part2.error().getCode(), "NoSuchUpload");

    basic.setKey(obj_name);
    basic.setUploadId(upload_id_);
    input_upload_part.setUploadPartBasicInput(basic);

    auto part3 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part3.isSuccess(), false);
    EXPECT_EQ(part3.error().getStatusCode(), 404);
    EXPECT_EQ(part3.error().getCode(), "NoSuchUpload");
}
TEST_F(ObjectUploadPartAndCompleteTest, listpartsToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix) + "notexistent";
    std::string upload_id_ = "1234";
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
    auto upload = cliV2->createMultipartUpload(input_part_create);
    ListPartsInput input_part_list(bkt_name, obj_name, upload.result().getUploadId());
    auto part = cliV2->listParts(input_part_list);
    EXPECT_EQ(part.isSuccess(), true);

    input_part_list.setBucket(bkt_name_);
    auto part1 = cliV2->listParts(input_part_list);
    EXPECT_EQ(part1.isSuccess(), false);
    EXPECT_EQ(part1.error().getStatusCode(), 404);
    EXPECT_EQ(part1.error().getCode(), "NoSuchBucket");

    input_part_list.setBucket(bkt_name);
    input_part_list.setUploadId(upload_id_);
    auto part2 = cliV2->listParts(input_part_list);
    EXPECT_EQ(part2.isSuccess(), false);
    EXPECT_EQ(part2.error().getStatusCode(), 404);
    EXPECT_EQ(part2.error().getCode(), "NotFound");

    input_part_list.setKey(obj_name_);
    input_part_list.setUploadId(upload.result().getUploadId());
    auto part3 = cliV2->listParts(input_part_list);
    EXPECT_EQ(part3.isSuccess(), false);
    EXPECT_EQ(part3.error().getStatusCode(), 404);
    EXPECT_EQ(part3.error().getCode(), "NotFound");

    input_part_list.setKey(obj_name);
    auto part4 = cliV2->listParts(input_part_list);
    EXPECT_EQ(part4.isSuccess(), true);
}

TEST_F(ObjectUploadPartAndCompleteTest, CompleteToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
    auto upload = cliV2->createMultipartUpload(input_part_create);
    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    auto part1 = cliV2->uploadPart(input_upload_part);
    EXPECT_EQ(part1.isSuccess(), true);
    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }
    UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    auto part2 = cliV2->uploadPart(input_upload_part_2);
    EXPECT_EQ(part2.isSuccess(), true);
    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
    auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
    std::vector<UploadedPartV2> vec = {part_1, part_2};

    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string obj_name_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string upload_id_ = "1234";

    CompleteMultipartUploadV2Input input_upload_complete(bkt_name_, obj_name, upload_id_, vec);
    auto com = cliV2->completeMultipartUpload(input_upload_complete);
    EXPECT_EQ(com.isSuccess(), false);
    EXPECT_EQ(com.error().getStatusCode(), 404);
    EXPECT_EQ(com.error().getCode(), "NoSuchBucket");

    CompleteMultipartUploadV2Input input_upload_complete_2(bkt_name, obj_name, upload_id_, vec);
    auto com_2 = cliV2->completeMultipartUpload(input_upload_complete_2);
    EXPECT_EQ(com_2.isSuccess(), false);
    EXPECT_EQ(com_2.error().getStatusCode(), 404);
    EXPECT_EQ(com_2.error().getCode(), "NotFound");

    CompleteMultipartUploadV2Input input_upload_complete_3(bkt_name, obj_name_, upload.result().getUploadId(), vec);
    auto com_3 = cliV2->completeMultipartUpload(input_upload_complete_3);
    EXPECT_EQ(com_3.isSuccess(), false);
    EXPECT_EQ(com_3.error().getStatusCode(), 404);
    EXPECT_EQ(com_3.error().getCode(), "NotFound");
}

}  // namespace VolcengineTos
