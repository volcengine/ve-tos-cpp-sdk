#include <gtest/gtest.h>

#if defined(__linux__) && GTEST_HAS_DEATH_TEST
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "TosClientV2.h"
#include "spdlog/sinks/ostream_sink.h"
#include "transport/http/HttpClient.h"
#include "utils/LogUtils.h"

using namespace VolcengineTos;

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class LocalServer {
public:
    LocalServer(bool slow, int requests, int status, bool refuse) {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        require(fd_ >= 0, "socket failed");
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
            close(fd_);
            throw std::runtime_error("bind failed");
        }
        socklen_t length = sizeof(address);
        if (getsockname(fd_, reinterpret_cast<sockaddr*>(&address), &length) != 0 || (!refuse && listen(fd_, 1) != 0)) {
            close(fd_);
            throw std::runtime_error("listener setup failed");
        }
        port_ = ntohs(address.sin_port);
        if (refuse) {
            return;
        }
        worker_ = std::thread([this, slow, requests, status] {
            pollfd ready{fd_, POLLIN, 0};
            if (poll(&ready, 1, 3000) <= 0) {
                return;
            }
            sockaddr_in address{};
            socklen_t length = sizeof(address);
            const int peer = accept(fd_, reinterpret_cast<sockaddr*>(&address), &length);
            if (peer < 0) {
                return;
            }
            timeval timeout{3, 0};
            setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            for (int request = 0; request < requests; ++request) {
                std::string headers;
                char buffer[4096];
                while (headers.find("\r\n\r\n") == std::string::npos && headers.size() < 16384) {
                    const auto count = recv(peer, buffer, sizeof(buffer), 0);
                    if (count <= 0) {
                        close(peer);
                        return;
                    }
                    headers.append(buffer, static_cast<size_t>(count));
                }
                ports_.push_back(ntohs(address.sin_port));
                if (slow) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(650));
                }
                const auto response = "HTTP/1.1 " + std::to_string(status) + " Test\r\nContent-Length: 2\r\n\r\nok";
                size_t sent = 0;
                while (sent < response.size()) {
                    const auto count = send(peer, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
                    if (count <= 0) {
                        close(peer);
                        return;
                    }
                    sent += static_cast<size_t>(count);
                }
            }
            close(peer);
        });
    }

    ~LocalServer() {
        shutdown(fd_, SHUT_RDWR);
        finish();
        close(fd_);
    }

    void finish() {
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    int port() const {
        return port_;
    }

    const std::vector<int>& observedPorts() const {
        return ports_;
    }

private:
    int fd_ = -1;
    int port_ = 0;
    std::thread worker_;
    std::vector<int> ports_;
};

class CaptureStdout {
public:
    CaptureStdout() : previous_(std::cout.rdbuf(output.rdbuf())) {
    }
    ~CaptureStdout() {
        std::cout.rdbuf(previous_);
    }
    std::ostringstream output;

private:
    std::streambuf* previous_;
};

void runCase(int logLevel, bool slow, bool expectLog, int requests = 1, bool refuse = false, int status = 200) {
    for (const char* name : {"http_proxy", "https_proxy", "all_proxy", "HTTP_PROXY", "HTTPS_PROXY", "ALL_PROXY"}) {
        unsetenv(name);
    }
    setenv("NO_PROXY", "127.0.0.1", 1);
    setenv("no_proxy", "127.0.0.1", 1);
    std::ostringstream output;
    const auto path = "/tmp/tos-http-timing-log-" + std::to_string(getpid());
    if (logLevel >= 0) {
        LogUtils::SetLogger(path, "timing-test", static_cast<LogLevel>(logLevel));
        auto logger = LogUtils::GetLogger();
        logger->sinks().clear();
        logger->sinks().push_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(output));
        logger->set_pattern("%l %v");
    }
    CaptureStdout captured;
    InitializeClient();
    LocalServer server(slow, requests, status, refuse);
    HttpConfig config{};
    config.maxConnections = 1;
    config.socketTimeout = 3000;
    config.connectTimeout = 1000;
    config.requestTimeout = 3000;
    config.proxyPort = -1;
    config.highLatencyLogThreshold = 100;
    {
        HttpClient client(config);
        for (int index = 0; index < requests; ++index) {
            auto request = std::make_shared<HttpRequest>();
            request->setUrl(Url("http://127.0.0.1:" + std::to_string(server.port()) + "/timing-test"));
            request->setCheckHighLatency(slow);
            const auto response = client.doRequest(request);
            require(response->getCurlErrCode() == (refuse ? CURLE_COULDNT_CONNECT : CURLE_OK), "curl result changed");
            if (!refuse) {
                require(response->statusCode() == status, "HTTP status changed");
                require(response->isHighLatencyReq() == slow, "slow-request classification changed");
                require(response->Body() != nullptr, "missing response body");
                const std::string body(std::istreambuf_iterator<char>(*response->Body()),
                                       std::istreambuf_iterator<char>());
                require(body == "ok", "response changed");
            }
        }
    }
    server.finish();
    CloseClient();
    const auto actual = logLevel < 0 ? captured.output.str() : output.str();
    if (logLevel >= 0) {
        require(captured.output.str().empty(), "configured logger unexpectedly fell back to stdout");
        LogUtils::GetLogger()->flush();
        std::remove(path.c_str());
    }
    if (!expectLog) {
        require(actual.find("Local Port:") == std::string::npos, "log level unexpectedly emitted port");
        require(actual.find("Total HTTP request time:") == std::string::npos, "log level behavior changed");
        return;
    }
    if (refuse) {
        require(actual.find("Local Port:-1") != std::string::npos, "missing unavailable-port marker");
        return;
    }
    const auto& observed = server.observedPorts();
    require(observed.size() == static_cast<size_t>(requests), "server missed requests");
    size_t offset = 0;
    for (const auto port : observed) {
        require(port > 0 && port <= 65535, "invalid accepted source port");
        offset = actual.find("Local Port:" + std::to_string(port) + "\n", offset);
        require(offset != std::string::npos, "logged port does not match socket peer port");
        ++offset;
    }
    if (requests > 1) {
        require(observed.front() == observed.back(), "connection reuse changed source port");
    }
    if (logLevel >= 0) {
        require(actual.find(slow ? "warning Method:" : "debug Method:") != std::string::npos,
                "timing log severity changed");
    }
}

void probe(int level, bool slow, bool expectLog, int requests = 1, bool refuse = false, int status = 200) {
    try {
        runCase(level, slow, expectLog, requests, refuse, status);
        std::_Exit(0);
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        std::_Exit(1);
    }
}

// Each subprocess starts with an untouched process-wide logger singleton.
TEST(HttpTimingLogDeathTest, SlowRequestWithoutLoggerPrintsPortToStdout) {
    ASSERT_EXIT(probe(-1, true, true), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, OrdinaryRequestWithoutLoggerRemainsSilent) {
    ASSERT_EXIT(probe(-1, false, false), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, DebugLogsOrdinaryRequestPort) {
    ASSERT_EXIT(probe(LogDebug, false, true), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, InfoDoesNotLogOrdinaryRequestTiming) {
    ASSERT_EXIT(probe(LogInfo, false, false), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, InfoLogsSlowRequestPortAtWarning) {
    ASSERT_EXIT(probe(LogInfo, true, true), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, ExplicitLogOffSuppressesSlowRequestOutput) {
    ASSERT_EXIT(probe(LogOff, true, false), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, ReusedConnectionLogsSameActualPort) {
    ASSERT_EXIT(probe(LogDebug, false, true, 2), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, ConnectionFailureLogsUnavailablePort) {
    ASSERT_EXIT(probe(LogDebug, false, true, 1, true), ::testing::ExitedWithCode(0), "");
}

TEST(HttpTimingLogDeathTest, HttpErrorStillLogsEstablishedConnectionPort) {
    ASSERT_EXIT(probe(LogDebug, false, true, 1, false, 503), ::testing::ExitedWithCode(0), "");
}
}  // namespace
#else
TEST(HttpTimingLogTest, LoopbackSubprocessChecksRequireLinux) {
    SUCCEED();
}
#endif
