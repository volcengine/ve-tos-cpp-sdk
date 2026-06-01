#pragma once

#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "model/async/base/SseBase.h"

#include <string>

namespace VolcengineTos {
class CopySourceSseBase : virtual public Headers {
public:
    // SSE和Range不一样，Range不能同时存在(故继承)，但是SSE会source和目标都存在
    CopySourceSseBase() = default;
    ~CopySourceSseBase() override = default;

    const std::string& getCopySourceSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setCopySourceSsecAlgorithm(const std::string& value) {
        ssecAlgorithm_ = value;
    }
    const std::string& getCopySourceSsecKey() const {
        return ssecKey_;
    }
    void setCopySourceSsecKey(const std::string& value) {
        ssecKey_ = value;
    }
    const std::string& getCopySourceSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setCopySourceSsecKeyMd5(const std::string& value) {
        ssecKeyMD5_ = value;
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
        addHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_ALGORITHM, getCopySourceSsecAlgorithm());
        addHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY, getCopySourceSsecKey());
        addHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY_MD5, getCopySourceSsecKeyMd5());
    }

private:
    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKey_;
    std::string ssecKeyMD5_;
};
}  // namespace VolcengineTos
