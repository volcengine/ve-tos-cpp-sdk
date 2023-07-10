//#include "../TestConfig.h"
//#include "../Utils.h"
//#include "TosClientV2.h"
//#include <gtest/gtest.h>
//
// namespace VolcengineTos {
// class CallBackTest : public ::testing::Test {
// protected:
//    CallBackTest() {
//    }
//
//    ~CallBackTest() override {
//    }
//
//    static void SetUpTestCase() {
//        ClientConfig conf;
//        conf.endPoint = TestConfig::Endpoint;
//        conf.enableCRC = true;
//        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
//        bucketName = TestUtils::GetBucketName(TestConfig::TestPrefix);
//        TestUtils::CreateBucket(cliV2, bucketName);
//        workPath = FileUtils::getWorkPath();
//        std::cout << workPath << std::endl;
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
//    static std::string workPath;
//};
//
// std::shared_ptr<TosClientV2> CallBackTest::cliV2 = nullptr;
// std::string CallBackTest::bucketName = "";
// std::string CallBackTest::workPath = "";
//
// TEST_F(CallBackTest, putCallBackTest) {
//    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//
//    std::string data =
//            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./ :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
//            "()_+<>?,./   :'";
//    auto ss = std::make_shared<std::stringstream>(data);
//    PutObjectV2Input input(bucketName, obj_key, ss);
//    input.setCallBack(
//            "");
//    input.setCallBackVar("Cgl7CgkJIng6a2V5MSIgOiAiY2VzaGkiCgl9");
//    auto output_obj_put = cliV2->putObject(input);
//    EXPECT_EQ(output_obj_put.isSuccess(), true);
//    EXPECT_EQ(output_obj_put.result().getCallbackResult().empty(), false);
//}
//
// TEST_F(CallBackTest, multipartCallBackTest) {
//    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//    CreateMultipartUploadInput input_part_create(bucketName, obj_name);
//
//    auto upload = cliV2->createMultipartUpload(input_part_create);
//
//    // generate some data..
//    auto ss1 = std::make_shared<std::stringstream>();
//    for (int i = 0; i < (5 << 20); ++i) {
//        *ss1 << 1;
//    }
//    UploadPartV2Input input_upload_part(bucketName, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
//    auto part1 = cliV2->uploadPart(input_upload_part);
//    EXPECT_EQ(part1.isSuccess(), true);
//
//    // generate some data..
//    auto ss2 = std::make_shared<std::stringstream>();
//    for (int i = 0; i < (4 << 20); ++i) {
//        *ss2 << 2;
//    }
//    UploadPartV2Input input_upload_part_2(bucketName, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
//    auto part2 = cliV2->uploadPart(input_upload_part_2);
//    EXPECT_EQ(part2.isSuccess(), true);
//    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
//    auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
//    std::vector<UploadedPartV2> vec = {part_1, part_2};
//    CompleteMultipartUploadV2Input input_upload_complete(bucketName, obj_name, upload.result().getUploadId(), vec);
//    input_upload_complete.setCallBack(
//            "");
//    input_upload_complete.setCallBackVar("Cgl7CgkJIng6a2V5MSIgOiAiY2VzaGkiCgl9");
//
//    auto com = cliV2->completeMultipartUpload(input_upload_complete);
//    EXPECT_EQ(com.isSuccess(), true);
//    EXPECT_EQ(com.result().getCallbackResult().empty(), false);
//}
//}  // namespace VolcengineTos
