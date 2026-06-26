#include <gtest/gtest.h>
#include <chrono>
#include "transport/http/HttpClient.h"

using namespace VolcengineTos;

namespace {
class TestHttpClient : public HttpClient {
   public:
    explicit TestHttpClient(const HttpConfig& config) : HttpClient(config) {}

    using HttpClient::buildResolveBinding;
    using HttpClient::releaseResolveBinding;
    using HttpClient::removeFailedIp;
};
}  // namespace

TEST(DnsIpBalancingTest, HostIpCacheSupportsRoundRobinAndExpiry) {
    HostIpCache cache;
    cache.Put("tos-cn-beijing.volces.com", {"192.0.2.1", "192.0.2.2"},
              std::chrono::steady_clock::now() + std::chrono::minutes(1));

    auto ipList = cache.Get("tos-cn-beijing.volces.com");
    ASSERT_EQ(ipList.size(), 2);
    auto first = cache.Select("tos-cn-beijing.volces.com");
    auto second = cache.Select("tos-cn-beijing.volces.com");
    auto third = cache.Select("tos-cn-beijing.volces.com");
    EXPECT_NE(first, second);
    EXPECT_EQ(first, third);
    EXPECT_TRUE(first == "192.0.2.1" || first == "192.0.2.2");
    EXPECT_TRUE(second == "192.0.2.1" || second == "192.0.2.2");

    cache.Put("expired.volces.com", {"198.51.100.10"}, std::chrono::steady_clock::now() - std::chrono::seconds(1));
    EXPECT_TRUE(cache.Get("expired.volces.com").empty());
}

TEST(DnsIpBalancingTest, HostIpCacheRandomizesInitialOffsetAcrossCaches) {
    HostIpCache firstCache;
    HostIpCache secondCache;
    const std::vector<std::string> ipList{"192.0.2.1", "192.0.2.2"};
    const auto expireAt = std::chrono::steady_clock::now() + std::chrono::minutes(1);

    firstCache.Put("same-host.example.com", ipList, expireAt);
    secondCache.Put("same-host.example.com", ipList, expireAt);

    auto firstPick = firstCache.Select("same-host.example.com");
    auto secondPick = secondCache.Select("same-host.example.com");
    EXPECT_NE(firstPick, secondPick);
}

TEST(DnsIpBalancingTest, HostIpCacheRemovesFailedIpAndClearsExhaustedHost) {
    HostIpCache cache;
    cache.Put("multi-ip.volces.com", {"192.0.2.10", "192.0.2.11"},
              std::chrono::steady_clock::now() + std::chrono::minutes(1));

    EXPECT_TRUE(cache.RemoveIp("multi-ip.volces.com", "192.0.2.10"));
    auto ipList = cache.Get("multi-ip.volces.com");
    ASSERT_EQ(ipList.size(), 1);
    EXPECT_EQ(ipList.front(), "192.0.2.11");

    EXPECT_TRUE(cache.RemoveIp("multi-ip.volces.com", "192.0.2.11"));
    EXPECT_TRUE(cache.Get("multi-ip.volces.com").empty());
}

TEST(DnsIpBalancingTest, HostIpCacheEvictsLeastRecentlyUsedHostWhenCapacityExceeded) {
    HostIpCache cache(2);
    const auto expireAt = std::chrono::steady_clock::now() + std::chrono::minutes(1);

    cache.Put("host-a.example.com", {"192.0.2.1"}, expireAt);
    cache.Put("host-b.example.com", {"192.0.2.2"}, expireAt);
    ASSERT_EQ(cache.Size(), 2U);

    ASSERT_EQ(cache.Get("host-a.example.com").size(), 1U);
    cache.Put("host-c.example.com", {"192.0.2.3"}, expireAt);

    EXPECT_EQ(cache.Size(), 2U);
    EXPECT_TRUE(cache.Get("host-b.example.com").empty());
    EXPECT_EQ(cache.Get("host-a.example.com").size(), 1U);
    EXPECT_EQ(cache.Get("host-c.example.com").size(), 1U);
}

TEST(DnsIpBalancingTest, HttpClientBuildResolveBindingPreservesHostAndRotatesIp) {
    HttpConfig config{};
    config.maxConnections = 2;
    config.socketTimeout = 1000;
    config.requestTimeout = 0;
    config.dialTimeout = 1;
    config.tcpKeepAlive = 1;
    config.connectTimeout = 1000;
    config.enableVerifySSL = true;
    config.proxyPort = -1;
    config.dnsCacheTime = 1;
    config.enableDnsIpBalancing = true;
    config.dnsLookupCallback = [](const std::string&) { return std::vector<std::string>{"192.0.2.1", "192.0.2.2"}; };
    TestHttpClient client(config);

    auto request = std::make_shared<HttpRequest>(http::MethodGet);
    Url url;
    url.setScheme("https");
    url.setHost("custom.example.com");
    url.setPath("/resource");
    request->setUrl(url);

    auto first = client.buildResolveBinding(request);
    ASSERT_TRUE(first.active());
    EXPECT_EQ(first.host, "custom.example.com");
    EXPECT_EQ(first.port, "443");
    EXPECT_EQ(request->url().host(), "custom.example.com");
    EXPECT_TRUE(first.selectedIp == "192.0.2.1" || first.selectedIp == "192.0.2.2");

    auto second = client.buildResolveBinding(request);
    ASSERT_TRUE(second.active());
    EXPECT_NE(second.selectedIp, first.selectedIp);

    client.releaseResolveBinding(first);
    client.releaseResolveBinding(second);
}

TEST(DnsIpBalancingTest, HttpClientRetriesWithAnotherIpAndReResolvesAfterExhaustion) {
    HttpConfig config{};
    config.maxConnections = 2;
    config.socketTimeout = 1000;
    config.requestTimeout = 0;
    config.dialTimeout = 1;
    config.tcpKeepAlive = 1;
    config.connectTimeout = 1000;
    config.enableVerifySSL = true;
    config.proxyPort = -1;
    config.dnsCacheTime = 1;
    config.enableDnsIpBalancing = true;

    int lookupCount = 0;
    config.dnsLookupCallback = [&lookupCount](const std::string&) {
        ++lookupCount;
        return std::vector<std::string>{"198.51.100.1", "198.51.100.2"};
    };
    TestHttpClient client(config);

    auto request = std::make_shared<HttpRequest>(http::MethodGet);
    Url url;
    url.setScheme("https");
    url.setHost("retry.example.com");
    url.setPath("/object");
    request->setUrl(url);

    auto first = client.buildResolveBinding(request);
    ASSERT_TRUE(first.active());
    EXPECT_TRUE(first.selectedIp == "198.51.100.1" || first.selectedIp == "198.51.100.2");
    EXPECT_EQ(lookupCount, 1);
    client.removeFailedIp(first);
    client.releaseResolveBinding(first);

    auto second = client.buildResolveBinding(request);
    ASSERT_TRUE(second.active());
    EXPECT_NE(second.selectedIp, first.selectedIp);
    EXPECT_EQ(lookupCount, 1);
    client.removeFailedIp(second);
    client.releaseResolveBinding(second);

    auto third = client.buildResolveBinding(request);
    ASSERT_TRUE(third.active());
    EXPECT_TRUE(third.selectedIp == "198.51.100.1" || third.selectedIp == "198.51.100.2");
    EXPECT_EQ(lookupCount, 2);
    client.releaseResolveBinding(third);
}

TEST(DnsIpBalancingTest, HttpClientFormatsIpv6ResolveEntry) {
    HttpConfig config{};
    config.maxConnections = 1;
    config.socketTimeout = 1000;
    config.requestTimeout = 0;
    config.dialTimeout = 1;
    config.tcpKeepAlive = 1;
    config.connectTimeout = 1000;
    config.enableVerifySSL = true;
    config.proxyPort = -1;
    config.dnsCacheTime = 1;
    config.enableDnsIpBalancing = true;
    config.dnsLookupCallback = [](const std::string&) { return std::vector<std::string>{"2001:db8::1"}; };
    TestHttpClient client(config);

    auto request = std::make_shared<HttpRequest>(http::MethodGet);
    Url url;
    url.setScheme("https");
    url.setHost("ipv6.example.com");
    url.setPath("/resource");
    request->setUrl(url);

    auto binding = client.buildResolveBinding(request);
    ASSERT_TRUE(binding.active());
    ASSERT_NE(binding.resolveList, nullptr);
    EXPECT_STREQ(binding.resolveList->data, "ipv6.example.com:443:[2001:db8::1]");
    client.releaseResolveBinding(binding);
}

TEST(DnsIpBalancingTest, HttpClientSkipsSdkBalancingWhenFlagDisabled) {
    HttpConfig config{};
    config.maxConnections = 1;
    config.socketTimeout = 1000;
    config.requestTimeout = 0;
    config.dialTimeout = 1;
    config.tcpKeepAlive = 1;
    config.connectTimeout = 1000;
    config.enableVerifySSL = true;
    config.proxyPort = -1;
    config.dnsCacheTime = 1;
    config.enableDnsIpBalancing = false;

    int lookupCount = 0;
    config.dnsLookupCallback = [&lookupCount](const std::string&) {
        ++lookupCount;
        return std::vector<std::string>{"203.0.113.1", "203.0.113.2"};
    };
    TestHttpClient client(config);

    auto request = std::make_shared<HttpRequest>(http::MethodGet);
    Url url;
    url.setScheme("https");
    url.setHost("flag-disabled.example.com");
    url.setPath("/resource");
    request->setUrl(url);

    auto binding = client.buildResolveBinding(request);
    EXPECT_FALSE(binding.active());
    EXPECT_EQ(lookupCount, 0);
}

TEST(DnsIpBalancingTest, HttpClientSkipsSdkBalancingWhenDnsCacheDisabled) {
    HttpConfig config{};
    config.maxConnections = 1;
    config.socketTimeout = 1000;
    config.requestTimeout = 0;
    config.dialTimeout = 1;
    config.tcpKeepAlive = 1;
    config.connectTimeout = 1000;
    config.enableVerifySSL = true;
    config.proxyPort = -1;
    config.dnsCacheTime = 0;

    int lookupCount = 0;
    config.dnsLookupCallback = [&lookupCount](const std::string&) {
        ++lookupCount;
        return std::vector<std::string>{"203.0.113.1", "203.0.113.2"};
    };
    TestHttpClient client(config);

    auto request = std::make_shared<HttpRequest>(http::MethodGet);
    Url url;
    url.setScheme("https");
    url.setHost("disabled.example.com");
    url.setPath("/resource");
    request->setUrl(url);

    auto binding = client.buildResolveBinding(request);
    EXPECT_FALSE(binding.active());
    EXPECT_EQ(lookupCount, 0);
}
