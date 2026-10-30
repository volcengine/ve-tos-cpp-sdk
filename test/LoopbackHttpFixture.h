#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace tos_test {

// A finite, test-only HTTP/1.1 script server. It binds only 127.0.0.1:0,
// accepts one request per connection, and never logs request/header values.
// HEAD and GET require no request-body reader; every response closes its socket.
// This fixture reads no environment variables, SDK configuration or credentials.
class LoopbackHttpFixture {
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::milliseconds;

public:
    static constexpr std::size_t kMaxSteps = 64;
    static constexpr std::size_t kMaxHeaderBytes = 64 * 1024;
    static constexpr std::size_t kMaxScriptBytes = 16 * 1024 * 1024;

    class Gate {
    public:
        bool WaitUntilReached(Milliseconds timeout = Milliseconds(5000)) {
            ValidateTimeout(timeout);
            std::unique_lock<std::mutex> lock(mutex_);
            changed_.wait_for(lock, timeout, [&] { return reached_ || cancelled_; });
            return reached_;
        }
        void Release() {
            std::lock_guard<std::mutex> lock(mutex_);
            released_ = true;
            changed_.notify_all();
        }

    private:
        friend class LoopbackHttpFixture;
        bool Block(const std::atomic<bool>& stopping) {
            std::unique_lock<std::mutex> lock(mutex_);
            reached_ = true;
            changed_.notify_all();
            changed_.wait_until(lock, Clock::now() + Milliseconds(5000),
                                [&] { return released_ || cancelled_ || stopping.load(); });
            return released_ && !cancelled_ && !stopping.load();
        }
        void Cancel() {
            std::lock_guard<std::mutex> lock(mutex_);
            cancelled_ = true;
            changed_.notify_all();
        }
        std::mutex mutex_;
        std::condition_variable changed_;
        bool reached_{false};
        bool released_{false};
        bool cancelled_{false};
    };

    struct Request {
        std::string method;
        std::string target;
        // Lowercase names. Authorization/proxy-authorization/session tokens
        // are replaced with <redacted> before entering this recorded snapshot.
        std::map<std::string, std::string> headers;
    };

    enum class BodyFraming { ContentLength, CloseDelimited, Chunked };

    struct Response {
        int status{200};
        std::string reason{"OK"};
        std::map<std::string, std::string> headers;
        std::string body;
        std::optional<std::size_t> declared_content_length;
        bool disconnect{false};
        // Set only when a timeout/cancellation test intentionally closes its
        // client socket before releasing the response gate.
        bool allow_peer_disconnect{false};
        std::shared_ptr<Gate> gate;
        BodyFraming framing{BodyFraming::ContentLength};
    };

    struct Step {
        std::string method;
        std::string target;
        Response response;
    };

    LoopbackHttpFixture(std::initializer_list<Step> steps) : LoopbackHttpFixture(std::vector<Step>(steps)) {
    }

    explicit LoopbackHttpFixture(std::vector<Step> steps) : steps_(std::move(steps)) {
        ValidateScript();
        requests_.reserve(steps_.size());
        error_.reserve(256);
        int wake[2];
        if (::pipe(wake) != 0)
            SystemFailure("pipe");
        wake_read_.Reset(wake[0]);
        wake_write_.Reset(wake[1]);
        ConfigureFd(wake_read_.Get());
        ConfigureFd(wake_write_.Get());

        listener_.Reset(::socket(AF_INET, SOCK_STREAM, 0));
        if (listener_.Get() < 0)
            SystemFailure("socket");
        ConfigureFd(listener_.Get());
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (::bind(listener_.Get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
            SystemFailure("bind loopback");
        socklen_t length = sizeof(address);
        if (::getsockname(listener_.Get(), reinterpret_cast<sockaddr*>(&address), &length) != 0)
            SystemFailure("getsockname");
        if (address.sin_addr.s_addr != htonl(INADDR_LOOPBACK) || address.sin_port == 0)
            throw std::runtime_error("fixture did not bind a loopback ephemeral port");
        if (::listen(listener_.Get(), static_cast<int>(kMaxSteps)) != 0)
            SystemFailure("listen");
        endpoint_ = "http://127.0.0.1:" + std::to_string(ntohs(address.sin_port));
        worker_ = std::thread([this] { Serve(); });
    }

    ~LoopbackHttpFixture() {
        stopping_.store(true);
        for (const auto& step : steps_)
            if (step.response.gate)
                step.response.gate->Cancel();
        // The nonblocking pipe wakes accept/recv/send poll without closing an
        // fd concurrently with the worker. An already full pipe is also awake.
        const char wake = 1;
        while (::write(wake_write_.Get(), &wake, 1) < 0 && errno == EINTR) {
        }
        if (worker_.joinable())
            worker_.join();
    }

    LoopbackHttpFixture(const LoopbackHttpFixture&) = delete;
    LoopbackHttpFixture& operator=(const LoopbackHttpFixture&) = delete;

    const std::string& Endpoint() const {
        return endpoint_;
    }

    bool WaitForRequests(std::size_t count, Milliseconds timeout = Milliseconds(5000)) {
        ValidateTimeout(timeout);
        if (count > steps_.size())
            throw std::invalid_argument("request count exceeds finite script");
        std::unique_lock<std::mutex> lock(mutex_);
        changed_.wait_for(lock, timeout, [&] { return requests_.size() >= count || done_; });
        return requests_.size() >= count;
    }

    // True means the worker has ended; use Error()/AssertDone() to distinguish
    // successful script completion from a failed request or network deadline.
    bool WaitUntilDone(Milliseconds timeout = Milliseconds(5000)) {
        ValidateTimeout(timeout);
        std::unique_lock<std::mutex> lock(mutex_);
        return changed_.wait_for(lock, timeout, [&] { return done_; });
    }

    std::vector<Request> Requests() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return requests_;
    }

    std::string Error() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return error_;
    }

    void AssertDone() {
        if (!WaitUntilDone())
            throw std::runtime_error("loopback fixture completion deadline exceeded");
        std::lock_guard<std::mutex> lock(mutex_);
        if (!error_.empty())
            throw std::runtime_error(error_);
        if (completed_steps_ != steps_.size())
            throw std::runtime_error("loopback script incomplete");
    }

private:
    class Fd {
    public:
        explicit Fd(int value = -1) : value_(value) {
        }
        ~Fd() {
            Reset();
        }
        Fd(const Fd&) = delete;
        Fd& operator=(const Fd&) = delete;
        int Get() const {
            return value_;
        }
        void Reset(int value = -1) noexcept {
            if (value_ >= 0)
                ::close(value_);
            value_ = value;
        }

    private:
        int value_;
    };

    static void ValidateTimeout(Milliseconds timeout) {
        if (timeout.count() < 0 || timeout > Milliseconds(5000))
            throw std::invalid_argument("fixture timeout must be between zero and five seconds");
    }

    [[noreturn]] static void SystemFailure(const char* operation) {
        const int error = errno;
        throw std::runtime_error(std::string("loopback fixture ") + operation +
                                 " failed (errno=" + std::to_string(error) + ")");
    }

    static void ConfigureFd(int fd) {
        const int flags = ::fcntl(fd, F_GETFL);
        if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0 || ::fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
            SystemFailure("configure nonblocking fd");
    }

    static std::string Lower(std::string_view value) {
        std::string result(value);
        for (auto& ch : result)
            if (ch >= 'A' && ch <= 'Z')
                ch = static_cast<char>(ch + ('a' - 'A'));
        return result;
    }

    static bool Token(std::string_view value) {
        if (value.empty())
            return false;
        for (const char ch : value) {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
                continue;
            if (std::string_view("!#$%&'*+-.^_`|~").find(ch) == std::string_view::npos)
                return false;
        }
        return true;
    }

    static bool CleanLine(std::string_view value) {
        return value.find_first_of("\r\n") == std::string_view::npos && value.find('\0') == std::string_view::npos;
    }

    static std::string_view Trim(std::string_view value) {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            value.remove_prefix(1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
            value.remove_suffix(1);
        return value;
    }

    static void AppendHead(std::string& head, std::string_view bytes) {
        if (bytes.size() > kMaxHeaderBytes - head.size())
            throw std::invalid_argument("scripted response header too large");
        head.append(bytes.data(), bytes.size());
    }

    std::string ResponseHead(const Response& response) const {
        std::string head = "HTTP/1.1 " + std::to_string(response.status) + " ";
        AppendHead(head, response.reason);
        AppendHead(head, "\r\n");
        std::map<std::string, bool> names;
        for (const auto& item : response.headers) {
            if (item.first.size() > kMaxHeaderBytes || item.second.size() > kMaxHeaderBytes)
                throw std::invalid_argument("scripted response header too large");
            const auto name = Lower(item.first);
            if (!Token(item.first) || !CleanLine(item.second) || !names.emplace(name, true).second)
                throw std::invalid_argument("invalid or duplicate scripted response header");
            if (name == "connection" && Lower(Trim(item.second)) != "close")
                throw std::invalid_argument("fixture responses require Connection: close");
            if (name == "content-length" && response.declared_content_length)
                throw std::invalid_argument("two scripted Content-Length sources");
            if (name == "content-length" && response.framing != BodyFraming::ContentLength)
                throw std::invalid_argument("Content-Length conflicts with scripted body framing");
            if (name == "transfer-encoding" &&
                (response.framing != BodyFraming::Chunked || Lower(Trim(item.second)) != "chunked"))
                throw std::invalid_argument("Transfer-Encoding conflicts with scripted body framing");
            AppendHead(head, item.first);
            AppendHead(head, ": ");
            AppendHead(head, item.second);
            AppendHead(head, "\r\n");
        }
        if (response.declared_content_length && response.framing != BodyFraming::ContentLength)
            throw std::invalid_argument("declared Content-Length conflicts with scripted body framing");
        if (response.framing == BodyFraming::ContentLength && !names.count("content-length")) {
            AppendHead(head, "Content-Length: ");
            AppendHead(head, std::to_string(response.declared_content_length.value_or(response.body.size())));
            AppendHead(head, "\r\n");
        }
        if (response.framing == BodyFraming::Chunked && !names.count("transfer-encoding"))
            AppendHead(head, "Transfer-Encoding: chunked\r\n");
        if (!names.count("connection"))
            AppendHead(head, "Connection: close\r\n");
        AppendHead(head, "\r\n");
        return head;
    }

    void ValidateScript() const {
        if (steps_.empty() || steps_.size() > kMaxSteps)
            throw std::invalid_argument("fixture script requires 1..64 steps");
        std::size_t bytes = 0;
        for (const auto& step : steps_) {
            if ((step.method != "HEAD" && step.method != "GET") || step.target.empty() || step.target.front() != '/' ||
                step.target.find(' ') != std::string::npos || !CleanLine(step.target) ||
                step.target.size() > kMaxHeaderBytes || step.response.status < 100 || step.response.status > 599 ||
                step.response.reason.size() > kMaxHeaderBytes || !CleanLine(step.response.reason))
                throw std::invalid_argument("invalid scripted request or response status");
            if (step.response.framing != BodyFraming::ContentLength &&
                step.response.framing != BodyFraming::CloseDelimited && step.response.framing != BodyFraming::Chunked)
                throw std::invalid_argument("invalid scripted body framing");
            const auto head = ResponseHead(step.response);
            const std::size_t framing_bytes =
                    step.response.framing == BodyFraming::Chunked ? 2 * sizeof(std::size_t) + 9 : 0;
            for (const auto size : {step.method.size(), step.target.size(), head.size(), step.response.body.size(),
                                   framing_bytes}) {
                if (size > kMaxScriptBytes - bytes)
                    throw std::invalid_argument("fixture script byte limit exceeded");
                bytes += size;
            }
        }
    }

    bool Poll(int fd, short events, Clock::time_point deadline) {
        while (!stopping_.load()) {
            const auto remaining = deadline - Clock::now();
            if (remaining <= Clock::duration::zero())
                throw std::runtime_error("loopback network deadline exceeded");
            const auto millis = std::chrono::duration_cast<Milliseconds>(remaining).count();
            pollfd fds[2]{{fd, events, 0}, {wake_read_.Get(), POLLIN, 0}};
            const int result = ::poll(fds, 2, static_cast<int>(std::min<int64_t>(millis + 1, 5000)));
            if (result < 0) {
                if (errno == EINTR)
                    continue;
                SystemFailure("poll");
            }
            if (stopping_.load() || fds[1].revents)
                return false;
            if (result == 0)
                continue;
            if (fds[0].revents & POLLNVAL)
                throw std::runtime_error("loopback poll saw an invalid fd");
            if (fds[0].revents & (events | POLLERR | POLLHUP))
                return true;
        }
        return false;
    }

    int Accept() {
        const auto deadline = Clock::now() + Milliseconds(5000);
        while (Poll(listener_.Get(), POLLIN, deadline)) {
            const int fd = ::accept(listener_.Get(), nullptr, nullptr);
            if (fd >= 0)
                return fd;
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                SystemFailure("accept");
        }
        return -1;
    }

    std::optional<Request> ReadRequest(int fd) {
        const auto deadline = Clock::now() + Milliseconds(5000);
        std::string bytes;
        std::size_t end = std::string::npos;
        while ((end = bytes.find("\r\n\r\n")) == std::string::npos) {
            if (bytes.size() >= kMaxHeaderBytes)
                throw std::runtime_error("request header exceeded 64 KiB");
            if (!Poll(fd, POLLIN, deadline))
                return std::nullopt;
            char buffer[4096];
            const auto received = ::recv(fd, buffer, std::min(sizeof(buffer), kMaxHeaderBytes - bytes.size()), 0);
            if (received == 0)
                throw std::runtime_error("client disconnected before complete request headers");
            if (received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                    continue;
                SystemFailure("receive request");
            }
            bytes.append(buffer, static_cast<std::size_t>(received));
        }
        const std::string_view header(bytes.data(), end + 2);
        const auto first_end = header.find("\r\n");
        const auto first = header.substr(0, first_end);
        const auto space = first.find(' ');
        const auto second_space = space == std::string_view::npos ? space : first.find(' ', space + 1);
        if (space == std::string_view::npos || second_space == std::string_view::npos ||
            first.substr(second_space + 1) != "HTTP/1.1")
            throw std::runtime_error("invalid HTTP/1.1 request line");
        Request request;
        request.method = std::string(first.substr(0, space));
        request.target = std::string(first.substr(space + 1, second_space - space - 1));
        if (!Token(request.method) || request.target.empty() || !CleanLine(request.target))
            throw std::runtime_error("invalid request method or target");
        for (std::size_t position = first_end + 2; position < header.size();) {
            const auto line_end = header.find("\r\n", position);
            if (line_end == std::string_view::npos)
                throw std::runtime_error("invalid request header terminator");
            const auto line = header.substr(position, line_end - position);
            const auto colon = line.find(':');
            if (colon == std::string_view::npos || !Token(line.substr(0, colon)))
                throw std::runtime_error("invalid request header name");
            auto name = Lower(line.substr(0, colon));
            const auto value = Trim(line.substr(colon + 1));
            if (!CleanLine(value))
                throw std::runtime_error("invalid request header value");
            const bool redact = name == "authorization" || name == "proxy-authorization" ||
                                name == "x-tos-security-token" || name == "x-amz-security-token";
            auto inserted = request.headers.emplace(name, redact ? "<redacted>" : std::string(value));
            if (!inserted.second && !redact)
                inserted.first->second += "," + std::string(value);
            position = line_end + 2;
        }
        return request;
    }

    bool Send(int fd, std::string_view bytes, Clock::time_point deadline, bool allow_peer_disconnect) {
        while (!bytes.empty()) {
            if (!Poll(fd, POLLOUT, deadline))
                return false;
            const auto sent = ::send(fd, bytes.data(), bytes.size(), MSG_NOSIGNAL);
            if (sent > 0) {
                bytes.remove_prefix(static_cast<std::size_t>(sent));
                continue;
            }
            if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
                continue;
            if (allow_peer_disconnect && (sent == 0 || errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN))
                return true;
            SystemFailure("send response");
        }
        return !stopping_.load();
    }

    void Serve() noexcept {
        try {
            for (std::size_t index = 0; index < steps_.size() && !stopping_.load(); ++index) {
                const auto& step = steps_[index];
                Fd client(Accept());
                if (client.Get() < 0)
                    break;
                ConfigureFd(client.Get());
                auto request = ReadRequest(client.Get());
                if (!request)
                    break;
                const bool matches = request->method == step.method && request->target == step.target;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    requests_.push_back(std::move(*request));
                    changed_.notify_all();
                }
                if (!matches)
                    throw std::runtime_error("request method/target did not match script step " +
                                             std::to_string(index));
                if (step.response.gate && !step.response.gate->Block(stopping_)) {
                    if (stopping_.load())
                        break;
                    throw std::runtime_error("loopback response gate deadline exceeded");
                }
                if (!step.response.disconnect) {
                    const auto head = ResponseHead(step.response);
                    const auto deadline = Clock::now() + Milliseconds(5000);
                    if (!Send(client.Get(), head, deadline, step.response.allow_peer_disconnect))
                        break;
                    if (step.method != "HEAD") {
                        if (step.response.framing == BodyFraming::Chunked) {
                            char hex_size[2 * sizeof(std::size_t)];
                            const auto encoded = std::to_chars(hex_size, hex_size + sizeof(hex_size),
                                                               step.response.body.size(), 16);
                            const std::string prefix = std::string(hex_size, encoded.ptr) + "\r\n";
                            const std::string suffix = step.response.body.empty() ? "\r\n" : "\r\n0\r\n\r\n";
                            if (!Send(client.Get(), prefix, deadline, step.response.allow_peer_disconnect) ||
                                !Send(client.Get(), step.response.body, deadline, step.response.allow_peer_disconnect) ||
                                !Send(client.Get(), suffix, deadline, step.response.allow_peer_disconnect))
                                break;
                        } else if (!Send(client.Get(), step.response.body, deadline, step.response.allow_peer_disconnect)) {
                            break;
                        }
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    ++completed_steps_;
                }
            }
        } catch (const std::exception& error) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!stopping_.load())
                error_.assign(error.what(), std::min<std::size_t>(255, std::char_traits<char>::length(error.what())));
        } catch (...) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!stopping_.load())
                error_ = "unexpected loopback server exception";
        }
        listener_.Reset();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_ = true;
            changed_.notify_all();
        }
    }

    const std::vector<Step> steps_;
    Fd listener_;
    Fd wake_read_;
    Fd wake_write_;
    std::string endpoint_;
    std::atomic<bool> stopping_{false};
    mutable std::mutex mutex_;
    std::condition_variable changed_;
    std::vector<Request> requests_;
    std::string error_;
    std::size_t completed_steps_{0};
    bool done_{false};
    std::thread worker_;
};

}  // namespace tos_test
