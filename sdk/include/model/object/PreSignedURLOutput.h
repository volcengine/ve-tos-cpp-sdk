#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class PreSignedURLOutput {
public:
    const std::string& getSignUrl() const {
        return signURL_;
    }
    void setSignUrl(const std::string& signURL) {
        signURL_ = signURL;
    }
    const std::map<std::string, std::string>& getSignHeader() const {
        return signHeader_;
    }
    void setSignHeader(const std::map<std::string, std::string>& signHeader) {
        signHeader_ = signHeader;
    }

private:
    std::string signURL_;
    std::map<std::string, std::string> signHeader_;
};
}  // namespace VolcengineTos
