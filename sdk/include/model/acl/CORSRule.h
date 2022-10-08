#pragma once

#include <string>
#include <vector>
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class CORSRule {
public:
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

private:
    std::string id_;  // todo: 这个字段是不是多余的
    std::vector<std::string> allowedOrigins_;
    std::vector<std::string> allowedMethods_;
    std::vector<std::string> allowedHeaders_;
    std::vector<std::string> exposeHeaders_;
    int maxAgeSeconds_ = 0;
};
}  // namespace VolcengineTos
