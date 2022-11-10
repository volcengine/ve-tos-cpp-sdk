#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectUploadPartCopyTest : public ::testing::Test {
protected:
    ObjectUploadPartCopyTest() {
    }

    ~ObjectUploadPartCopyTest() override {
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

std::shared_ptr<TosClientV2> ObjectUploadPartCopyTest::cliV2 = nullptr;
std::string ObjectUploadPartCopyTest::bkt_name = "";

TEST_F(ObjectUploadPartCopyTest, CreateUploadAndGetMultipartObjectTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto srcPut = cliV2->putObject(input_obj_put);
    EXPECT_EQ(srcPut.isSuccess(), true);

    std::string dstKey = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, dstKey);
    auto upload = cliV2->createMultipartUpload(input_part_create);
    EXPECT_EQ(upload.isSuccess(), true);
    // 获取要copy的对象大小
    HeadObjectV2Input input_head(bkt_name, obj_name);
    auto head = cliV2->headObject(input_head);
    EXPECT_EQ(head.isSuccess(), true);
    int64_t size = head.result().getContentLength();

    int partSize = 5 * 1024 * 1024;  // 10MB
    int partCount = (int)(size / partSize);
    std::vector<UploadedPartV2> copyParts(partCount);
    int64_t copySourceRangeEnd_ = 0;
    for (int i = 0; i < partCount; ++i) {
        int64_t partLen = partSize;
        if (partCount == i + 1 && (size % partLen) > 0) {
            partLen += size % (int64_t)partSize;
        }
        int64_t copySourceRangeStart_ = copySourceRangeEnd_;
        copySourceRangeEnd_ = copySourceRangeStart_ + partLen;

        UploadPartCopyV2Input input(bkt_name, dstKey, bkt_name, obj_name, i + 1, upload.result().getUploadId());
        input.setCopySourceRangeStart(copySourceRangeStart_);
        input.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto res = cliV2->uploadPartCopy(input);
        UploadedPartV2 temp(res.result().getPartNumber(), res.result().getETag());
        copyParts[i] = temp;
        copySourceRangeEnd_ = copySourceRangeEnd_ + 1;
    }

    CompleteMultipartUploadV2Input input(bkt_name, dstKey, upload.result().getUploadId(), copyParts);
    auto complete = cliV2->completeMultipartUpload(input);
    EXPECT_EQ(complete.isSuccess(), true);

    std::string content = TestUtils::GetObjectContentByStream(cliV2, bkt_name, dstKey);

    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == content);
    EXPECT_EQ(check_data, true);
}

TEST_F(ObjectUploadPartCopyTest, UploadpartToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = bkt_name + "nonexistent";
    std::string obj_name_ = obj_name + "nonexistent";

    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto srcPut = cliV2->putObject(input_obj_put);
    EXPECT_EQ(srcPut.isSuccess(), true);

    std::string dstKey = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    CreateMultipartUploadInput input_part_create(bkt_name, dstKey);
    auto upload = cliV2->createMultipartUpload(input_part_create);
    EXPECT_EQ(upload.isSuccess(), true);
    // 获取要copy的对象大小
    HeadObjectV2Input input_head(bkt_name, obj_name);
    auto head = cliV2->headObject(input_head);
    EXPECT_EQ(head.isSuccess(), true);
    int64_t size = head.result().getContentLength();

    int partSize = 5 * 1024 * 1024;  // 10MB
    int partCount = (int)(size / partSize);
    std::vector<UploadedPartV2> copyParts(partCount);
    int64_t copySourceRangeEnd_ = 0;
    for (int i = 0; i < partCount; ++i) {
        int64_t partLen = partSize;
        if (partCount == i + 1 && (size % partLen) > 0) {
            partLen += size % (int64_t)partSize;
        }
        int64_t copySourceRangeStart_ = copySourceRangeEnd_;
        copySourceRangeEnd_ = copySourceRangeStart_ + partLen;

        // 从不存在的桶复制段
        UploadPartCopyV2Input input_1(bkt_name_, dstKey, bkt_name, obj_name, i + 1, upload.result().getUploadId());
        input_1.setCopySourceRangeStart(copySourceRangeStart_);
        input_1.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto out_1 = cliV2->uploadPartCopy(input_1);
        EXPECT_EQ(out_1.isSuccess(), false);
        EXPECT_EQ(out_1.error().getStatusCode(), 404);
        EXPECT_EQ(out_1.error().getCode(), "NoSuchBucket");
        // 复制段使用不存在的对象名
        std::string dstKey_ = dstKey + "nonexistent";
        UploadPartCopyV2Input input_2(bkt_name, dstKey_, bkt_name, obj_name, i + 1, upload.result().getUploadId());
        input_2.setCopySourceRangeStart(copySourceRangeStart_);
        input_2.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto out_2 = cliV2->uploadPartCopy(input_2);
        EXPECT_EQ(out_2.isSuccess(), false);
        EXPECT_EQ(out_2.error().getStatusCode(), 404);
        EXPECT_EQ(out_2.error().getCode(), "NoSuchUpload");
        // 复制段使用不存在的UploadID
        UploadPartCopyV2Input input_3(bkt_name, dstKey, bkt_name, obj_name, i + 1, "1234");
        input_3.setCopySourceRangeStart(copySourceRangeStart_);
        input_3.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto out_3 = cliV2->uploadPartCopy(input_3);
        EXPECT_EQ(out_3.isSuccess(), false);
        EXPECT_EQ(out_3.error().getStatusCode(), 404);
        EXPECT_EQ(out_2.error().getCode(), "NoSuchUpload");
        // 复制段使用不存在的源桶
        UploadPartCopyV2Input input_4(bkt_name, dstKey, bkt_name_, obj_name, i + 1, upload.result().getUploadId());
        input_4.setCopySourceRangeStart(copySourceRangeStart_);
        input_4.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto out_4 = cliV2->uploadPartCopy(input_4);
        EXPECT_EQ(out_4.isSuccess(), false);
        EXPECT_EQ(out_4.error().getStatusCode(), 404);
        EXPECT_EQ(out_4.error().getCode(), "NoSuchBucket");
        // 复制段使用不存在的源对象名
        UploadPartCopyV2Input input_5(bkt_name, dstKey, bkt_name, obj_name_, i + 1, upload.result().getUploadId());
        input_5.setCopySourceRangeStart(copySourceRangeStart_);
        input_5.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto out_5 = cliV2->uploadPartCopy(input_5);
        EXPECT_EQ(out_5.isSuccess(), false);
        EXPECT_EQ(out_5.error().getStatusCode(), 404);
        EXPECT_EQ(out_5.error().getCode(), "NoSuchKey");
        // todo:复制段使用不存在的VersionID
        //
        UploadPartCopyV2Input input(bkt_name, dstKey, bkt_name, obj_name, i + 1, upload.result().getUploadId());
        input.setCopySourceRangeStart(copySourceRangeStart_);
        input.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto res = cliV2->uploadPartCopy(input);
        UploadedPartV2 temp(res.result().getPartNumber(), res.result().getETag());
        copyParts[i] = temp;
        copySourceRangeEnd_ = copySourceRangeEnd_ + 1;
    }

    CompleteMultipartUploadV2Input input(bkt_name, dstKey, upload.result().getUploadId(), copyParts);
    auto complete = cliV2->completeMultipartUpload(input);
    EXPECT_EQ(complete.isSuccess(), true);

    std::string content = TestUtils::GetObjectContentByStream(cliV2, bkt_name, dstKey);

    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == content);
    EXPECT_EQ(check_data, true);
}

}  // namespace VolcengineTos
