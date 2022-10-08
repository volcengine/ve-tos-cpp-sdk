#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class PreSignedURLInput {
public:
    HttpMethodType getHttpMethod() const {
        return httpMethod_;
    }
    void setHttpMethod(HttpMethodType httpMethod) {
        httpMethod_ = httpMethod;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    int64_t getExpires() const {
        return expires_;
    }
    void setExpires(int64_t expires) {
        expires_ = expires;
    }
    const std::map<std::string, std::string>& getHeader() const {
        return header_;
    }
    void setHeader(const std::map<std::string, std::string>& header) {
        header_ = header;
    }
    const std::map<std::string, std::string>& getQuery() const {
        return query_;
    }
    void setQuery(const std::map<std::string, std::string>& query) {
        query_ = query;
    }
    const std::string& getAlternativeEndpoint() const {
        return alternativeEndpoint_;
    }
    void setAlternativeEndpoint(const std::string& alternativeEndpoint) {
        alternativeEndpoint_ = alternativeEndpoint;
    }

private:
    HttpMethodType httpMethod_ = HttpMethodType::NotSet;
    std::string bucket_;
    std::string key_;
    int64_t expires_ = 0;
    std::map<std::string, std::string> header_;
    std::map<std::string, std::string> query_;
    std::string alternativeEndpoint_;
};
}  // namespace VolcengineTos
