#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace tos_test {
// Bounded loopback-only load fixture. One poll thread, <=256 connections,
// <=200000 requests, streaming <=100 MiB per request, no object-size buffers.
// Supports Content-Length GET/PUT and connection reuse; never logs headers.
class AsyncLoadHttpFixture {
    using Clock = std::chrono::steady_clock;
    struct Connection {
        std::string head, response;
        bool parsed = false, put = false;
        size_t size = 0, received = 0, sent = 0, head_sent = 0;
        Clock::time_point ready;
    };

   public:
    explicit AsyncLoadHttpFixture(bool verify_uploads = true) : verify_(verify_uploads), block_(65536, 'x') {
        listener_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (listener_ < 0) throw std::runtime_error("load fixture socket");
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        socklen_t length = sizeof(addr);
        if (bind(listener_, reinterpret_cast<sockaddr*>(&addr), length) != 0 ||
            getsockname(listener_, reinterpret_cast<sockaddr*>(&addr), &length) != 0 || listen(listener_, 256) != 0) {
            close(listener_);
            throw std::runtime_error("load fixture bind/listen");
        }
        endpoint_ = "http://127.0.0.1:" + std::to_string(ntohs(addr.sin_port));
        thread_ = std::thread([this] {
            try {
                Run();
            } catch (...) {
                failed_ = true;
            }
        });
    }
    ~AsyncLoadHttpFixture() {
        stop_ = true;
        thread_.join();  // poll is bounded to 10 ms; never close a live poll fd.
        for (const auto& item : connections_) close(item.first);
        close(listener_);
    }
    const std::string& Endpoint() const {
        return endpoint_;
    }
    void Check() const {
        if (failed_ || bad_upload_) throw std::runtime_error("HTTP fixture/protocol/content failure");
    }
    uint64_t Requests() const {
        return requests_;
    }
    uint64_t Uploaded() const {
        return uploaded_;
    }
    uint64_t ConnectionsAccepted() const { return accepted_; }

   private:
    bool Read(int fd, Connection& c) {
        char input[65536];
        const auto n = recv(fd, input, sizeof(input), 0);
        if (n == 0) return false;
        if (n < 0) return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR;
        const char* data = input;
        size_t size = static_cast<size_t>(n);
        if (!c.parsed) {
            c.head.append(data, size);
            const auto end = c.head.find("\r\n\r\n");
            if (end == std::string::npos) {
                if (c.head.size() > 65536) throw std::runtime_error("oversized load header");
                return true;
            }
            unsigned delay = 0;
            c.put = c.head.compare(0, 4, "PUT ") == 0;
            if ((!c.put && c.head.compare(0, 4, "GET ") != 0) ||
                std::sscanf(c.head.c_str() + 4, "/bytes/%zu/%u", &c.size, &delay) != 2 || c.size > 100 * 1024 * 1024 ||
                delay > 3000 || ++requests_ > 200000)
                throw std::runtime_error("invalid load request");
            c.parsed = true;
            c.ready = Clock::now() + std::chrono::milliseconds(delay);
            c.response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(c.put ? 0 : c.size) +
                         "\r\nConnection: keep-alive\r\n\r\n";
            data = c.head.data() + end + 4;
            size = c.head.size() - end - 4;
        }
        if (c.put) {
            if (size > c.size - c.received) throw std::runtime_error("unexpected upload length/pipeline");
            if (verify_ && std::find_if(data, data + size, [](char byte) { return byte != 'x'; }) != data + size)
                bad_upload_ = true;
            c.received += size;
            uploaded_.fetch_add(size, std::memory_order_relaxed);
        } else if (size) {
            throw std::runtime_error("unexpected GET body/pipeline");
        }
        if (c.parsed) c.head.clear();
        return true;
    }
    bool Write(int fd, Connection& c) {
        const bool header = c.head_sent < c.response.size();
        const char* data = header ? c.response.data() + c.head_sent : block_.data();
        const size_t remaining = header ? c.response.size() - c.head_sent : std::min(block_.size(), c.size - c.sent);
        const auto n = send(fd, data, remaining, MSG_NOSIGNAL);
        if (n < 0) return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR;
        if (n == 0) return false;
        if (header)
            c.head_sent += static_cast<size_t>(n);
        else
            c.sent += static_cast<size_t>(n);
        if (c.head_sent == c.response.size() && (c.put || c.sent == c.size)) c = Connection{};
        return true;
    }
    void Run() {
        std::vector<pollfd> fds;
        fds.reserve(257);
        while (!stop_) {
            fds.clear();
            fds.push_back({listener_, POLLIN, 0});
            for (const auto& item : connections_) {
                const auto& c = item.second;
                short events = 0;
                if (!c.parsed || (c.put && c.received < c.size))
                    events = POLLIN;
                else if (Clock::now() >= c.ready)
                    events = POLLOUT;
                fds.push_back({item.first, events, 0});
            }
            const int n = poll(fds.data(), fds.size(), 10);
            if (n < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("load poll");
            }
            if (fds[0].revents & POLLIN) {
                const int fd = accept4(listener_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (fd >= 0) {
                    ++accepted_;
                    if (connections_.size() >= 256) {
                        close(fd);
                        throw std::runtime_error("load connection bound");
                    }
                    const int one = 1;
                    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
                    connections_.emplace(fd, Connection{});
                }
            }
            for (size_t i = 1; i < fds.size(); ++i) {
                const auto fd = fds[i].fd;
                auto& c = connections_.at(fd);
                bool alive = !(fds[i].revents & (POLLERR | POLLNVAL));
                if (alive && (fds[i].revents & (POLLIN | POLLHUP))) alive = Read(fd, c);
                if (alive && (fds[i].revents & POLLOUT)) alive = Write(fd, c);
                if (!alive) {
                    close(fd);
                    connections_.erase(fd);
                }
            }
        }
    }
    const bool verify_;
    const std::string block_;
    int listener_ = -1;
    std::string endpoint_;
    std::thread thread_;
    std::map<int, Connection> connections_;
    std::atomic<bool> stop_{false}, failed_{false}, bad_upload_{false};
    std::atomic<uint64_t> requests_{0}, uploaded_{0};
    std::atomic<uint64_t> accepted_{0};
};
}  // namespace tos_test
