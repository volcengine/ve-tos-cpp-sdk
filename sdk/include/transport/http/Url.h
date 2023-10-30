#pragma once

#include <map>
#include <string>

namespace VolcengineTos {
class Url {
public:
    explicit Url(const std::string& url = "");
    ~Url();

    bool operator==(const Url& url) const;
    bool operator!=(const Url& url) const;

    void clear();
    void fromString(const std::string& url);
    std::string toString() const;

    std::string scheme() const;
    void setScheme(const std::string& scheme);

    std::string host() const {
        return host_;
    }
    void setHost(const std::string& host);

    std::string path() const {
        return path_;
    }
    void setPath(const std::string& path) {
        path_ = path;
    }

    std::string port() const {
        return port_;
    }
    void setPort(const std::string& port) {
        port_ = port;
    }
    std::string ipPort() const {
        return host_ + port_;
    }

    bool hasQuery() const {
        return !query_.empty();
    }
    bool isEmpty() const {
        return scheme_.empty() && host_.empty() && path_.empty() && (port_.empty()) && query_.empty();
    }
    std::string queryToString() const;
    std::string queryToStringWithEncode() const;
    std::map<std::string, std::string> query() const {
        return query_;
    }
    void setQuery(const std::map<std::string, std::string>& query) {
        query_ = query;
    }
    void addQuery(const std::string& k, const std::string& v) {
        query_[k] = v;
    }

private:
    std::string scheme_;
    std::string path_;
    std::string host_;
    std::string port_;
    std::map<std::string, std::string> query_;
};
}  // namespace VolcengineTos
