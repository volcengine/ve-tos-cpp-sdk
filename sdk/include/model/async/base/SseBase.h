#pragma once

#include <string>
#include "Headers.h"

namespace VolcengineTos {
static std::string isValidSSEC(const std::string& SSECAlgorithm, const std::string& SSECKey,
                               const std::string& SSECKeyMd5);

class SseBase : virtual public Headers {
public:
    SseBase() = default;
    ~SseBase() override = default;

    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& value) {
        ssecAlgorithm_ = value;
    }
    void setSsecKey(const std::string& value) {
        ssecKey_ = value;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& value) {
        ssecKeyMD5_ = value;
    }
    void setServerSideEncryption(const std::string& value) {
        serverSideEncryption_ = value;
    }

protected:
    std::string valid() override {
        std::string error_string = isValidSSEC(ssecAlgorithm_, ssecKey_, ssecKeyMD5_);
        if (!error_string.empty()) {
            return error_string;
        }

        return "";
    }

    void input2Headers() override {
        addHeader(HEADER_SSE_CUSTOMER_ALGORITHM, ssecAlgorithm_);

        addHeader(HEADER_SSE_CUSTOMER_KEY, ssecKey_);

        addHeader(HEADER_SSE_CUSTOMER_KEY_MD5, ssecKeyMD5_);

        addHeader(HEADER_SSE, serverSideEncryption_);
    }

    void headers2Output(HttpResponse& response) override {
        // 响应比请求少两种header，去掉部分set
        ssecAlgorithm_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_SSE_CUSTOMER_ALGORITHM);
        ssecKeyMD5_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_SSE_CUSTOMER_KEY_MD5);
    }

private:
    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey_;
    std::string ssecKeyMD5_;

    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256
};
static std::string isValidSSEC(const std::string& SSECAlgorithm, const std::string& SSECKey,
                               const std::string& SSECKeyMd5) {
    if (SSECAlgorithm.empty() && SSECKey.empty() && SSECKeyMd5.empty()) {
        return "";
    }
    if (SSECAlgorithm.empty() || SSECKey.empty() || SSECKeyMd5.empty()) {
        return "invalid encryption-decryption algorithm";
    }
    return "";
}
}  // namespace VolcengineTos
