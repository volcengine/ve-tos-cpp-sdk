#include <algorithm>
#include <sstream>
#include <vector>
#include "../src/auth/SignV4.h"
#include "transport/http/Url.h"

using namespace VolcengineTos;

Url::Url(const std::string& url) : scheme_(), host_(), path_(), port_(), query_() {
    if (!url.empty())
        fromString(url);
}

Url::~Url() = default;

bool Url::operator==(const Url& url) const {
    return scheme_ == url.scheme_ && host_ == url.host_ && path_ == url.path_ && port_ == url.port_ &&
           query_ == url.query_;
}

bool Url::operator!=(const Url& url) const {
    return !(*this == url);
}

void Url::clear() {
    scheme_.clear();
    host_.clear();
    path_.clear();
    port_.clear();
    query_.clear();
}

void Url::fromString(const std::string& url) {
    clear();
    if (url.empty())
        return;

    std::string str = url;
    std::string::size_type pos = 0;
    std::string scheme, host, port, path;
    std::map<std::string, std::string> query;

    pos = str.find("://");
    if (pos != std::string::npos) {
        scheme = str.substr(0, pos);
        str.erase(0, pos + 3);
    }

    pos = str.find(':');
    if (pos != std::string::npos) {
        host = str.substr(0, pos);
        str.erase(0, pos + 1);
        pos = str.find('/');
        if (pos != std::string::npos) {
            port = str.substr(0, pos);
            str.erase(0, pos);
        }
    } else {
        pos = str.find('/');
        if (pos != std::string::npos) {
            host = str.substr(0, pos);
            str.erase(0, pos);
        }
    }

    pos = str.find('?');
    if (pos != std::string::npos) {
        path = str.substr(0, pos);
        str.erase(0, pos + 1);
    } else {
        path = str;
        str = "";
    }

    std::vector<std::string> queries;
    while (!str.empty()) {
        pos = str.find('&');
        if (pos != std::string::npos) {
            std::string q = str.substr(0, pos);
            str = str.erase(0, pos + 1);
            queries.push_back(q);
        } else {
            queries.push_back(str);
            str = "";
        }
    }
    for (auto q : queries) {
        pos = q.find('=');
        std::string k, v;
        if (pos != std::basic_string<char>::npos) {
            k = q.substr(0, pos);
            q = q.erase(0, pos + 1);
            v = q;
        } else {
            k = q;
        }
        query[k] = v;
    }

    setScheme(scheme);
    setHost(host);
    setPort(port);
    setPath(path);
    setQuery(query);
}

std::string Url::toString() const {
    std::ostringstream out;
    if (!scheme_.empty())
        out << scheme_ << "://";
    if (!host_.empty())
        out << host_;
    if (!port_.empty())
        out << ":" << port_;
    if (!path_.empty() && path_ != "/")
        out << path_;
    if (hasQuery())
        out << "?" << queryToStringWithEncode();
    return out.str();
}

void Url::setHost(const std::string& host) {
    if (host.empty()) {
        host_.clear();
        return;
    }
    host_ = host;
    std::transform(host_.begin(), host_.end(), host_.begin(), ::tolower);
}
std::string Url::scheme() const {
    return scheme_;
}
void Url::setScheme(const std::string& scheme) {
    if (scheme.empty()) {
        scheme_.clear();
        return;
    }
    scheme_ = scheme;
    std::transform(scheme_.begin(), scheme_.end(), scheme_.begin(), ::tolower);
}

std::string Url::queryToString() const {
    if (query_.empty()) {
        return "";
    }
    std::string res;
    for (const auto& q : query_) {
        res += q.first;
        res += "=";
        res += q.second;
        res += "&";
    }
    res = res.substr(0, res.size() - 1);
    return res;
}

std::string Url::queryToStringWithEncode() const {
    if (query_.empty()) {
        return "";
    }
    std::string res;
    for (const auto& q : query_) {
        if (q.first == "X-Tos-Credential") {
            res += q.first;
            res += "=";
            res += q.second;
            res += "&";
        } else {
            res += SignV4::uriEncode(q.first, true);
            res += "=";
            res += SignV4::uriEncode(q.second, true);
            res += "&";
        }
    }
    res = res.substr(0, res.size() - 1);
    return res;
}