#pragma once

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace tos_test {
// Loopback-only TLS fixture. Ephemeral private key stays in memory. Only the
// public CA certificate is written to a private temporary file, then removed.
// <=128 accepted sockets and <=64 KiB headers; bodies stream in 16 KiB blocks.
class AsyncTlsHttpFixture {
    template <class T, void (*Free)(T*)>
    using Handle = std::unique_ptr<T, decltype(Free)>;
    Handle<SSL_CTX, SSL_CTX_free> context_{nullptr, SSL_CTX_free};
    int listener_{-1};
    std::string endpoint_, ca_;
    std::atomic<bool> stop_{false}, failed_{false};
    std::atomic<unsigned> connections_{0}, handshakes_{0}, resumed_{0}, requests_{0};
    std::mutex mutex_;
    std::set<int> sockets_;
    std::thread acceptor_;
    std::vector<std::thread> workers_;

    static void Require(bool b) {
        if (!b)
            throw std::runtime_error("TLS fixture setup failed");
    }
    void Stop() noexcept {
        stop_ = true;
        if (acceptor_.joinable())
            acceptor_.join();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (int fd : sockets_)
                shutdown(fd, SHUT_RDWR);
        }
        for (auto& t : workers_)
            if (t.joinable())
                t.join();
        if (listener_ >= 0) {
            close(listener_);
            listener_ = -1;
        }
        if (!ca_.empty()) {
            unlink(ca_.c_str());
            ca_.clear();
        }
    }
    static bool Write(SSL* ssl, const char* p, size_t size) {
        while (size) {
            int n = SSL_write(ssl, p, static_cast<int>(size));
            if (n <= 0)
                return false;
            p += n;
            size -= n;
        }
        return true;
    }
    void Serve(int fd) noexcept {
        try {
            Handle<SSL, SSL_free> ssl(SSL_new(context_.get()), SSL_free);
            Require(ssl && SSL_set_fd(ssl.get(), fd) == 1);
            if (SSL_accept(ssl.get()) == 1) {
                ++handshakes_;
                if (SSL_session_reused(ssl.get()))
                    ++resumed_;
                std::string header;
                char block[16384];
                while (!stop_) {
                    int n = SSL_read(ssl.get(), block, sizeof(block));
                    if (n <= 0)
                        break;
                    header.append(block, n);
                    Require(header.size() <= 65536);
                    if (header.find("\r\n\r\n") == std::string::npos)
                        continue;
                    Require(header.compare(0, 4, "GET ") == 0);
                    ++requests_;
                    const bool close_after = header.compare(4, 6, "/close") == 0;
                    const bool large = header.compare(4, 6, "/large") == 0;
                    if (header.compare(4, 5, "/slow") == 0) {
                        for (int i = 0; i != 30 && !stop_; ++i)
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    const size_t bytes = large ? 100 * 1024 * 1024 : 4096;
                    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(bytes) +
                                           "\r\nConnection: " + (close_after ? "close" : "keep-alive") + "\r\n\r\n";
                    if (!Write(ssl.get(), response.data(), response.size()))
                        break;
                    std::fill(block, block + sizeof(block), 'x');
                    bool alive = true;
                    for (size_t sent = 0; sent < bytes && alive && !stop_;) {
                        size_t count = std::min(sizeof(block), bytes - sent);
                        alive = Write(ssl.get(), block, count);
                        sent += count;
                    }
                    if (!alive || close_after)
                        break;
                    header.clear();
                }
                // One shutdown call sends close_notify, without waiting for the peer.
                SSL_shutdown(ssl.get());
            }  // Untrusted CA / peer cancellation are intentional test cases.
        } catch (...) {
            failed_ = true;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        sockets_.erase(fd);
        close(fd);
    }

public:
    explicit AsyncTlsHttpFixture(int version) {
        try {
            Handle<EVP_PKEY_CTX, EVP_PKEY_CTX_free> keygen(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr),
                                                           EVP_PKEY_CTX_free);
            Require(keygen && EVP_PKEY_keygen_init(keygen.get()) == 1 &&
                    EVP_PKEY_CTX_set_rsa_keygen_bits(keygen.get(), 2048) == 1);
            EVP_PKEY* raw_key = nullptr;
            Require(EVP_PKEY_keygen(keygen.get(), &raw_key) == 1);
            Handle<EVP_PKEY, EVP_PKEY_free> key(raw_key, EVP_PKEY_free);
            Handle<X509, X509_free> cert(X509_new(), X509_free);
            Require(cert && X509_set_version(cert.get(), 2) == 1 &&
                    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1) == 1);
            Require(X509_gmtime_adj(X509_getm_notBefore(cert.get()), -60) &&
                    X509_gmtime_adj(X509_getm_notAfter(cert.get()), 3600));
            Require(X509_set_pubkey(cert.get(), key.get()) == 1);
            auto* name = X509_get_subject_name(cert.get());
            Require(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                               reinterpret_cast<const unsigned char*>("localhost"), -1, -1, 0) == 1);
            Require(X509_set_issuer_name(cert.get(), name) == 1);
            auto extension = [&](int nid, const char* value) {
                Handle<X509_EXTENSION, X509_EXTENSION_free> ext(
                        X509V3_EXT_conf_nid(nullptr, nullptr, nid, const_cast<char*>(value)), X509_EXTENSION_free);
                Require(ext && X509_add_ext(cert.get(), ext.get(), -1) == 1);
            };
            extension(NID_basic_constraints, "critical,CA:TRUE");
            extension(NID_subject_alt_name, "DNS:localhost,IP:127.0.0.1");
            Require(X509_sign(cert.get(), key.get(), EVP_sha256()) > 0);
            context_.reset(SSL_CTX_new(TLS_server_method()));
            Require(context_ && SSL_CTX_set_min_proto_version(context_.get(), version) == 1 &&
                    SSL_CTX_set_max_proto_version(context_.get(), version) == 1 &&
                    SSL_CTX_use_certificate(context_.get(), cert.get()) == 1 &&
                    SSL_CTX_use_PrivateKey(context_.get(), key.get()) == 1);
            SSL_CTX_set_session_cache_mode(context_.get(), SSL_SESS_CACHE_SERVER);
            const unsigned char id[] = "offline-shared-engine";
            Require(SSL_CTX_set_session_id_context(context_.get(), id, sizeof(id)) == 1);
            char path[] = "/tmp/tos-shared-ca-XXXXXX";
            int file = mkstemp(path);
            Require(file >= 0);
            ca_ = path;
            FILE* out = fdopen(file, "w");
            if (!out) {
                close(file);
                throw std::runtime_error("TLS CA fixture file");
            }
            const int written = PEM_write_X509(out, cert.get());
            const int closed = fclose(out);
            Require(written == 1 && closed == 0);
            listener_ = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
            Require(listener_ >= 0);
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            socklen_t length = sizeof(address);
            Require(bind(listener_, reinterpret_cast<sockaddr*>(&address), length) == 0 &&
                    getsockname(listener_, reinterpret_cast<sockaddr*>(&address), &length) == 0 &&
                    listen(listener_, 32) == 0);
            endpoint_ = "https://127.0.0.1:" + std::to_string(ntohs(address.sin_port));
            acceptor_ = std::thread([this] {
                try {
                    while (!stop_) {
                        pollfd event{listener_, POLLIN, 0};
                        if (poll(&event, 1, 10) <= 0)
                            continue;
                        int fd = accept4(listener_, nullptr, nullptr, SOCK_CLOEXEC);
                        if (fd < 0)
                            continue;
                        if (++connections_ > 128) {
                            close(fd);
                            throw std::runtime_error("TLS socket bound");
                        }
                        const timeval timeout{5, 0};
                        const int one = 1;
                        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
                        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
                        {
                            std::lock_guard<std::mutex> lock(mutex_);
                            sockets_.insert(fd);
                        }
                        try {
                            workers_.emplace_back([this, fd] { Serve(fd); });
                        } catch (...) {
                            std::lock_guard<std::mutex> lock(mutex_);
                            sockets_.erase(fd);
                            close(fd);
                            throw;
                        }
                    }
                } catch (...) {
                    failed_ = true;
                }
            });
        } catch (...) {
            Stop();
            throw;
        }
    }
    ~AsyncTlsHttpFixture() {
        Stop();
    }
    const std::string& Endpoint() const {
        return endpoint_;
    }
    const std::string& CaFile() const {
        return ca_;
    }
    unsigned Connections() const {
        return connections_;
    }
    unsigned Handshakes() const {
        return handshakes_;
    }
    unsigned Resumed() const {
        return resumed_;
    }
    unsigned Requests() const {
        return requests_;
    }
    void Check() const {
        Require(!failed_);
    }
};
}  // namespace tos_test
