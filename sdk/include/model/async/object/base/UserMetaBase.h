#pragma once

#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "utils/BaseUtils.h"

#include <cstring>
#include <map>
#include <string>

namespace VolcengineTos {
static std::map<std::string, std::string> userMeta(const std::map<std::string, std::string>& headers);
class UserMetaBase : virtual public Headers {
public:
    UserMetaBase() = default;
    ~UserMetaBase() override = default;

    const std::map<std::string, std::string>& getMeta() const {
        return meta_;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }

protected:
    void input2Headers() override {
        for (auto& meta_ : meta_) {
            addHeader(VolcengineTos::HEADER_META_PREFIX + CryptoUtils::UrlEncodeChinese(meta_.first),
                      CryptoUtils::UrlEncodeChinese(meta_.second));
        }
    }

    void headers2Output(HttpResponse& response) override {
        meta_ = userMeta(response.Headers());
    }

private:
    std::map<std::string, std::string> meta_;  // TOS SDK 会对 Key/Value 包含的中文汉字进行 URL 编码
};

static std::map<std::string, std::string> userMeta(const std::map<std::string, std::string>& headers) {
    std::map<std::string, std::string> meta;
    for (const auto& header : headers) {
        std::string header_first = VolcengineTos::CryptoUtils::UrlDecodeChinese(header.first);
        const std::string header_second = VolcengineTos::CryptoUtils::UrlDecodeChinese(header.second);
        if (VolcengineTos::StringUtils::startsWithIgnoreCase(header_first, VolcengineTos::HEADER_META_PREFIX)) {
            auto kk = header_first.substr(std::strlen(VolcengineTos::HEADER_META_PREFIX), header_first.size());
            meta[kk] = header_second;
        }
    }
    return meta;
}
}  // namespace VolcengineTos
