#pragma once

#include <string>
#include <utility>
#include <vector>
#include "../../../src/external/json/json.hpp"
namespace VolcengineTos {
class CORSRule {
public:
    CORSRule() = default;
    virtual ~CORSRule() = default;
    CORSRule(std::string id, std::vector<std::string> allowedOrigins, std::vector<std::string> allowedMethods,
             std::vector<std::string> allowedHeaders, std::vector<std::string> exposeHeaders, int maxAgeSeconds)
            : id_(std::move(id)),
              allowedOrigins_(std::move(allowedOrigins)),
              allowedMethods_(std::move(allowedMethods)),
              allowedHeaders_(std::move(allowedHeaders)),
              exposeHeaders_(std::move(exposeHeaders)),
              maxAgeSeconds_(maxAgeSeconds) {
    }
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    const std::vector<std::string>& getAllowedOrigins() const {
        return allowedOrigins_;
    }
    void setAllowedOrigins(const std::vector<std::string>& allowedOrigins) {
        allowedOrigins_ = allowedOrigins;
    }
    const std::vector<std::string>& getAllowedMethods() const {
        return allowedMethods_;
    }
    void setAllowedMethods(const std::vector<std::string>& allowedMethods) {
        allowedMethods_ = allowedMethods;
    }
    const std::vector<std::string>& getAllowedHeaders() const {
        return allowedHeaders_;
    }
    void setAllowedHeaders(const std::vector<std::string>& allowedHeaders) {
        allowedHeaders_ = allowedHeaders;
    }
    const std::vector<std::string>& getExposeHeaders() const {
        return exposeHeaders_;
    }
    void setExposeHeaders(const std::vector<std::string>& exposeHeaders) {
        exposeHeaders_ = exposeHeaders;
    }
    int getMaxAgeSeconds() const {
        return maxAgeSeconds_;
    }
    void setMaxAgeSeconds(int maxAgeSeconds) {
        maxAgeSeconds_ = maxAgeSeconds;
    }
    void addAllowedOrigin(const std::string& allowedOrigins) {
        allowedOrigins_.push_back(allowedOrigins);
    }
    void addAllowedMethod(const std::string& allowedMethods) {
        allowedMethods_.push_back(allowedMethods);
    }
    void addAllowedHeader(const std::string& allowedHeaders) {
        allowedHeaders_.push_back(allowedHeaders);
    }
    void addExposeHeader(const std::string& exposeHeaders) {
        exposeHeaders_.push_back(exposeHeaders);
    }

private:
    std::string id_;
    std::vector<std::string> allowedOrigins_;
    std::vector<std::string> allowedMethods_;
    std::vector<std::string> allowedHeaders_;
    std::vector<std::string> exposeHeaders_;
    int maxAgeSeconds_ = 0;
};
}  // namespace VolcengineTos
