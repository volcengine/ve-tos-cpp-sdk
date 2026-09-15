#include "LoopbackHttpFixture.h"
#include "TosAsyncClient.h"
#include "executor/TosClientTemplate.h"
#include "logger/logger.h"
#include <curl/curl.h>
#include <iostream>
#include <limits>

using namespace VolcengineTos;
using Server = tos_test::LoopbackHttpFixture;
#define CHECK(x)                                \
    do {                                        \
        if (!(x)) throw std::runtime_error(#x); \
    } while (false)

void RetryRead(bool partial) {
    Server::Response first;
    first.status = partial ? 200 : 503;
    first.reason = partial ? "OK" : "Service Unavailable";
    first.body = partial ? "partial" : "";
    if (partial) first.declared_content_length = 64;
    std::vector<Server::Step> steps{{"GET", "/retry", first}};
    if (!partial) {
        Server::Response second;
        second.body = "complete";
        steps.push_back({"GET", "/retry", second});
    }
    Server server(std::move(steps));
    ClientConfig config;
    config.endPoint = server.Endpoint();
    config.isCustomDomain = true;
    config.event_thread_count_ = 1;
    config.async_transport_mode_ = AsyncTransportMode::Isolated;
    config.maxConnections = 1;
    config.maxRetryCount = 1;
    config.requestTimeout = 1500;
    config.enableCRC = false;
    config.connection_reuse_ = false;
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), config);
    std::mutex mutex;
    std::condition_variable cv;
    unsigned calls = 0;
    bool success = false;
    std::string bytes;
    client.getObjectAsync(GetObjectAsyncInput("fixture-bucket", "retry"),
                          [&](char* data, size_t length, AsyncEvent*) {
                              bytes.append(data, length);
                              return length;
                          },
                          [&](auto& outcome) {
                              std::lock_guard<std::mutex> lock(mutex);
                              ++calls;
                              success = outcome.isSuccess();
                              cv.notify_all();
                          });
    {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls > 0; }));
    }
    client.close();
    server.AssertDone();
    CHECK(calls == 1 && success == !partial);
    CHECK(bytes == (partial ? "partial" : "complete"));
    CHECK(server.Requests().size() == (partial ? 1 : 2));
}

// Exercise local failures with retries enabled. A second request to the same
// key would violate the finite script (the next step is a healthy different
// key), so a retry cannot accidentally turn this test into a success.
void TerminalReceiveFailures() {
    enum class Fault { ThrowBefore, ThrowAfter, ThrowNonStandard, FailEvent, FailAfter, Zero, Oversize };
    const std::vector<Fault> faults{Fault::ThrowBefore, Fault::ThrowAfter, Fault::ThrowNonStandard,
                                    Fault::FailEvent, Fault::FailAfter, Fault::Zero, Fault::Oversize};
    std::vector<Server::Step> steps;
    for (size_t i = 0; i < faults.size(); ++i) {
        Server::Response response;
        response.body = "abc";
        // Local terminal errors must win over retryable HTTP statuses too.
        if (faults[i] == Fault::FailEvent) {
            response.status = 503;
            response.reason = "Service Unavailable";
        }
        steps.push_back({"GET", "/terminal-" + std::to_string(i), response});
        response.status = 200;
        response.reason = "OK";
        steps.push_back({"GET", "/healthy-" + std::to_string(i), response});
    }
    Server server(std::move(steps));
    ClientConfig config;
    config.endPoint = server.Endpoint();
    config.isCustomDomain = true;
    config.event_thread_count_ = 1;
    config.async_transport_mode_ = AsyncTransportMode::Isolated;
    config.maxConnections = 1;
    config.maxRetryCount = 3;
    config.requestTimeout = 1500;
    config.enableCRC = false;
    config.connection_reuse_ = false;
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), config);
    for (size_t i = 0; i < faults.size(); ++i) {
        for (const bool healthy : {false, true}) {
            const auto fault = faults[i];
            unsigned receives = 0, calls = 0;
            std::string bytes;
            bool success = false;
            int curl_error = 0;
            std::mutex mutex;
            std::condition_variable cv;
            client.getObjectAsync(GetObjectAsyncInput("fixture-bucket",
                                        (healthy ? "healthy-" : "terminal-") + std::to_string(i)),
                [&](char* data, size_t length, AsyncEvent* event) -> size_t {
                    ++receives;
                    if (!healthy) {
                        if (fault == Fault::ThrowBefore) throw std::runtime_error("before consume");
                        if (fault == Fault::ThrowNonStandard) throw 17;
                        if (fault == Fault::FailEvent) { event->markFailed(); return 0; }
                        if (fault == Fault::Zero) return 0;
                        if (fault == Fault::Oversize) return length + 1;
                    }
                    bytes.append(data, length);
                    if (!healthy && fault == Fault::ThrowAfter) throw std::runtime_error("after consume");
                    if (!healthy && fault == Fault::FailAfter) event->markFailed();
                    return length;
                },
                [&](auto& outcome) {
                    std::lock_guard<std::mutex> lock(mutex);
                    ++calls;
                    success = outcome.isSuccess();
                    curl_error = outcome.error().getCurlErrCode();
                    cv.notify_all();
                });
            std::unique_lock<std::mutex> lock(mutex);
            CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls != 0; }));
            CHECK(calls == 1 && receives == 1 && success == healthy);
            CHECK(curl_error == (healthy ? CURLE_OK : CURLE_WRITE_ERROR));
            const bool consumed = healthy || fault == Fault::ThrowAfter || fault == Fault::FailAfter;
            CHECK(bytes == (consumed ? "abc" : ""));
        }
    }
    client.close();
    server.AssertDone();
    CHECK(server.Requests().size() == faults.size() * 2);
}
int main() {
    try {
        Logger::getInstance().setAsyncLogLevel(ERROR);
        CHECK(checkShouldRetry("getObject", 503, 0, 0));
        CHECK(!checkShouldRetry("getObject", 503, 0, 1));
        CHECK(!checkShouldRetry("getObjectAsync", 503, 0, int64_t(1) << 32));
        CHECK(!checkShouldRetry("getObject", 403, 0, 0));
        for (const char* method : {"getObject", "getObjectAsync", "headObject", "putObjectAsync"})
            for (const int status : {200, 429, 503})
                for (const int error : {CURLE_WRITE_ERROR, CURLE_READ_ERROR, CURLE_ABORTED_BY_CALLBACK,
                                       CURLE_SEND_FAIL_REWIND, CURLE_OUT_OF_MEMORY, CURLE_FAILED_INIT})
                    CHECK(!checkShouldRetry(method, status, error, 0));
        CHECK(checkShouldRetry("getObject", 0, CURLE_COULDNT_CONNECT, 0));
        CHECK(checkShouldRetry("getObject", 200, CURLE_OPERATION_TIMEDOUT, 0));
        RetryRead(false);
        RetryRead(true);
        TerminalReceiveFailures();
        std::cout << "PASS: empty GET retry, partial GET no replay, 64-bit byte count\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
