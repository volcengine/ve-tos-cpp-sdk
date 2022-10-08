#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

namespace VolcengineTos {
class ObjectUploadPartCopyClientV1Test : public ::testing::Test {
protected:
    ObjectUploadPartCopyClientV1Test() {
    }

    ~ObjectUploadPartCopyClientV1Test() override {
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
        cliV1 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::shared_ptr<TosClient> cliV1;
    static std::string bkt_name;
};

std::shared_ptr<TosClientV2> ObjectUploadPartCopyClientV1Test::cliV2 = nullptr;
std::shared_ptr<TosClient> ObjectUploadPartCopyClientV1Test::cliV1 = nullptr;
std::string ObjectUploadPartCopyClientV1Test::bkt_name = "";

TEST_F(ObjectUploadPartCopyClientV1Test, UploadPartCopyTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }

    auto srcPut = cliV1->putObject(bkt_name, obj_name, ss);
    EXPECT_EQ(srcPut.isSuccess(), true);

    std::string dstKey = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto upload = cliV1->createMultipartUpload(bkt_name, dstKey);
    EXPECT_EQ(upload.isSuccess(), true);
    // 获取要copy的对象大小
    auto head = cliV1->headObject(bkt_name, obj_name);
    EXPECT_EQ(head.isSuccess(), true);
    int64_t size = head.result().getObjectMeta().getContentLength();

    int partSize = 5 * 1024 * 1024;  // 10MB
    int partCount = (int)(size / partSize);
    std::vector<UploadPartCopyOutput> copyParts(partCount);
    int64_t copySourceRangeEnd_ = 0;
    for (int i = 0; i < partCount; ++i) {
        int64_t partLen = partSize;
        if (partCount == i + 1 && (size % partLen) > 0) {
            partLen += size % (int64_t)partSize;
        }
        int64_t startOffset = (int64_t)i * partSize;
        UploadPartCopyInput input;
        input.setUploadId(upload.result().getUploadId());
        input.setDestinationKey(dstKey);
        input.setSourceBucket(bkt_name);
        input.setStartOffset(startOffset);
        input.setPartSize(partLen);
        input.setPartNumber(i + 1);
        input.setSourceKey(obj_name);
        auto res = cliV1->uploadPartCopy(bkt_name, input);
        copyParts[i] = res.result();
    }
    CompleteMultipartCopyUploadInput input(dstKey, upload.result().getUploadId(), copyParts);
    auto complete = cliV1->completeMultipartUpload(bkt_name, input);
    EXPECT_EQ(complete.isSuccess(), true);

    std::string content = TestUtils::GetObjectContentByStream(cliV2, bkt_name, dstKey);

    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == content);
    EXPECT_EQ(check_data, true);
}

TEST_F(ObjectUploadPartCopyClientV1Test, UploadpartToNonExistentNameTest) {
    std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = bkt_name + "nonexistent";
    std::string obj_name_ = obj_name + "nonexistent";
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }

    auto srcPut = cliV1->putObject(bkt_name, obj_name, ss);
    EXPECT_EQ(srcPut.isSuccess(), true);

    std::string dstKey = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto upload = cliV1->createMultipartUpload(bkt_name, dstKey);
    EXPECT_EQ(upload.isSuccess(), true);
    // 获取要copy的对象大小
    auto head = cliV1->headObject(bkt_name, obj_name);
    EXPECT_EQ(head.isSuccess(), true);
    int64_t size = head.result().getObjectMeta().getContentLength();

    int partSize = 5 * 1024 * 1024;  // 10MB
    int partCount = (int)(size / partSize);
    std::vector<UploadPartCopyOutput> copyParts(partCount);
    int64_t copySourceRangeEnd_ = 0;
    for (int i = 0; i < partCount; ++i) {
        int64_t partLen = partSize;
        if (partCount == i + 1 && (size % partLen) > 0) {
            partLen += size % (int64_t)partSize;
        }
        int64_t startOffset = (int64_t)i * partSize;
        UploadPartCopyInput input;
        input.setUploadId(upload.result().getUploadId());
        input.setDestinationKey(dstKey);
        input.setSourceBucket(bkt_name);
        input.setStartOffset(startOffset);
        input.setPartSize(partLen);
        input.setPartNumber(i + 1);
        input.setSourceKey(obj_name);

        // 从不存在的桶复制段
        auto out_1 = cliV1->uploadPartCopy(bkt_name_, input);
        EXPECT_EQ(out_1.isSuccess(), false);
        EXPECT_EQ(out_1.error().getStatusCode(), 404);
        EXPECT_EQ(out_1.error().getCode(), "NoSuchBucket");
        // 复制段使用不存在的对象名
        std::string dstKey_ = TestUtils::GetObjectKey(TestConfig::TestPrefix);
        input.setDestinationKey(dstKey_);
        auto out_2 = cliV1->uploadPartCopy(bkt_name, input);
        EXPECT_EQ(out_2.isSuccess(), false);
        EXPECT_EQ(out_2.error().getStatusCode(), 404);
        EXPECT_EQ(out_2.error().getCode(), "NotFound");
        // 复制段使用不存在的UploadID
        input.setDestinationKey(dstKey);
        input.setUploadId("1234");
        auto out_3 = cliV1->uploadPartCopy(bkt_name, input);
        EXPECT_EQ(out_3.isSuccess(), false);
        EXPECT_EQ(out_3.error().getStatusCode(), 404);
        EXPECT_EQ(out_3.error().getCode(), "NotFound");
        // 复制段使用不存在的源桶
        input.setUploadId(upload.result().getUploadId());
        input.setSourceBucket(bkt_name_);
        auto out_4 = cliV1->uploadPartCopy(bkt_name, input);
        EXPECT_EQ(out_4.isSuccess(), false);
        EXPECT_EQ(out_4.error().getStatusCode(), 404);
        EXPECT_EQ(out_4.error().getCode(), "NoSuchBucket");
        // 复制段使用不存在的源对象名
        input.setSourceBucket(bkt_name);
        input.setSourceKey(obj_name_);
        auto out_5 = cliV1->uploadPartCopy(bkt_name, input);
        EXPECT_EQ(out_5.isSuccess(), false);
        EXPECT_EQ(out_5.error().getStatusCode(), 404);
        EXPECT_EQ(out_5.error().getCode(), "NoSuchKey");
        // todo:复制段使用不存在的VersionID
        //
        input.setSourceKey(obj_name);
        auto res = cliV1->uploadPartCopy(bkt_name, input);
        copyParts[i] = res.result();
    }
    CompleteMultipartCopyUploadInput input(dstKey, upload.result().getUploadId(), copyParts);
    auto complete = cliV1->completeMultipartUpload(bkt_name, input);
    EXPECT_EQ(complete.isSuccess(), true);

    std::string content = TestUtils::GetObjectContentByStream(cliV2, bkt_name, dstKey);

    std::string data = std::string((11 << 20), '1');
    bool check_data = (data == content);
    EXPECT_EQ(check_data, true);
}

}  // namespace VolcengineTos
