#include <gtest/gtest.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <condition_variable>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <process.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "TosClientImpl.h"
#include "TosClientV2.h"
#include "external/json/json.hpp"

namespace VolcengineTos {
class TosClientImplTestPeer {
public:
    static void setTransport(TosClientImpl& client, const std::shared_ptr<Transport>& transport) {
        client.transport_ = transport;
    }
    static void allowLocalTestPort(TosClientImpl& client) {
        client.connectWithIP_ = false;
    }
    static Outcome<TosError, std::shared_ptr<TosResponse>> request(TosClientImpl& client,
                                                                   const std::shared_ptr<TosRequest>& request,
                                                                   bool multipleCodes) {
        return multipleCodes ? client.roundTrip(request, std::vector<int>{200, 206}) : client.roundTrip(request, 200);
    }
};
}  // namespace VolcengineTos

using namespace VolcengineTos;

namespace {
constexpr int64_t kPartSize = 5 * 1024 * 1024;

class ScriptedTransport : public Transport {
public:
    std::function<std::shared_ptr<TosResponse>(const std::shared_ptr<TosRequest>&)> handler;
    std::shared_ptr<TosResponse> roundTrip(const std::shared_ptr<TosRequest>& request) override {
        return handler(request);
    }
};

std::shared_ptr<TosResponse> response(int status, const std::string& body = "{}", int curlCode = 0) {
    auto res = std::make_shared<TosResponse>(std::make_shared<std::stringstream>(body));
    res->setStatusCode(status);
    res->setCurlErrCode(curlCode);
    res->setContentLength(6);
    res->setHeaders({{"x-tos-request-id", "request-id"},
                     {"x-tos-id-2", "id-2"},
                     {"x-tos-ec", "header-ec"},
                     {"Content-Length", "6"},
                     {"ETag", "etag"}});
    if (curlCode != 0) {
        res->setStatusMsg("curlCode: " + std::to_string(curlCode) + ", simulated transport failure");
    }
    return res;
}

std::shared_ptr<TosResponse> serverError(int status = 403, const std::string& code = "AccessDenied") {
    return response(status, "{\"Code\":\"" + code +
                                    "\",\"Message\":\"original service message\",\"RequestId\":\"request-id\","
                                    "\"HostId\":\"host-id\",\"EC\":\"service-ec\"}");
}

std::string operation(const std::shared_ptr<TosRequest>& req) {
    if (!req->getFuncName().empty()) {
        return req->getFuncName();
    }
    const auto& queries = req->getQueries();
    if (queries.count("uploads")) {
        return "createMultipartUpload";
    }
    if (queries.count("partNumber")) {
        return MapUtils::findValueByKeyIgnoreCase(req->getHeaders(), HEADER_COPY_SOURCE).empty() ? "uploadPart"
                                                                                                 : "uploadPartCopy";
    }
    if (queries.count("uploadId")) {
        return req->getMethod() == http::MethodDelete ? "abortMultipartUpload" : "completeMultipartUpload";
    }
    return req->getMethod() == http::MethodGet ? "getObject" : "headObject";
}

void expectSameError(const TosError& expected, const TosError& actual, bool sameMessage = true) {
    EXPECT_EQ(expected.getCurlErrCode(), actual.getCurlErrCode());
    EXPECT_EQ(expected.getStatusCode(), actual.getStatusCode());
    EXPECT_EQ(expected.getCode(), actual.getCode());
    if (sameMessage) {
        EXPECT_EQ(expected.getMessage(), actual.getMessage());
    }
    EXPECT_EQ(expected.isClientError(), actual.isClientError());
    EXPECT_EQ(expected.IsTosHandlerError(), actual.IsTosHandlerError());
    EXPECT_EQ(expected.getRequestId(), actual.getRequestId());
    EXPECT_EQ(expected.getHostId(), actual.getHostId());
    EXPECT_EQ(expected.getRequestUrl(), actual.getRequestUrl());
    EXPECT_EQ(expected.getEc(), actual.getEc());
    EXPECT_EQ(expected.getResource(), actual.getResource());
    EXPECT_EQ(expected.getCondition(), actual.getCondition());
    EXPECT_EQ(expected.getRequestInfo().getRequestId(), actual.getRequestInfo().getRequestId());
    EXPECT_EQ(expected.getRequestInfo().getId2(), actual.getRequestInfo().getId2());
    EXPECT_EQ(expected.getRequestInfo().getStatusCode(), actual.getRequestInfo().getStatusCode());
    EXPECT_EQ(expected.getRequestInfo().getHeaders(), actual.getRequestInfo().getHeaders());
}

class SyncErrorPropagationTest : public ::testing::Test {
protected:
    std::shared_ptr<ScriptedTransport> transport = std::make_shared<ScriptedTransport>();
    std::unique_ptr<TosClientImpl> client;
    std::string path;
    std::string failingMethod;
    int failureStatus = 403;
    int failureCurlCode = 0;
    std::string failureCode = "AccessDenied";
    std::atomic<int> abortCalls{0};
    std::atomic<int> partCalls{0};
    bool failCleanup = false;
    std::shared_ptr<CancelHook> cancelOnPart;

    static void SetUpTestCase() {
        InitializeClient();
    }
    static void TearDownTestCase() {
        CloseClient();
    }

    void SetUp() override {
#ifdef _WIN32
        const auto pid = _getpid();
        const char* temp = std::getenv("TEMP");
        path = std::string(temp ? temp : ".") + "/tos-error-propagation-" + std::to_string(pid);
#else
        path = "/tmp/tos-error-propagation-" + std::to_string(getpid());
#endif
        {
            std::ofstream file(path, std::ios::binary);
            file << "abcdef";
        }
        ClientConfig config;
        config.maxRetryCount = 0;
        config.enableCRC = false;
        client.reset(
                new TosClientImpl("https://unit-test.invalid", "cn-beijing", StaticCredentials("ak", "sk"), config));
        TosClientImplTestPeer::setTransport(*client, transport);
        transport->handler = [this](const std::shared_ptr<TosRequest>& req) {
            const auto method = operation(req);
            if (method == "abortMultipartUpload") {
                ++abortCalls;
                return failCleanup ? serverError(500, "CleanupFailed") : response(204);
            }
            if (method == "uploadPart" || method == "uploadPartCopy" || method == "getObject") {
                ++partCalls;
                if (cancelOnPart) {
                    cancelOnPart->Cancel(false);
                }
            }
            if (!failingMethod.empty() && method == failingMethod) {
                return failureCurlCode != 0 ? response(failureStatus, "", failureCurlCode)
                                            : serverError(failureStatus, failureCode);
            }
            if (method == "createMultipartUpload") {
                return response(200, "{\"UploadId\":\"upload-id\",\"Bucket\":\"test-bucket\",\"Key\":\"key\"}");
            }
            if (method == "getObject") {
                const bool ranged = !MapUtils::findValueByKeyIgnoreCase(req->getHeaders(), "Range").empty();
                return response(ranged ? 206 : 200, "abcdef");
            }
            return response(200, "{\"ETag\":\"etag\",\"Key\":\"key\",\"Bucket\":\"test-bucket\"}");
        };
    }

    void TearDown() override {
        std::remove(path.c_str());
        std::remove((path + ".download").c_str());
        std::remove((path + ".download.temp").c_str());
        std::remove((path + ".upload").c_str());
    }

    UploadFileV2Input uploadInput() {
        UploadFileV2Input input("test-bucket", "key");
        input.setFilePath(path);
        input.setTaskNum(1);
        input.setPartSize(kPartSize);
        input.setEnableCheckpoint(false);
        return input;
    }

    UploadFileInput legacyInput() {
        UploadFileInput input;
        input.setObjectKey("key");
        input.setUploadFilePath(path);
        input.setPartSize(kPartSize);
        input.setTaskNum(1);
        input.setEnableCheckpoint(false);
        return input;
    }

    ResumableCopyObjectInput copyInput() {
        ResumableCopyObjectInput input;
        input.setBucket("test-bucket");
        input.setKey("key");
        input.setSrcBucket("source-bucket");
        input.setSrcKey("source-key");
        input.setPartSize(kPartSize);
        input.setTaskNum(1);
        input.setEnableCheckpoint(false);
        return input;
    }

    DownloadFileInput downloadInput() {
        DownloadFileInput input("test-bucket", "key");
        input.setFilePath(path + ".download");
        input.setPartSize(kPartSize);
        input.setTaskNum(1);
        input.setEnableCheckpoint(false);
        return input;
    }

    TosError expectedError(const std::string& method, const std::string& requestPath) {
        auto req = std::make_shared<TosRequest>();
        req->setFuncName(method);
        req->setMethod(http::MethodPost);
        req->setScheme("https");
        req->setHost("test-bucket.unit-test.invalid");
        req->setPath("/key");
        if (requestPath == "create") {
            req->setSingleQuery("uploads", "");
        } else if (requestPath == "part") {
            req->setSingleQuery("partNumber", "1");
            req->setSingleQuery("uploadId", "upload-id");
        } else if (requestPath == "complete") {
            req->setSingleQuery("uploadId", "upload-id");
        }
        return TosClientImplTestPeer::request(*client, req, false).error();
    }

#if GTEST_HAS_DEATH_TEST && !defined(_WIN32)
    void disableCoreDumps() {
        const rlimit limit = {0, 0};
        setrlimit(RLIMIT_CORE, &limit);
    }

    int workerExceptionResult(const std::string& transfer, bool unknown) {
        disableCoreDumps();
        const auto fallback = transport->handler;
        std::atomic<int> completes{0};
        transport->handler = [&](const std::shared_ptr<TosRequest>& req) {
            const auto method = operation(req);
            if (method == "completeMultipartUpload") {
                ++completes;
            }
            if (method == (transfer == "legacy" ? "uploadPart" : transfer)) {
                if (unknown) {
                    throw 42;
                }
                throw std::runtime_error("injected worker failure");
            }
            return fallback(req);
        };
        TosError error;
        bool success;
        if (transfer == "legacy") {
            const auto result = client->uploadFile("test-bucket", legacyInput(), RequestOptionBuilder());
            success = result.isSuccess();
            error = result.error();
        } else if (transfer == "uploadPart") {
            const auto result = client->uploadFile(uploadInput());
            success = result.isSuccess();
            error = result.error();
        } else if (transfer == "uploadPartCopy") {
            const auto result = client->resumableCopyObject(copyInput());
            success = result.isSuccess();
            error = result.error();
        } else {
            const auto result = client->downloadFile(downloadInput());
            success = result.isSuccess();
            error = result.error();
        }
        const bool ok =
                !success && error.isClientError() && completes == 0 && abortCalls == 0 &&
                error.getMessage().find(unknown ? "unknown exception" : "injected worker failure") != std::string::npos;
        TearDown();
        return ok ? 0 : 1;
    }

    int callbackExceptionResult(bool copy, bool unknown, int status) {
        disableCoreDumps();
        if (status != 0) {
            failingMethod = copy ? "uploadPartCopy" : "uploadPart";
            failureStatus = status;
            failureCode = status == 403 ? "AccessDenied" : "InternalError";
        }
        std::atomic<int> completes{0};
        const auto fallback = transport->handler;
        transport->handler = [&](const std::shared_ptr<TosRequest>& req) {
            if (operation(req) == "completeMultipartUpload") {
                ++completes;
            }
            return fallback(req);
        };
        const auto throwException = [unknown]() {
            if (unknown) {
                throw 42;
            }
            throw std::runtime_error("injected event callback failure");
        };
        TosError error;
        bool success;
        if (copy) {
            auto input = copyInput();
            input.setCopyEventListener({[=](const std::shared_ptr<CopyEvent>& event) {
                if (event->type_ >= 3 && event->type_ <= 5) {
                    throwException();
                }
            }});
            const auto result = client->resumableCopyObject(input);
            success = result.isSuccess();
            error = result.error();
        } else {
            auto input = uploadInput();
            input.setUploadEventListener({[=](const std::shared_ptr<UploadEvent>& event) {
                if (event->type_ >= 3 && event->type_ <= 5) {
                    throwException();
                }
            }});
            const auto result = client->uploadFile(input);
            success = result.isSuccess();
            error = result.error();
        }
        const bool ok =
                !success && completes == 0 &&
                (status != 0
                         ? (!error.isClientError() && error.getCode() == failureCode &&
                            error.getStatusCode() == status && error.getRequestId() == "request-id" &&
                            abortCalls == (status == 403 ? 1 : 0))
                         : (error.isClientError() && abortCalls == 0 &&
                            error.getMessage().find(unknown ? "unknown exception"
                                                            : "injected event callback failure") != std::string::npos));
        TearDown();
        return ok ? 0 : 1;
    }
#endif
};

TEST_F(SyncErrorPropagationTest, BothRoundTripOverloadsPreserveTransportFailures) {
    failingMethod = "probe";
    for (bool multipleCodes : {false, true}) {
        for (int curlCode : {6, 7, 28}) {
            failureCurlCode = curlCode;
            failureStatus = curlCode == 7 ? -1 : -2;
            auto req = std::make_shared<TosRequest>();
            req->setFuncName(failingMethod);
            req->setMethod(http::MethodGet);
            const auto result = TosClientImplTestPeer::request(*client, req, multipleCodes);
            ASSERT_FALSE(result.isSuccess());
            EXPECT_EQ(curlCode, result.error().getCurlErrCode());
            EXPECT_EQ(failureStatus, result.error().getStatusCode());
            EXPECT_TRUE(result.error().isClientError());
        }
    }
}

TEST_F(SyncErrorPropagationTest, UploadInitializationPreservesNetworkAndServiceErrors) {
    failingMethod = "createMultipartUpload";
    for (int curlCode : {6, 7, 0}) {
        failureCurlCode = curlCode;
        failureStatus = curlCode == 0 ? 403 : (curlCode == 7 ? -1 : -2);
        const auto expected = expectedError(failingMethod, "create");
        const auto result = client->uploadFile(uploadInput());
        ASSERT_FALSE(result.isSuccess());
        expectSameError(expected, result.error());
    }
}

TEST_F(SyncErrorPropagationTest, LegacyUploadPreservesInitializationPartAndCompleteErrors) {
    for (const auto& stage :
         std::vector<std::pair<std::string, std::string>>{{"createMultipartUpload", "create"},
                                                          {"uploadPart", "part"},
                                                          {"completeMultipartUpload", "complete"}}) {
        failingMethod = stage.first;
        const auto expected = expectedError(stage.first, stage.second);
        const auto result = client->uploadFile("test-bucket", legacyInput(), RequestOptionBuilder());
        ASSERT_FALSE(result.isSuccess());
        expectSameError(expected, result.error());
    }
}

TEST_F(SyncErrorPropagationTest, UploadPartFailureSurvivesAbortCleanupFailure) {
    failingMethod = "uploadPart";
    failCleanup = true;
    const auto expected = expectedError(failingMethod, "part");
    const auto result = client->uploadFile(uploadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(1, abortCalls.load());
    expectSameError(expected, result.error());
}

TEST_F(SyncErrorPropagationTest, RetryableUploadPartFailureIsNotReplacedByGenericMessage) {
    failingMethod = "uploadPart";
    failureStatus = -2;
    failureCurlCode = 28;
    const auto expected = expectedError(failingMethod, "part");
    const auto result = client->uploadFile(uploadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(0, abortCalls.load());
    expectSameError(expected, result.error());
}

TEST_F(SyncErrorPropagationTest, CopyPreservesInitializationPartAndCompleteErrors) {
    for (const auto& stage :
         std::vector<std::pair<std::string, std::string>>{{"createMultipartUpload", "create"},
                                                          {"uploadPartCopy", "part"},
                                                          {"completeMultipartUpload", "complete"}}) {
        SCOPED_TRACE(stage.first);
        failingMethod = stage.first;
        const auto expected = expectedError(stage.first, stage.second);
        const auto result = client->resumableCopyObject(copyInput());
        ASSERT_FALSE(result.isSuccess());
        expectSameError(expected, result.error());
    }
}

TEST_F(SyncErrorPropagationTest, DownloadPreservesPartNetworkAndServiceErrors) {
    failingMethod = "getObject";
    for (int curlCode : {0, 6}) {
        failureCurlCode = curlCode;
        failureStatus = curlCode == 0 ? 403 : -2;
        const auto expected = expectedError(failingMethod, "get");
        const auto result = client->downloadFile(downloadInput());
        ASSERT_FALSE(result.isSuccess());
        expectSameError(expected, result.error());
    }
}

TEST_F(SyncErrorPropagationTest, UserPauseKeepsCancellationSemantics) {
    auto cancel = std::make_shared<MyCancelHook>();
    cancel->Cancel(false);
    auto input = uploadInput();
    input.setCancelHook(cancel);
    const auto result = client->uploadFile(input);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_EQ("the task is canceled", result.error().getMessage());
    EXPECT_EQ(0, abortCalls.load());
    EXPECT_EQ(0, partCalls.load());
}

TEST_F(SyncErrorPropagationTest, SuccessPathsStillWork) {
    auto upload = client->uploadFile(uploadInput());
    EXPECT_TRUE(upload.isSuccess()) << upload.error().String();
    auto legacy = client->uploadFile("test-bucket", legacyInput(), RequestOptionBuilder());
    EXPECT_TRUE(legacy.isSuccess()) << legacy.error().String();
    auto copy = client->resumableCopyObject(copyInput());
    EXPECT_TRUE(copy.isSuccess()) << copy.error().String();
    auto download = client->downloadFile(downloadInput());
    EXPECT_TRUE(download.isSuccess()) << download.error().String();
    std::ifstream file(path + ".download", std::ios::binary);
    EXPECT_EQ("abcdef", std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()));
}

TEST_F(SyncErrorPropagationTest, LocalValidationRemainsClientError) {
    auto input = uploadInput();
    input.setPartSize(1);
    const auto result = client->uploadFile(input);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_EQ(0, result.error().getCurlErrCode());
}

TEST_F(SyncErrorPropagationTest, TimeoutSpecialCasePreservesResponseDiagnostics) {
    transport->handler = [](const std::shared_ptr<TosRequest>&) {
        auto res = response(-2, "", 28);
        res->setStatusMsg("operation timeout");
        return res;
    };
    for (bool multipleCodes : {false, true}) {
        const auto result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes);
        ASSERT_FALSE(result.isSuccess());
        EXPECT_EQ(-2, result.error().getStatusCode());
        EXPECT_EQ(28, result.error().getCurlErrCode());
        EXPECT_EQ("request-id", result.error().getRequestId());
        EXPECT_EQ("operation timeout", result.error().getCode());
        EXPECT_TRUE(result.error().isClientError());
    }
}

TEST_F(SyncErrorPropagationTest, MalformedErrorBodyKeepsRawTextAndHeaderDiagnostics) {
    for (bool multipleCodes : {false, true}) {
        for (int status : {502, 203}) {
            for (const std::string body :
                 {"upstream connection reset", "<html>Bad Gateway</html>", "{\"Code\":", "{}", ""}) {
                SCOPED_TRACE(std::to_string(status) + ": " + body);
                transport->handler = [=](const std::shared_ptr<TosRequest>&) { return response(status, body); };
                auto req = std::make_shared<TosRequest>();
                req->setScheme("https");
                req->setHost("test-bucket.unit-test.invalid");
                req->setPath("/key");
                req->setMethod(http::MethodPost);
                req->setSingleHeader(HEADER_CALLBACK, "callback");
                const auto result = TosClientImplTestPeer::request(*client, req, multipleCodes);
                ASSERT_FALSE(result.isSuccess());
                EXPECT_EQ(body, result.error().getMessage());
                EXPECT_EQ("unable to do serialization", result.error().getCode());
                EXPECT_TRUE(result.error().isClientError());
                EXPECT_EQ(status, result.error().getStatusCode());
                EXPECT_EQ(0, result.error().getCurlErrCode());
                EXPECT_EQ("request-id", result.error().getRequestId());
                EXPECT_EQ("header-ec", result.error().getEc());
                EXPECT_EQ(req->toUrl().toString(), result.error().getRequestUrl());
                EXPECT_EQ("request-id", result.error().getRequestInfo().getRequestId());
                EXPECT_EQ("id-2", result.error().getRequestInfo().getId2());
                EXPECT_EQ(status, result.error().getRequestInfo().getStatusCode());
            }
        }
    }
}

TEST_F(SyncErrorPropagationTest, ServiceErrorWithoutBodyRequestIdUsesHeaderRequestId) {
    transport->handler = [](const std::shared_ptr<TosRequest>&) {
        return response(403, "{\"Code\":\"AccessDenied\",\"Message\":\"original service message\"}");
    };
    for (bool multipleCodes : {false, true}) {
        const auto result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes);
        ASSERT_FALSE(result.isSuccess());
        EXPECT_EQ("AccessDenied", result.error().getCode());
        EXPECT_EQ("original service message", result.error().getMessage());
        EXPECT_EQ("request-id", result.error().getRequestId());
        EXPECT_EQ("header-ec", result.error().getEc());
        EXPECT_FALSE(result.error().isClientError());
    }
}

TEST_F(SyncErrorPropagationTest, ServiceBodyRequestIdTakesPrecedenceOverHeader) {
    transport->handler = [](const std::shared_ptr<TosRequest>&) {
        return response(403,
                        "{\"Code\":\"AccessDenied\",\"Message\":\"original service message\","
                        "\"RequestId\":\"body-request-id\",\"EC\":\"body-ec\"}");
    };
    for (bool multipleCodes : {false, true}) {
        const auto result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes);
        ASSERT_FALSE(result.isSuccess());
        EXPECT_EQ("body-request-id", result.error().getRequestId());
        EXPECT_EQ("request-id", result.error().getRequestInfo().getRequestId());
        EXPECT_EQ("body-ec", result.error().getEc());
        EXPECT_EQ("AccessDenied", result.error().getCode());
        EXPECT_FALSE(result.error().isClientError());
    }
}

TEST_F(SyncErrorPropagationTest, MalformedErrorBodyWithoutHeaderRequestIdStillUsesRawText) {
    transport->handler = [](const std::shared_ptr<TosRequest>&) {
        auto res = response(502, "upstream failure");
        auto headers = res->getHeaders();
        headers.erase("x-tos-request-id");
        res->setHeaders(headers);
        return res;
    };
    for (bool multipleCodes : {false, true}) {
        const auto result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes);
        ASSERT_FALSE(result.isSuccess());
        EXPECT_EQ("upstream failure", result.error().getMessage());
        EXPECT_EQ("unable to do serialization", result.error().getCode());
        EXPECT_TRUE(result.error().getRequestId().empty());
        EXPECT_TRUE(result.error().isClientError());
    }
}

TEST_F(SyncErrorPropagationTest, NonStringErrorFieldsFallBackWithoutThrowing) {
    using nlohmann::json;
    for (const std::string field : {"Code", "Message", "RequestId", "HostId", "EC"}) {
        for (const auto& value : {json(nullptr), json(123), json(false), json::array(), json::object()}) {
            const auto body = json{{field, value}}.dump();
            SCOPED_TRACE(body);
            transport->handler = [body](const std::shared_ptr<TosRequest>&) { return response(502, body); };
            for (bool multipleCodes : {false, true}) {
                Outcome<TosError, std::shared_ptr<TosResponse>> result;
                ASSERT_NO_THROW(result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(),
                                                                        multipleCodes));
                ASSERT_FALSE(result.isSuccess());
                EXPECT_EQ(body, result.error().getMessage());
                EXPECT_EQ("unable to do serialization", result.error().getCode());
                EXPECT_EQ(502, result.error().getStatusCode());
                EXPECT_EQ("request-id", result.error().getRequestId());
                EXPECT_EQ("header-ec", result.error().getEc());
                EXPECT_TRUE(result.error().isClientError());
            }
        }
    }
}

TEST_F(SyncErrorPropagationTest, NonObjectErrorJsonFallsBackToRawBody) {
    for (const std::string body : {"null", "[]", "123", "false", "\"error text\""}) {
        transport->handler = [body](const std::shared_ptr<TosRequest>&) { return response(502, body); };
        for (bool multipleCodes : {false, true}) {
            const auto result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes);
            ASSERT_FALSE(result.isSuccess());
            EXPECT_EQ(body, result.error().getMessage());
            EXPECT_EQ("unable to do serialization", result.error().getCode());
            EXPECT_EQ("request-id", result.error().getRequestId());
            EXPECT_EQ(502, result.error().getStatusCode());
        }
    }
}

TEST_F(SyncErrorPropagationTest, ValidErrorFieldsSurviveMalformedOptionalFields) {
    using nlohmann::json;
    for (const std::string field : {"Code", "Message", "RequestId", "HostId", "EC"}) {
        json bodyJson = {{"Code", "AccessDenied"},
                         {"Message", "original message"},
                         {"RequestId", "body-id"},
                         {"HostId", "host-id"},
                         {"EC", "body-ec"}};
        bodyJson[field] = nullptr;
        const auto body = bodyJson.dump();
        SCOPED_TRACE(body);
        transport->handler = [body](const std::shared_ptr<TosRequest>&) { return response(403, body); };
        for (bool multipleCodes : {false, true}) {
            Outcome<TosError, std::shared_ptr<TosResponse>> result;
            ASSERT_NO_THROW(
                    result = TosClientImplTestPeer::request(*client, std::make_shared<TosRequest>(), multipleCodes));
            EXPECT_EQ(field == "Code" ? "" : "AccessDenied", result.error().getCode());
            EXPECT_EQ(field == "Message" ? "" : "original message", result.error().getMessage());
            EXPECT_EQ(field == "RequestId" ? "request-id" : "body-id", result.error().getRequestId());
            EXPECT_EQ(field == "HostId" ? "" : "host-id", result.error().getHostId());
            EXPECT_EQ(field == "EC" ? "header-ec" : "body-ec", result.error().getEc());
            EXPECT_FALSE(result.error().isClientError());
        }
    }
}

#if GTEST_HAS_DEATH_TEST && !defined(_WIN32)
TEST_F(SyncErrorPropagationTest, UploadAndCopyWorkerExceptionsDoNotTerminate) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    for (const std::string transfer : {"uploadPart", "uploadPartCopy", "legacy"}) {
        for (bool unknown : {false, true}) {
            SCOPED_TRACE(transfer);
            ASSERT_EXIT(std::_Exit(workerExceptionResult(transfer, unknown)), ::testing::ExitedWithCode(0), "");
        }
    }
}

TEST_F(SyncErrorPropagationTest, DownloadUnknownWorkerExceptionDoesNotTerminate) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    ASSERT_EXIT(std::_Exit(workerExceptionResult("getObject", true)), ::testing::ExitedWithCode(0), "");
}

TEST_F(SyncErrorPropagationTest, UploadAndCopyEventExceptionsDoNotTerminate) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    for (bool copy : {false, true}) {
        for (bool unknown : {false, true}) {
            ASSERT_EXIT(std::_Exit(callbackExceptionResult(copy, unknown, 0)), ::testing::ExitedWithCode(0), "");
        }
    }
}

TEST_F(SyncErrorPropagationTest, CallbackExceptionKeepsOriginalFatalErrorAndCleanup) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    for (bool copy : {false, true}) {
        for (bool unknown : {false, true}) {
            ASSERT_EXIT(std::_Exit(callbackExceptionResult(copy, unknown, 403)), ::testing::ExitedWithCode(0), "");
        }
    }
}

TEST_F(SyncErrorPropagationTest, CallbackExceptionKeepsOriginalRetryableErrorWithoutCleanup) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    for (bool copy : {false, true}) {
        for (bool unknown : {false, true}) {
            ASSERT_EXIT(std::_Exit(callbackExceptionResult(copy, unknown, 500)), ::testing::ExitedWithCode(0), "");
        }
    }
}
#endif

TEST_F(SyncErrorPropagationTest, UploadAndCopyWorkerExceptionsKeepCheckpointAndCanResume) {
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(kPartSize);
        file.put('x');
    }
    const auto fallback = transport->handler;
    for (bool copy : {false, true}) {
        SCOPED_TRACE(copy);
        { std::ofstream checkpoint(path + ".upload"); }
        bool fail = true;
        int creates = 0;
        std::vector<std::string> parts;
        transport->handler = [&](const std::shared_ptr<TosRequest>& req) {
            const auto method = operation(req);
            if (method == "createMultipartUpload") {
                ++creates;
            }
            if (method == "headObject") {
                auto res = fallback(req);
                auto headers = res->getHeaders();
                headers["Content-Length"] = std::to_string(kPartSize + 1);
                res->setHeaders(headers);
                res->setContentLength(kPartSize + 1);
                return res;
            }
            if (method == (copy ? "uploadPartCopy" : "uploadPart")) {
                const auto number = req->getQueries().at("partNumber");
                parts.push_back(number);
                if (fail && number == "2") {
                    throw std::runtime_error("injected resumable worker failure");
                }
            }
            return fallback(req);
        };
        auto upload = uploadInput();
        upload.setEnableCheckpoint(true);
        upload.setCheckpointFile(path + ".upload");
        auto copyRequest = copyInput();
        copyRequest.setEnableCheckpoint(true);
        copyRequest.setCheckpointFile(path + ".upload");
        const auto invoke = [&]() {
            if (copy) {
                return client->resumableCopyObject(copyRequest).isSuccess();
            }
            return client->uploadFile(upload).isSuccess();
        };
        EXPECT_FALSE(invoke());
        EXPECT_TRUE(std::ifstream(path + ".upload").good());
        EXPECT_EQ(0, abortCalls.load());
        EXPECT_EQ(1, creates);
        EXPECT_EQ((std::vector<std::string>{"1", "2"}), parts);

        fail = false;
        parts.clear();
        EXPECT_TRUE(invoke());
        EXPECT_EQ(1, creates);
        EXPECT_EQ((std::vector<std::string>{"2"}), parts);
        EXPECT_FALSE(std::ifstream(path + ".upload").good());
        EXPECT_EQ(0, abortCalls.load());
    }
}

TEST_F(SyncErrorPropagationTest, WorkerExceptionWaitsForOtherWorkersBeforeReturning) {
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(kPartSize * 2);
        file.put('x');
    }
    std::mutex mutex;
    std::condition_variable cv;
    int arrived = 0;
    std::atomic<int> finished{0};
    std::atomic<int> completes{0};
    const auto fallback = transport->handler;
    transport->handler = [&](const std::shared_ptr<TosRequest>& req) {
        const auto method = operation(req);
        if (method == "completeMultipartUpload") {
            ++completes;
        }
        if (method == "uploadPart") {
            {
                std::unique_lock<std::mutex> lock(mutex);
                ++arrived;
                cv.notify_all();
                if (!cv.wait_for(lock, std::chrono::seconds(5), [&]() { return arrived == 3; })) {
                    throw std::runtime_error("test worker barrier timed out");
                }
            }
            if (req->getQueries().at("partNumber") == "1") {
                throw std::runtime_error("injected concurrent worker failure");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            ++finished;
        }
        return fallback(req);
    };
    auto input = uploadInput();
    input.setTaskNum(3);
    const auto result = client->uploadFile(input);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_NE(std::string::npos, result.error().getMessage().find("injected concurrent worker failure"));
    EXPECT_EQ(3, arrived);
    EXPECT_EQ(2, finished.load());
    EXPECT_EQ(0, completes.load());
    EXPECT_EQ(0, abortCalls.load());
}

TEST_F(SyncErrorPropagationTest, RejectedEndpointReturnsErrorWithoutNullResponseCrash) {
    for (const auto& endpoint : {"http://127.0.0.1", "https://tos-s3-cn-beijing.volces.com"}) {
        ClientConfig config;
        config.maxRetryCount = 0;
        TosClientImpl rejected(endpoint, "cn-beijing", StaticCredentials("ak", "sk"), config);
        auto upload = rejected.uploadFile(uploadInput());
        ASSERT_FALSE(upload.isSuccess());
        EXPECT_TRUE(upload.error().isClientError());
        EXPECT_NE(std::string::npos, upload.error().getMessage().find("please do not use"));
        auto get = rejected.getObject(GetObjectV2Input("test-bucket", "key"), nullptr, nullptr);
        ASSERT_FALSE(get.isSuccess());
        EXPECT_TRUE(get.error().isClientError());
    }
}

TEST_F(SyncErrorPropagationTest, HeadFailureIsPreservedByDownloadAndCopy) {
    failingMethod = "headObject";
    auto expected = expectedError(failingMethod, "get");
    auto download = client->downloadFile(downloadInput());
    ASSERT_FALSE(download.isSuccess());
    expectSameError(expected, download.error());
    auto copy = copyInput();
    copy.setSrcBucket("test-bucket");
    copy.setSrcKey("key");
    auto copied = client->resumableCopyObject(copy);
    ASSERT_FALSE(copied.isSuccess());
    expectSameError(expected, copied.error());
}

TEST_F(SyncErrorPropagationTest, UploadCompleteFailureRetainsNoSuchUploadContextAndCleanupOption) {
    failingMethod = "completeMultipartUpload";
    failureStatus = 404;
    failureCode = "NoSuchUpload";
    const auto expected = expectedError(failingMethod, "complete");
    for (bool cleanup : {false, true}) {
        { std::ofstream checkpoint(path + ".upload"); }
        auto input = uploadInput();
        input.setEnableCheckpoint(true);
        input.setCheckpointFile(path + ".upload");
        input.setEnableCheckpointCleanupOnNoSuchUpload(cleanup);
        const auto result = client->uploadFile(input);
        ASSERT_FALSE(result.isSuccess());
        expectSameError(expected, result.error(), false);
        EXPECT_NE(std::string::npos, result.error().getMessage().find("completed remotely, aborted, or expired"));
        EXPECT_NE(std::string::npos, result.error().getMessage().find(expected.getMessage()));
        EXPECT_EQ(!cleanup, std::ifstream(path + ".upload").good());
        std::remove((path + ".upload").c_str());
    }
}

TEST_F(SyncErrorPropagationTest, UploadCompleteNetworkErrorIsUnchanged) {
    failingMethod = "completeMultipartUpload";
    failureCurlCode = 28;
    failureStatus = -2;
    const auto expected = expectedError(failingMethod, "complete");
    const auto result = client->uploadFile(uploadInput());
    ASSERT_FALSE(result.isSuccess());
    expectSameError(expected, result.error());
}

TEST_F(SyncErrorPropagationTest, PauseDuringPartAndExplicitAbortKeepTheirMeaning) {
    auto cancel = std::make_shared<MyCancelHook>();
    cancelOnPart = cancel;
    auto input = uploadInput();
    input.setCancelHook(cancel);
    auto result = client->uploadFile(input);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ("the task is canceled", result.error().getMessage());
    EXPECT_EQ(0, abortCalls.load());
    cancelOnPart.reset();
    cancel = std::make_shared<MyCancelHook>();
    cancel->Cancel(true);
    input.setCancelHook(cancel);
    result = client->uploadFile(input);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ("the task is canceled", result.error().getMessage());
    EXPECT_EQ(1, abortCalls.load());
}

TEST_F(SyncErrorPropagationTest, DownloadLocalFailureReportsIoCauseInsteadOfCancellation) {
    const auto fallback = transport->handler;
    transport->handler = [this, fallback](const std::shared_ptr<TosRequest>& req) {
        if (operation(req) == "getObject") {
            std::remove((path + ".download.temp").c_str());
        }
        return fallback(req);
    };
    const auto result = client->downloadFile(downloadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_NE(std::string::npos, result.error().getMessage().find("failed to open download temporary file"));
}

TEST_F(SyncErrorPropagationTest, DownloadWorkerExceptionRetainsItsCause) {
    const auto fallback = transport->handler;
    transport->handler = [fallback](const std::shared_ptr<TosRequest>& req) {
        if (operation(req) == "getObject") {
            throw std::runtime_error("simulated download failure");
        }
        return fallback(req);
    };
    const auto result = client->downloadFile(downloadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_NE(std::string::npos, result.error().getMessage().find("simulated download failure"));
}

TEST_F(SyncErrorPropagationTest, FirstFailureIsNotReplacedByLaterFatalPartFailure) {
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(kPartSize);
        file.put('x');
    }
    failingMethod = "uploadPart";
    failureCurlCode = 28;
    failureStatus = -2;
    const auto expected = expectedError(failingMethod, "part");
    const auto fallback = transport->handler;
    transport->handler = [fallback](const std::shared_ptr<TosRequest>& req) {
        if (operation(req) == "uploadPart" && req->getQueries().at("partNumber") == "2") {
            return serverError(403, "LaterFailure");
        }
        return fallback(req);
    };
    const auto result = client->uploadFile(uploadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(1, abortCalls.load());
    expectSameError(expected, result.error());
}

TEST_F(SyncErrorPropagationTest, UploadCheckpointCanResumeAfterPartFailure) {
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(kPartSize);
        file.put('x');
    }
    { std::ofstream checkpoint(path + ".upload"); }
    auto input = uploadInput();
    input.setEnableCheckpoint(true);
    input.setCheckpointFile(path + ".upload");
    bool failPartTwo = true;
    std::vector<std::string> requestedParts;
    const auto fallback = transport->handler;
    transport->handler = [&](const std::shared_ptr<TosRequest>& req) {
        if (operation(req) == "uploadPart") {
            const auto number = req->getQueries().at("partNumber");
            requestedParts.push_back(number);
            if (number == "2" && failPartTwo) {
                return response(-2, "", 28);
            }
        }
        return fallback(req);
    };
    auto first = client->uploadFile(input);
    ASSERT_FALSE(first.isSuccess());
    EXPECT_EQ(28, first.error().getCurlErrCode());
    ASSERT_TRUE(std::ifstream(path + ".upload").good());
    EXPECT_EQ((std::vector<std::string>{"1", "2"}), requestedParts);
    failPartTwo = false;
    requestedParts.clear();
    const auto resumed = client->uploadFile(input);
    ASSERT_TRUE(resumed.isSuccess()) << resumed.error().String();
    EXPECT_EQ((std::vector<std::string>{"2"}), requestedParts);
    EXPECT_FALSE(std::ifstream(path + ".upload").good());
}

TEST_F(SyncErrorPropagationTest, DownloadAndCopyKeepUserCancellationSemantics) {
    for (bool abort : {false, true}) {
        auto cancel = std::make_shared<MyCancelHook>();
        cancel->Cancel(abort);
        auto download = downloadInput();
        download.setCancelHook(cancel);
        auto downloaded = client->downloadFile(download);
        ASSERT_FALSE(downloaded.isSuccess());
        EXPECT_EQ("the task is canceled", downloaded.error().getMessage());
        EXPECT_TRUE(downloaded.error().isClientError());
        auto copy = copyInput();
        copy.setCancelHook(cancel);
        auto copied = client->resumableCopyObject(copy);
        ASSERT_FALSE(copied.isSuccess());
        EXPECT_EQ("the task is canceled", copied.error().getMessage());
        EXPECT_TRUE(copied.error().isClientError());
    }
    EXPECT_EQ(1, abortCalls.load());
}

TEST_F(SyncErrorPropagationTest, ConcurrentFailuresKeepOneCompleteErrorAcrossUploadDownloadAndCopy) {
    {
        std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(2 * kPartSize);
        file.put('x');
    }
    const auto fallback = transport->handler;
    for (const auto& stage : {"uploadPart", "getObject", "uploadPartCopy"}) {
        SCOPED_TRACE(stage);
        std::mutex mutex;
        std::condition_variable ready;
        int arrived = 0;
        transport->handler = [&, stage](const std::shared_ptr<TosRequest>& req) {
            if (operation(req) == "headObject") {
                auto res = response(200);
                res->setContentLength(2 * kPartSize + 1);
                auto headers = res->getHeaders();
                headers["Content-Length"] = std::to_string(2 * kPartSize + 1);
                res->setHeaders(headers);
                return res;
            }
            if (operation(req) != stage) {
                return fallback(req);
            }
            int id;
            {
                std::unique_lock<std::mutex> lock(mutex);
                id = ++arrived;
                ready.notify_all();
                ready.wait_for(lock, std::chrono::seconds(5), [&] { return arrived == 3; });
            }
            const auto tag = std::to_string(id);
            return response(409, "{\"Code\":\"Part" + tag + "\",\"Message\":\"failure-" + tag +
                                         "\",\"RequestId\":\"request-" + tag + "\",\"HostId\":\"host-" + tag + "\"}");
        };
        TosError error;
        if (std::string(stage) == "uploadPart") {
            auto input = uploadInput();
            input.setTaskNum(3);
            auto result = client->uploadFile(input);
            ASSERT_FALSE(result.isSuccess());
            error = result.error();
        } else if (std::string(stage) == "getObject") {
            auto input = downloadInput();
            input.setTaskNum(3);
            auto result = client->downloadFile(input);
            ASSERT_FALSE(result.isSuccess());
            error = result.error();
        } else {
            auto input = copyInput();
            input.setTaskNum(3);
            auto result = client->resumableCopyObject(input);
            ASSERT_FALSE(result.isSuccess());
            error = result.error();
        }
        EXPECT_EQ(3, arrived);
        EXPECT_EQ(409, error.getStatusCode());
        ASSERT_EQ(0u, error.getCode().find("Part"));
        const auto tag = error.getCode().substr(4);
        EXPECT_EQ("failure-" + tag, error.getMessage());
        EXPECT_EQ("request-" + tag, error.getRequestId());
        EXPECT_EQ("host-" + tag, error.getHostId());
        EXPECT_FALSE(error.isClientError());
        EXPECT_EQ(409, error.getRequestInfo().getStatusCode());
        EXPECT_FALSE(error.getRequestUrl().empty());
    }
}

#ifndef _WIN32
class ScopedNoProxy {
public:
    ScopedNoProxy() {
        for (const auto& name : {"NO_PROXY", "no_proxy"}) {
            const char* previous = std::getenv(name);
            if (previous != nullptr) {
                previous_[name] = previous;
            }
            setenv(name, "*", 1);
        }
    }
    ~ScopedNoProxy() {
        for (const auto& name : {"NO_PROXY", "no_proxy"}) {
            const auto previous = previous_.find(name);
            if (previous == previous_.end()) {
                unsetenv(name);
            } else {
                setenv(name, previous->second.c_str(), 1);
            }
        }
    }

private:
    std::map<std::string, std::string> previous_;
};

TEST_F(SyncErrorPropagationTest, RealDnsFailureReachesUploadFileCaller) {
    ScopedNoProxy noProxy;
    ClientConfig config;
    config.maxRetryCount = 0;
    config.connectionTimeout = 1000;
    config.requestTimeout = 3000;
    config.isCustomDomain = true;
    TosClientImpl realClient("http://tos-sdk-error-test.invalid", "cn-beijing", StaticCredentials("ak", "sk"), config);
    const auto result = realClient.uploadFile(uploadInput());
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(6, result.error().getCurlErrCode()) << result.error().getMessage();
    EXPECT_EQ(-2, result.error().getStatusCode());
    EXPECT_EQ("UnexpectedStatusCode error", result.error().getCode());
    EXPECT_TRUE(result.error().isClientError());
    EXPECT_FALSE(result.error().getRequestUrl().empty());
}

TEST_F(SyncErrorPropagationTest, RealConnectionFailureReachesUploadFileCaller) {
    ScopedNoProxy noProxy;
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(0, bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)));
    socklen_t size = sizeof(address);
    ASSERT_EQ(0, getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size));
    ClientConfig config;
    config.maxRetryCount = 0;
    config.connectionTimeout = 1000;
    config.requestTimeout = 3000;
    config.isCustomDomain = true;
    TosClientImpl realClient("http://localhost:" + std::to_string(ntohs(address.sin_port)), "cn-beijing",
                             StaticCredentials("ak", "sk"), config);
    // Production endpoint validation rejects explicit ports. Only bypass that
    // guard here so the real HTTP transport can use a private ephemeral port.
    TosClientImplTestPeer::allowLocalTestPort(realClient);
    const auto result = realClient.uploadFile(uploadInput());
    close(fd);
    ASSERT_FALSE(result.isSuccess());
    EXPECT_EQ(7, result.error().getCurlErrCode()) << result.error().getMessage();
    EXPECT_EQ(-1, result.error().getStatusCode());
    EXPECT_TRUE(result.error().isClientError());
}
#endif
}  // namespace
