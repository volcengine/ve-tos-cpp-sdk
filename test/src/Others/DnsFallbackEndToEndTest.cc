#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "TosClientV2.h"
#ifdef VOLCENGINE_TOS_ASYNC_TEST
#include "TosAsyncClient.h"
#include "logger/logger.h"
#include "transport/http/async/AsyncHttpClient.h"
#endif

#ifndef _WIN32
namespace {

class DnsServer {
public:
    DnsServer() {
        fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd_ < 0)
            return;
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(0x7f000002);
        address.sin_port = htons(53);
        if (bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
            close(fd_);
            fd_ = -1;
            return;
        }
        thread_ = std::thread([this] { serve(); });
    }

    ~DnsServer() {
        stop_ = true;
        if (thread_.joinable())
            thread_.join();
        if (fd_ >= 0)
            close(fd_);
    }

    bool ready() const {
        return fd_ >= 0;
    }
    void setFailure(bool value) {
        failure_ = value;
    }
    void setDrop(bool value) {
        drop_ = value;
    }
    int queries() const {
        return queries_.load();
    }

private:
    void serve() {
        while (!stop_) {
            pollfd pfd{fd_, POLLIN, 0};
            if (poll(&pfd, 1, 100) <= 0 || !(pfd.revents & POLLIN))
                continue;
            uint8_t query[512];
            sockaddr_in peer{};
            socklen_t peerSize = sizeof(peer);
            const ssize_t size = recvfrom(fd_, query, sizeof(query), 0, reinterpret_cast<sockaddr*>(&peer), &peerSize);
            if (size < 17)
                continue;
            ++queries_;
            if (drop_)
                continue;
            size_t end = 12;
            while (end < static_cast<size_t>(size) && query[end] != 0)
                end += query[end] + 1;
            if (end + 5 > static_cast<size_t>(size))
                continue;
            end += 5;
            const bool answerA = !failure_ && query[end - 4] == 0 && query[end - 3] == 1;
            std::vector<uint8_t> reply(query, query + end);
            reply[2] = 0x81;
            reply[3] = failure_ ? 0x82 : 0x80;
            reply[4] = 0;
            reply[5] = 1;
            reply[6] = 0;
            reply[7] = answerA ? 1 : 0;
            reply[8] = reply[9] = reply[10] = reply[11] = 0;
            if (answerA) {
                const uint8_t answer[] = {0xc0, 0x0c, 0, 1, 0, 1, 0, 0, 0, 0, 0, 4, 127, 0, 0, 1};
                reply.insert(reply.end(), answer, answer + sizeof(answer));
            }
            (void)sendto(fd_, reply.data(), reply.size(), 0, reinterpret_cast<sockaddr*>(&peer), peerSize);
        }
    }

    int fd_ = -1;
    std::thread thread_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> failure_{false};
    std::atomic<bool> drop_{false};
    std::atomic<int> queries_{0};
};

class OneShotHttpServer {
public:
    OneShotHttpServer() {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0)
            return;
        int reuse = 1;
        (void)setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(80);
        if (bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(fd_, 1) != 0) {
            close(fd_);
            fd_ = -1;
            return;
        }
        thread_ = std::thread([this] {
            const int peer = accept(fd_, nullptr, nullptr);
            if (peer < 0)
                return;
            char request[2048];
            const ssize_t size = recv(peer, request, sizeof(request), 0);
            if (size > 0) {
                received_ = std::string(request, static_cast<size_t>(size));
                accepted_ = true;
            }
            const std::string reply = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            (void)send(peer, reply.data(), reply.size(), 0);
            close(peer);
        });
    }

    ~OneShotHttpServer() {
        if (fd_ >= 0) {
            shutdown(fd_, SHUT_RDWR);
            close(fd_);
        }
        if (thread_.joinable())
            thread_.join();
    }

    bool ready() const {
        return fd_ >= 0;
    }
    bool accepted() const {
        return accepted_.load();
    }
    const std::string& received() {
        if (thread_.joinable())
            thread_.join();
        return received_;
    }

private:
    int fd_ = -1;
    std::thread thread_;
    std::string received_;
    std::atomic<bool> accepted_{false};
};

}  // namespace

TEST(DnsFallbackEndToEndTest, PublicHeadBucketUsesLastGoodAfterRealDnsFailure) {
    if (std::getenv("FSX_DNS_REAL_TEST") == nullptr)
        return;
    setenv("NO_PROXY", "*", 1);
    setenv("no_proxy", "*", 1);
    VolcengineTos::InitializeClient();
    DnsServer dns;
    ASSERT_TRUE(dns.ready());
    VolcengineTos::ClientConfig config;
    config.dnsCacheTime = 1;
    config.maxRetryCount = 0;
    config.isCustomDomain = true;
    config.endPoint = "http://fsx-dns-master-e2e.invalid";
    VolcengineTos::TosClientV2 client("cn-test", VolcengineTos::StaticCredentials(), config);
    const VolcengineTos::HeadBucketV2Input input("test-bucket");

    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        const auto first = client.headBucket(input);
        ASSERT_TRUE(first.isSuccess()) << first.error().String();
        EXPECT_NE(server.received().find("HEAD "), std::string::npos);
        EXPECT_NE(server.received().find("Host: fsx-dns-master-e2e.invalid"), std::string::npos);
    }

    // A refused connection invalidates libcurl's cached DNS entry, not the
    // independently retained last successful IP.
    EXPECT_FALSE(client.headBucket(input).isSuccess());
    dns.setFailure(true);
    const int queriesBeforeFallback = dns.queries();
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        const auto fallback = client.headBucket(input);
        ASSERT_TRUE(fallback.isSuccess()) << fallback.error().String();
        EXPECT_NE(server.received().find("HEAD "), std::string::npos);
        EXPECT_NE(server.received().find("Host: fsx-dns-master-e2e.invalid"), std::string::npos);
    }
    EXPECT_GT(dns.queries(), queriesBeforeFallback);

    // The successful fallback can populate libcurl's shared DNS entry again.
    // First make a refused connection evict that entry, then fail the fallback.
    EXPECT_FALSE(client.headBucket(input).isSuccess());
    const auto failedFallback = client.headBucket(input);
    ASSERT_FALSE(failedFallback.isSuccess());
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        const auto cooling = client.headBucket(input);
        ASSERT_FALSE(cooling.isSuccess());
        EXPECT_FALSE(server.accepted());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        const auto retried = client.headBucket(input);
        ASSERT_TRUE(retried.isSuccess()) << retried.error().String();
    }

    dns.setFailure(false);
    VolcengineTos::ClientConfig dropConfig = config;
    dropConfig.endPoint = "http://fsx-dns-master-e2e-drop.invalid";
    VolcengineTos::TosClientV2 dropClient("cn-test", VolcengineTos::StaticCredentials(), dropConfig);
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        ASSERT_TRUE(dropClient.headBucket(input).isSuccess());
    }
    EXPECT_FALSE(dropClient.headBucket(input).isSuccess());
    dns.setDrop(true);
    const int queriesBeforeDrop = dns.queries();
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        const auto timeoutFallback = dropClient.headBucket(input);
        ASSERT_TRUE(timeoutFallback.isSuccess()) << timeoutFallback.error().String();
    }
    EXPECT_GT(dns.queries(), queriesBeforeDrop);
    dns.setDrop(false);
    dns.setFailure(true);

    VolcengineTos::TosClientV2 fresh("cn-test", VolcengineTos::StaticCredentials(), config);
    const auto noLastGood = fresh.headBucket(input);
    ASSERT_FALSE(noLastGood.isSuccess());
    VolcengineTos::ClientConfig disabledConfig = config;
    disabledConfig.dnsCacheTime = 0;
    VolcengineTos::TosClientV2 disabled("cn-test", VolcengineTos::StaticCredentials(), disabledConfig);
    EXPECT_FALSE(disabled.headBucket(input).isSuccess());
}

#ifdef VOLCENGINE_TOS_ASYNC_TEST
TEST(AsyncDnsFallbackCacheTest, RefreshAndBoundedEviction) {
    VolcengineTos::AsyncDnsFallbackCache cache;
    cache.remember("old.invalid", "127.0.0.1");
    cache.scheduleRemoval("old.invalid");
    EXPECT_TRUE(cache.takeRemoval("old.invalid"));
    EXPECT_FALSE(cache.takeRemoval("old.invalid"));
    cache.coolDownIfMatches("old.invalid", "127.0.0.1");
    EXPECT_TRUE(cache.lookup("old.invalid").empty());
    cache.remember("old.invalid", "127.0.0.2");
    EXPECT_EQ(cache.lookup("old.invalid"), "127.0.0.2");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    for (int i = 0; i < 512; ++i)
        cache.remember("host-" + std::to_string(i) + ".invalid", "127.0.0.1");
    EXPECT_TRUE(cache.lookup("old.invalid").empty());
    EXPECT_EQ(cache.lookup("host-511.invalid"), "127.0.0.1");
}

struct AsyncHeadResult {
    std::mutex mutex;
    std::condition_variable ready;
    int calls = 0;
    bool success = false;
    int curl_error = 0;

    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        ASSERT_TRUE(ready.wait_for(lock, std::chrono::seconds(15), [this] { return calls > 0; }));
        EXPECT_EQ(calls, 1);
    }
};

std::shared_ptr<AsyncHeadResult> asyncHead(VolcengineTos::TosAsyncClient& client) {
    auto result = std::make_shared<AsyncHeadResult>();
    client.headBucketAsync(VolcengineTos::HeadBucketAsyncInput("test-bucket"), [result](auto& outcome) {
        std::lock_guard<std::mutex> lock(result->mutex);
        ++result->calls;
        result->success = outcome.isSuccess();
        if (!result->success) result->curl_error = outcome.error().getCurlErrCode();
        result->ready.notify_all();
    });
    result->wait();
    return result;
}

TEST(DnsFallbackEndToEndTest, PublicAsyncHeadBucketUsesLastGoodAfterRealDnsFailure) {
    if (std::getenv("FSX_DNS_REAL_TEST") == nullptr) return;
    VolcengineTos::Logger::getInstance().setAsyncLogLevel(VolcengineTos::ERROR);
    setenv("NO_PROXY", "*", 1);
    setenv("no_proxy", "*", 1);
    DnsServer dns;
    ASSERT_TRUE(dns.ready());
    VolcengineTos::ClientConfig config;
    config.dnsCacheTime = 1;
    config.maxRetryCount = 0;
    config.requestTimeout = 10000;
    config.connectionTimeout = 2000;
    config.event_thread_count_ = 1;
    config.detail_log_ = false;
    config.isCustomDomain = true;
    config.endPoint = "http://fsx-async-isolated.invalid";
    VolcengineTos::TosAsyncClient isolated("cn-test", VolcengineTos::StaticCredentials(), config);
    config.endPoint = "http://fsx-async-failure.invalid";
    VolcengineTos::TosAsyncClient failing("cn-test", VolcengineTos::StaticCredentials(), config);
    config.endPoint = "http://fsx-async-drop.invalid";
    VolcengineTos::TosAsyncClient dropping("cn-test", VolcengineTos::StaticCredentials(), config);
    config.async_transport_mode_ = VolcengineTos::AsyncTransportMode::Shared;
    config.endPoint = "http://fsx-async-shared.invalid";
    VolcengineTos::TosAsyncClient shared("cn-test", VolcengineTos::StaticCredentials(), config);

    std::vector<std::shared_ptr<AsyncHeadResult>> results;
    for (auto* client : {&isolated, &failing, &dropping, &shared}) {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        auto result = asyncHead(*client);
        ASSERT_TRUE(result->success);
        EXPECT_NE(server.received().find("HEAD "), std::string::npos);
        EXPECT_NE(server.received().find("Host: fsx-async-"), std::string::npos);
        results.push_back(result);
    }

    // A refused connection schedules removal of libcurl's DNS entry before
    // the next transfer, while the independently retained last-good IP stays.
    for (auto* client : {&isolated, &failing, &dropping, &shared}) {
        auto closed = asyncHead(*client);
        EXPECT_FALSE(closed->success);
        results.push_back(closed);
    }
    dns.setFailure(true);
    const int queriesBeforeFallback = dns.queries();
    for (auto* client : {&isolated, &shared}) {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        auto result = asyncHead(*client);
        ASSERT_TRUE(result->success) << result->curl_error;
        EXPECT_NE(server.received().find("HEAD "), std::string::npos);
        EXPECT_NE(server.received().find("Host: fsx-async-"), std::string::npos);
        results.push_back(result);
    }
    EXPECT_GT(dns.queries(), queriesBeforeFallback);

    auto failed = asyncHead(failing);
    EXPECT_FALSE(failed->success);
    results.push_back(failed);
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        auto cooling = asyncHead(failing);
        EXPECT_FALSE(cooling->success);
        EXPECT_FALSE(server.accepted());
        results.push_back(cooling);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        auto recovered = asyncHead(failing);
        EXPECT_TRUE(recovered->success) << recovered->curl_error;
        results.push_back(recovered);
    }

    dns.setFailure(false);
    dns.setDrop(true);
    const int queriesBeforeDrop = dns.queries();
    {
        OneShotHttpServer server;
        ASSERT_TRUE(server.ready());
        auto dropped = asyncHead(dropping);
        EXPECT_TRUE(dropped->success) << dropped->curl_error;
        EXPECT_NE(server.received().find("HEAD "), std::string::npos);
        results.push_back(dropped);
    }
    EXPECT_GT(dns.queries(), queriesBeforeDrop);
    dns.setDrop(false);
    dns.setFailure(true);

    config.endPoint = "http://fsx-async-no-history.invalid";
    VolcengineTos::TosAsyncClient fresh("cn-test", VolcengineTos::StaticCredentials(), config);
    auto missing = asyncHead(fresh);
    EXPECT_FALSE(missing->success);
    results.push_back(missing);
    config.dnsCacheTime = 0;
    VolcengineTos::TosAsyncClient disabled("cn-test", VolcengineTos::StaticCredentials(), config);
    auto off = asyncHead(disabled);
    EXPECT_FALSE(off->success);
    results.push_back(off);
    for (const auto& result : results) {
        std::lock_guard<std::mutex> lock(result->mutex);
        EXPECT_EQ(result->calls, 1);
    }
}
#endif
#endif
