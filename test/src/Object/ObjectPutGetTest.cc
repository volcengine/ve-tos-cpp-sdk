#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>

//#include <dirent.h>
namespace VolcengineTos {
class ObjectPutGetTest : public ::testing::Test {
protected:
    ObjectPutGetTest() {
    }

    ~ObjectPutGetTest() override {
    }

    static void SetUpTestCase() {
        ClientConfig conf;
        conf.endPoint = TestConfig::HTTPsEndpoint;
        conf.enableVerifySSL = false;
        conf.maxRetryCount = 0;
        conf.userAgentProductName = "tos";
        conf.userAgentSoftName="cxxSdk";
        conf.userAgentSoftVersion = "2.6.15";
        conf.userAgentCustomizedKeyValues = {{"key1", "value1"}, {"key2", "value2"}};
        cliV2 = std::make_shared<TosClientV2>(TestConfig::Region, TestConfig::Ak, TestConfig::Sk, conf);
        bkt_name = TestUtils::GetBucketName(TestConfig::TestPrefix);
        bkt_name_hns = TestUtils::GetBucketName(TestConfig::TestPrefixHns);

        workPath = FileUtils::getWorkPath();
        std::cout << workPath << std::endl;
        TestUtils::CreateBucket(cliV2, bkt_name);
        TestUtils::CreateBucket(cliV2, bkt_name_hns, true);
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
        TestUtils::CleanBucket(cliV2, bkt_name);
        cliV2 = nullptr;
    }

public:
    static std::shared_ptr<TosClientV2> cliV2;
    static std::string bkt_name;
    static std::string bkt_name_hns;
    static std::string workPath;
};

std::shared_ptr<TosClientV2> ObjectPutGetTest::cliV2 = nullptr;
std::string ObjectPutGetTest::bkt_name = "";
std::string ObjectPutGetTest::bkt_name_hns = "";
std::string ObjectPutGetTest::workPath = "";

TEST_F(ObjectPutGetTest, PutObjectWithoutParametersTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto output_obj_put = cliV2->putObject(input_obj_put);

    std::string data2 =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss2 = std::make_shared<std::stringstream>(data2);
    input_obj_put.setContent(ss2);
    auto output_obj_put2 = cliV2->putObject(input_obj_put);

    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = res_obj_get.result().getGetObjectBasicOutput();
    auto content_output = res_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();

    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    bool length_compare = (tmp_string.size() == data.size());
    bool content_length_compare = (basic_output.getContentLength() == data.length());
    bool content_compare = (data == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}
TEST_F(ObjectPutGetTest, PutGetFileStatusTestDirectory) {
    std::string obj_key = "cxx/sdk/test"+TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "123";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name_hns);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto headObjectInput = HeadObjectV2Input(bkt_name_hns, obj_key);
    auto headObjectOutput = cliV2->headObject(headObjectInput);
    EXPECT_EQ(headObjectOutput.isSuccess(), true);
    EXPECT_EQ(headObjectOutput.result().isDirectory(), false);

    headObjectInput = HeadObjectV2Input(bkt_name_hns, "cxx/sdk/");
    headObjectOutput = cliV2->headObject(headObjectInput);
    EXPECT_EQ(headObjectOutput.isSuccess(), true);
    EXPECT_EQ(headObjectOutput.result().isDirectory(), true);

    auto getObjectInput = GetObjectV2Input(bkt_name_hns, obj_key);
    auto getObjectOutput = cliV2->getObject(getObjectInput);
    EXPECT_EQ(getObjectOutput.isSuccess(), true);
    EXPECT_EQ(getObjectOutput.result().isDirectory(), false);

    getObjectInput = GetObjectV2Input(bkt_name_hns, "cxx/sdk/");
    getObjectOutput = cliV2->getObject(getObjectInput);
    EXPECT_EQ(getObjectOutput.isSuccess(), true);
    EXPECT_EQ(getObjectOutput.result().isDirectory(), true);
}
TEST_F(ObjectPutGetTest, PutGetFileStatusTestHNS) {
    std::string obj_key = "cxx/sdk/test"+TestUtils::GetObjectKey(TestConfig::TestPrefixHns);
    std::string data = "123";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name_hns);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto getFileStatusInput = GetFileStatusInput(bkt_name_hns, obj_key);
    auto getFileStatusOutput = cliV2->getFileStatus(getFileStatusInput);
    EXPECT_EQ(getFileStatusOutput.isSuccess(), true);

    getFileStatusInput = GetFileStatusInput(bkt_name_hns, obj_key);
    getFileStatusOutput = cliV2->getFileStatus(getFileStatusInput);
    EXPECT_EQ(getFileStatusOutput.isSuccess(), true);
}
TEST_F(ObjectPutGetTest, PutGetFileStatusTestFNS) {
    std::string obj_key = "cxx/sdk/test"+TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "123";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    auto getFileStatusInput = GetFileStatusInput(bkt_name, obj_key);
    auto getFileStatusOutput = cliV2->getFileStatus(getFileStatusInput);
    EXPECT_EQ(getFileStatusOutput.isSuccess(), true);

    obj_key = "cxx/sdk/";
    getFileStatusInput = GetFileStatusInput(bkt_name, obj_key);
    getFileStatusOutput = cliV2->getFileStatus(getFileStatusInput);
    EXPECT_EQ(getFileStatusOutput.isSuccess(), true);
    EXPECT_NE(getFileStatusOutput.result().getKey(),obj_key);
}
TEST_F(ObjectPutGetTest, PutZeroSizeObjectTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}
TEST_F(ObjectPutGetTest, PutObjectWithErrorNameTest) {
    std::string error_message = "invalid object name, the length must be [1, 696]";
    std::string obj_key = "";
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == error_message, true);
}
TEST_F(ObjectPutGetTest, PutObjectWithErrorMd5Test) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    std::string data2 = "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcde";
    std::string dataMd5 = CryptoUtils::md5Sum(data2);

    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);

    input_obj_put_basic.setContentMd5(dataMd5);

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The Content-MD5 you specified did not match what we received.",
              true);
}

TEST_F(ObjectPutGetTest, PutObjectWithSSECTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";

    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_key);
    input_obj_put.setContent(ss);
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();

    // 设置全部参数
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    input_obj_put_basic.setContentMd5(dataMd5);
    input_obj_put_basic.setCacheControl("no-cache");
    std::time_t expires = TestUtils::GetTimeWithDelay(10);
    input_obj_put_basic.setExpires(expires);
    input_obj_put_basic.setContentDisposition("attachment;filename=\" 中文.txt\"");
    input_obj_put_basic.setContentEncoding("gzip");
    input_obj_put_basic.setContentLanguage("en-US");
    //    input_obj_put_basic.setAcl(ACLType::Private);
    input_obj_put_basic.setGrantFullControl("id=123");
    input_obj_put_basic.setGrantRead("id=123");
    input_obj_put_basic.setGrantReadAcp("id=123");
    input_obj_put_basic.setGrantWriteAcp("id=123");

    std::map<std::string, std::string> meta_{};
#ifdef _WIN32
    meta_[u8"中文key1中文"] = "";
    meta_[u8"中文key2中文"] = u8"中文value2中文";
#else
    meta_["中文key1中文"] = "";
    meta_["中文key2中文"] = "中文value2中文";
#endif

    input_obj_put_basic.setMeta(meta_);

    input_obj_put_basic.setSsecAlgorithm("AES256");
    std::string ssecKey = "hoxnu1jii3u4h1h7cezrst3hpd8xv465";
    std::string ssecKeyMd5 = CryptoUtils::md5Sum(ssecKey);
    input_obj_put_basic.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
    input_obj_put_basic.setSsecKeyMd5(ssecKeyMd5);

    // todo:time_out
    //    input_obj_put_basic.setServerSideEncryption("AES256");

    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get(bkt_name, obj_key);
    input_obj_get.setSsecKeyMd5(ssecKeyMd5);
    input_obj_get.setSsecAlgorithm("AES256");
    input_obj_get.setSsecKey("aG94bnUxamlpM3U0aDFoN2NlenJzdDNocGQ4eHY0NjU=");
    auto output_obj_get = cliV2->getObject(input_obj_get);
    std::ostringstream ss_;
    ss_ << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss_.str();

    bool check_data = (data == tmp_string);
    auto meta = output_obj_get.result().getGetObjectBasicOutput().getMeta();

#ifdef _WIN32
    bool check_meta = ((meta[u8"中文key1中文"] == "") && (meta[u8"中文key2中文"] == u8"中文value2中文"));
#else
    bool check_meta = ((meta["中文key1中文"] == "") && (meta["中文key2中文"] == "中文value2中文"));
#endif
    EXPECT_EQ(check_data & check_meta, true);
    EXPECT_EQ(output_obj_get.isSuccess(), true);
}

TEST_F(ObjectPutGetTest, PutObjectToNoExistingBucketTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name_);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getStatusCode(), 404);
    EXPECT_EQ(output_obj_put.error().getMessage() == "The specified bucket does not exist.", true);
    EXPECT_EQ(output_obj_put.error().getEc(), "0006-00000001");
    auto endPoint = TestConfig::HTTPsEndpoint;
    std::string host, schme;
    if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTPS)) {
        host = endPoint.substr(std::strlen(http::SchemeHTTPS) + 3,
                               endPoint.length() - std::strlen(http::SchemeHTTPS) - 3);
        schme = "https://";
    } else if (StringUtils::startsWithIgnoreCase(endPoint, http::SchemeHTTP)) {
        host = endPoint.substr(std::strlen(http::SchemeHTTP) + 3,
                               endPoint.length() - std::strlen(http::SchemeHTTP) - 3);
        schme = "http://";
    } else {
        host = endPoint;
        schme = "https://";
    }
    EXPECT_EQ(output_obj_put.error().getRequestUrl(), schme + bkt_name_ + "." + host + "/" + obj_key);
}

TEST_F(ObjectPutGetTest, GetObjectWithNoExistingNameTest) {
    std::string obj_key = "111";
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get.isSuccess(), false);
    EXPECT_EQ(res_obj_get.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get.error().getCode() == "NoSuchKey", true);
    EXPECT_EQ(res_obj_get.error().getMessage() == "The specified key does not exist.", true);

    std::string bkt_name_ = TestUtils::GetBucketName(TestConfig::TestPrefix);
    input_obj_get.setBucket(bkt_name_);
    auto res_obj_get_ = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get_.isSuccess(), false);
    EXPECT_EQ(res_obj_get_.error().getStatusCode() == 404, true);
    EXPECT_EQ(res_obj_get_.error().getMessage() == "The specified bucket does not exist.", true);
}

TEST_F(ObjectPutGetTest, GetObjectWithRangeTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    int64_t begin = 10;
    int64_t end = 119;
    int length_range = end - begin + 1;

    input_obj_get.setRangeStart(begin);
    input_obj_get.setRangeEnd(end);
    auto res_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = res_obj_get.result().getGetObjectBasicOutput();
    auto content_output = res_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();
    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    bool length_compare = (tmp_string.size() == length_range);
    bool content_length_compare = (data.compare(begin, length_range, tmp_string) == 0);
    bool content_compare = (data.substr(begin, length_range) == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}

TEST_F(ObjectPutGetTest, GetObjectWithRange2Test) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
            "()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);
    int64_t begin = 10;
    int64_t end = 119;
    int length_range = end - begin + 1;

    input_obj_get.setRange("bytes=10-119");
    auto res_obj_get = cliV2->getObject(input_obj_get);
    auto basic_output = res_obj_get.result().getGetObjectBasicOutput();
    auto content_output = res_obj_get.result().getContent();
    std::string ss_;

    auto stream = content_output.get();
    char streamBuffer[256];
    memset(streamBuffer, 0, 256);
    while (stream->good()) {
        stream->read(streamBuffer, 256);
        auto bytesRead = stream->gcount();
    }
    std::string tmp_string(streamBuffer);
    bool length_compare = (tmp_string.size() == length_range);
    bool content_length_compare = (data.compare(begin, length_range, tmp_string) == 0);
    bool content_compare = (data.substr(begin, length_range) == tmp_string);
    EXPECT_EQ(content_length_compare & length_compare, true);
    EXPECT_EQ(content_compare, true);
}

TEST_F(ObjectPutGetTest, PutGetObjectFromFileTest) {
    auto workPath = FileUtils::getWorkPath();
    std::cout << workPath << std::endl;
    std::string filePath1 = workPath + "test" + TOS_PATH_DELIMITER +"testdata" + TOS_PATH_DELIMITER + "PutObjectTest.txt";
    std::string filePath2 = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "GetObjectTest.txt";

    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectFromFileInput input_obj_put(bkt_name, obj_key, filePath1);
    auto output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectToFileInput input_obj_get(bkt_name, obj_key, filePath2);
    auto out_obj_get = cliV2->getObjectToFile(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
}

TEST_F(ObjectPutGetTest, PutGetObjectFromEmptyFileTest) {
    auto workPath = FileUtils::getWorkPath();
    std::cout << workPath << std::endl;
    std::string filePath1 = workPath + "test" + TOS_PATH_DELIMITER +"testdata" + TOS_PATH_DELIMITER +"uploadFile2";

    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectFromFileInput input_obj_put(bkt_name, obj_key, filePath1);
    auto output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    GetObjectV2Input input_obj_get(bkt_name, obj_key);
    auto out_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);
}

TEST_F(ObjectPutGetTest, PutGetObjectFromFileWithTrafficLimitTest) {
    auto workPath = FileUtils::getWorkPath();
    std::cout << workPath << std::endl;
    std::string filePath1 = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "uploadFile1";
    std::string filePath2 =
            workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "GetObjectTest.txt";

    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    PutObjectFromFileInput input_obj_put(bkt_name, obj_key, filePath1);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endTime - startTime;
    auto time1 = fp_ms.count() / 1000;

    startTime = std::chrono::high_resolution_clock::now();
    input_obj_put.setTrafficLimit(1024 * 1024);
    output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time2 = fp_ms.count() / 1000;
    bool isInTime = time2 > time1;
    EXPECT_EQ(isInTime, true);

    GetObjectToFileInput input_obj_get(bkt_name, obj_key, filePath2);
    startTime = std::chrono::high_resolution_clock::now();
    auto out_obj_get = cliV2->getObjectToFile(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);

    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time3 = fp_ms.count() / 1000;

    input_obj_get.setTrafficLimit(1024 * 1024);
    startTime = std::chrono::high_resolution_clock::now();
    out_obj_get = cliV2->getObjectToFile(input_obj_get);
    EXPECT_EQ(out_obj_get.isSuccess(), true);

    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time4 = fp_ms.count() / 1000;

    bool isInTime2 = time4 > time3;
    EXPECT_EQ(isInTime2, true);
}

TEST_F(ObjectPutGetTest, PutObjectWithTrafficLimitTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);

    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (13 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    auto startTime = std::chrono::high_resolution_clock::now();
    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = endTime - startTime;
    auto time1 = fp_ms.count() / 1000;

    ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (13 << 20); ++i) {
        *ss << 1;
    }
    input_obj_put.setContent(ss);
    input_obj_put.setTrafficLimit(1024 * 1024);
    startTime = std::chrono::high_resolution_clock::now();
    output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time2 = fp_ms.count() / 1000;
    bool isInTime = time2 > time1;
    EXPECT_EQ(isInTime, true);

    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_key);

    startTime = std::chrono::high_resolution_clock::now();
    auto res_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time3 = fp_ms.count() / 1000;

    input_obj_get.setTrafficLimit(1024 * 1024);

    startTime = std::chrono::high_resolution_clock::now();
    res_obj_get = cliV2->getObject(input_obj_get);
    EXPECT_EQ(res_obj_get.isSuccess(), true);
    endTime = std::chrono::high_resolution_clock::now();
    fp_ms = endTime - startTime;
    auto time4 = fp_ms.count() / 1000;
    isInTime = time4 > time3;
    EXPECT_EQ(isInTime, true);
}

TEST_F(ObjectPutGetTest, PutObjectWithDataProcessTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    auto workPath = FileUtils::getWorkPath();
    std::cout << workPath << std::endl;
    std::string filePath1 = workPath + "test" + TOS_PATH_DELIMITER + "testdata" + TOS_PATH_DELIMITER + "example.jpg";
    PutObjectFromFileInput input_obj_put(bkt_name, obj_key, filePath1);
    auto output_obj_put = cliV2->putObjectFromFile(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);

    GetObjectV2Input input(bkt_name, obj_key);
    input.setProcess("image/info");
    auto res = cliV2->getObject(input);
    EXPECT_EQ(res.isSuccess(), true);
    std::ostringstream ss;
    ss << res.result().getContent()->rdbuf();
    std::string tmp_string = ss.str();
    EXPECT_EQ(
            tmp_string ==
                    "{\"FileSize\":{\"value\":\"21839\"},\"Format\":{\"value\":\"jpeg\"},\"ImageHeight\":{\"value\":\"267\"},\"ImageWidth\":{\"value\":\"400\"}}",
            true);
}

TEST_F(ObjectPutGetTest, PutZeroSizeObjectWithCustomReqTimeTest) {
    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    std::string data = "";
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put;
    auto input_obj_put_basic = input_obj_put.getPutObjectBasicInput();
    input_obj_put.setContent(ss);
    input_obj_put_basic.setBucket(bkt_name);
    input_obj_put_basic.setKey(obj_key);
    input_obj_put.setPutObjectBasicInput(input_obj_put_basic);

    std::time_t now = std::time(nullptr); // 当前时间
    std::time_t oneHourLater = now + 3600; // 直接加3600秒（1小时=60 * 60秒）
    input_obj_put.setRequestDate(oneHourLater);

    auto output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), false);
    EXPECT_EQ(output_obj_put.error().getEc(), "0002-00000018");

    auto headers = output_obj_put.result().getRequestInfo().getHeaders();
    std::tm tm = {};
    std::istringstream iss(headers["Date"]);

    // 解析格式：Sun, 08 Jun 2025 08:31:27 GMT
    iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    EXPECT_EQ(iss.fail(), false);
    auto server_date = timegm(&tm);
    EXPECT_EQ(server_date > 0, true);

    input_obj_put.setRequestDate(server_date);
    output_obj_put = cliV2->putObject(input_obj_put);
    EXPECT_EQ(output_obj_put.isSuccess(), true);
}

TEST_F(ObjectPutGetTest, GetObjectStramTest) {
    //    std::string obj_key = TestUtils::GetObjectKey(TestConfig::TestPrefix);
    //
    //    std::string data =
    //            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./
    //            :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
    //            "()_+<>?,./   :'";
    //    auto ss = std::make_shared<std::stringstream>(data);
    //    PutObjectV2Input input_obj_put(bkt_name, obj_key, ss);
    //    auto output_obj_put = cliV2->putObject(input_obj_put);
    //
    //    std::string data2 =
    //            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./
    //            :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*"
    //            "()_+<>?,./   :'";
    //    auto ss2 = std::make_shared<std::stringstream>(data2);
    //    input_obj_put.setContent(ss2);
    //    auto output_obj_put2 = cliV2->putObject(input_obj_put);
    //
    //    EXPECT_EQ(output_obj_put.isSuccess(), true);
    //
    //    GetObjectV2Input input_obj_get;
    //    input_obj_get.setBucket(bkt_name);
    //    input_obj_get.setKey(obj_key);
    //    std::shared_ptr<std::iostream> resContent;
    //    auto res_obj_get = cliV2->getObject(input_obj_get, resContent, nullptr);
    //    assert(res_obj_get.isSuccess());
}
// TEST_F(ObjectPutGetTest, AppendGetObjectWithPartNumberTest) {
//     std::string obj_name = TestUtils::GetObjectKey(TestConfig::TestPrefix);
//     CreateMultipartUploadInput input_part_create(bkt_name, obj_name);
//
//     auto upload = cliV2->createMultipartUpload(input_part_create);
//
//     // generate some data..
//     auto ss1 = std::make_shared<std::stringstream>();
//     for (int i = 0; i < (5 << 20); ++i) {
//         *ss1 << 1;
//     }
//     UploadPartV2Input input_upload_part(bkt_name, obj_name, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
//     auto part1 = cliV2->uploadPart(input_upload_part);
//     EXPECT_EQ(part1.isSuccess(), true);
//
//     // generate some data..
//     auto ss2 = std::make_shared<std::stringstream>();
//     for (int i = 0; i < (4 << 20); ++i) {
//         *ss2 << 2;
//     }
//     UploadPartV2Input input_upload_part_2(bkt_name, obj_name, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
//     auto part2 = cliV2->uploadPart(input_upload_part_2);
//     EXPECT_EQ(part2.isSuccess(), true);
//     auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
//     auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
//     std::vector<UploadedPartV2> vec = {part_1, part_2};
//     CompleteMultipartUploadV2Input input_upload_complete(bkt_name, obj_name, upload.result().getUploadId(), vec);
//     auto com = cliV2->completeMultipartUpload(input_upload_complete);
//     EXPECT_EQ(com.isSuccess(), true);
//
//     int data_size_1 = (4 << 20);
//     int data_size_2 = (5 << 20);
//     std::string data_1 = std::string(data_size_1, '2');
//     std::string data_2 = std::string(data_size_2, '1');
//
//     GetObjectV2Input input_obj_get(bkt_name, obj_name);
//     input_obj_get.setPartNumber(2);
//     auto out_obj_get = cliV2->getObject(input_obj_get);
//     std::ostringstream ssOutput1;
//     ssOutput1 << out_obj_get.result().getContent()->rdbuf();
//     std::string tmpString1 = ssOutput1.str();
//     bool lengthCompare = (data_size_1 == tmpString1.size());
//     bool contentCompare = (data_1 == tmpString1);
//     EXPECT_EQ(lengthCompare, true);
//     EXPECT_EQ(contentCompare, true);
//
//     input_obj_get.setPartNumber(1);
//     auto getObjectOutput = cliV2->getObject(input_obj_get);
//     auto getObjectBasicOutput = getObjectOutput.result().getGetObjectBasicOutput();
//     auto contentOutput = getObjectOutput.result().getContent();
//
//     std::ostringstream ssOutput2;
//     ssOutput2 << getObjectOutput.result().getContent()->rdbuf();
//     std::string tmpString2 = ssOutput2.str();
//
//     bool lengthCompare2 = (data_size_2 == tmpString2.size());
//     bool contentCompare2 = (data_2 == tmpString2);
//     EXPECT_EQ(lengthCompare2, true);
//     EXPECT_EQ(contentCompare2, true);
// }

}  // namespace VolcengineTos
