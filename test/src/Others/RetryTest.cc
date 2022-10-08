#include "../TestConfig.h"
#include "../Utils.h"
#include "TosClientV2.h"
#include <gtest/gtest.h>
#include "spdlog/spdlog.h"
#include "utils/LogUtils.h"

namespace VolcengineTos {
class RetryTest : public ::testing::Test {
protected:
    RetryTest() {
    }

    ~RetryTest() override {
    }

    static void SetUpTestCase() {
    }

    // Tears down the stuff shared by all tests in this test case.
    static void TearDownTestCase() {
    }
};

static std::set<std::string> CanRetryMethods = {"createBucket",
                                                "deleteBucket",
                                                "createMultipartUpload",
                                                "completeMultipartUpload",
                                                "abortMultipartUpload",
                                                "setObjectMeta",
                                                "putObjectAcl",
                                                "deleteObject",
                                                "putObject",
                                                "uploadPart"};
static bool findInCanRetryMethods(std::string method) {
    auto pos = CanRetryMethods.find(method);
    if (pos != CanRetryMethods.end()) {
        return true;
    }
    return false;
}

static int64_t calContentLength(const std::shared_ptr<std::iostream>& content) {
    int64_t currentPos = content->tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        content->clear();
    }
    content->seekg(0, content->end);
    int64_t size = content->tellg();
    content->seekg(currentPos, content->beg);
    return size;
}

static bool checkShouldRetry(const std::shared_ptr<TosRequest>& request, const std::shared_ptr<TosResponse>& response) {
    auto resCode = response->getStatusCode();
    bool timeout = (response->getStatusMsg() == "operation timeout");
    if (resCode == 429 || resCode >= 500 || timeout) {
        if (request->getMethod() == http::MethodGet || request->getMethod() == http::MethodHead) {
            if (request->getMethod() == http::MethodGet) {
                auto content_ = response->getContent();
                int64_t contentLength_ = calContentLength(content_);
                if (contentLength_ != 0) {
                    return false;
                }
            }
            return true;
        }
        if ((resCode == 429 || resCode >= 500) && findInCanRetryMethods(request->getFuncName())) {
            if (request->getFuncName() == "putObject" || request->getFuncName() == "uploadPart") {
                auto content = response->getContent();
                int64_t offset = request->getContentOffset();
                content->seekg(offset, content->beg);
            }
            return true;
        }
    }
    return false;
}

TEST_F(RetryTest, RetryGetMethodTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "";
    TosRequest req;
    req.setMethod(http::MethodGet);
    req.setFuncName("getObject");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
}
TEST_F(RetryTest, RetryGetMethodWithContentTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodGet);
    req.setFuncName("getObject");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, false);
}
TEST_F(RetryTest, RetryHeadMethodTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodHead);
    req.setFuncName("headObject");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}
TEST_F(RetryTest, RetryCreateBucketTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodPut);
    req.setFuncName("createBucket");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}
TEST_F(RetryTest, RetryDeleteBucketTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodDelete);
    req.setFuncName("deleteBucket");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}

TEST_F(RetryTest, RetrySetObjectMetaTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodPost);
    req.setFuncName("setObjectMeta");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}

TEST_F(RetryTest, RetryPutObjectAclTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodPut);
    req.setFuncName("putObjectAcl");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}

TEST_F(RetryTest, RetryDeleteObjectTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    TosRequest req;
    req.setMethod(http::MethodDelete);
    req.setFuncName("deleteObject");
    TosResponse res(ss);
    res.setStatusCode(503);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}

TEST_F(RetryTest, RetryPutObjectTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    ss->seekg(1, ss->beg);
    EXPECT_EQ(ss->tellg() == 1, true);
    TosRequest req;
    req.setMethod(http::MethodPut);
    req.setFuncName("putObject");
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->str() == "123", true);
}

TEST_F(RetryTest, RetryUploadPartTest) {
    auto ss = std::make_shared<std::stringstream>();
    *ss << "123";
    ss->seekg(2, ss->beg);
    EXPECT_EQ(ss->tellg() == 2, true);
    TosRequest req;
    req.setMethod(http::MethodPut);
    req.setFuncName("uploadPart");
    req.setContentOffset(1);
    TosResponse res(ss);
    res.setStatusCode(429);
    auto request = std::make_shared<TosRequest>(req);
    auto response = std::make_shared<TosResponse>(res);
    bool output = checkShouldRetry(request, response);
    EXPECT_EQ(output, true);
    EXPECT_EQ(ss->tellg() == 1, true);
}

}  // namespace VolcengineTos
