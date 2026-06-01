#pragma once
#include "Headers.h"
#include "common/Common.h"
#include "transport/http/HttpResponse.h"

#include <string>
#include <algorithm>  // 用于 std::transform 转小写
#include <cctype>     // 用于 ::tolower

namespace VolcengineTos {

class BaseHttp : virtual public Headers {
public:
    BaseHttp() = default;
    ~BaseHttp() override = default;

    explicit BaseHttp(const std::time_t requestDate,
                      const TransferEncoding transfer_encoding = TransferEncoding::ContentLength)
            : requestDate_(requestDate), transferEncoding_(transfer_encoding) {
    }

    // 原有 getter/setter 保持不变
    void setRequestDate(const std::time_t requestDate) {
        requestDate_ = requestDate;
    }

    std::time_t getRequestDate() const {
        return requestDate_;
    }

    const int64_t& getContentLength() const {
        return contentLength_;
    }
    void setContentLength(const int64_t content_length) {
        contentLength_ = content_length;
        if (content_length > 0) {
            transferEncoding_ = TransferEncoding::ContentLength;
        }
    }

    std::string getContentMd5() const {
        return contentMD5_;
    }
    void setContentMd5(const std::string& content_md5) {
        contentMD5_ = content_md5;
    }

    std::string getContentSha256() const {
        return contentSHA256_;
    }
    void setContentSha256(const std::string& content_sha256) {
        contentSHA256_ = content_sha256;
    }

    std::string getCacheControl() const {
        return CacheControl;
    }
    void setCacheControl(const std::string& cache_control) {
        CacheControl = cache_control;
    }

    std::string getContentDisposition() const {
        return ContentDisposition;
    }
    void setContentDisposition(const std::string& content_disposition) {
        ContentDisposition = content_disposition;
    }

    std::string getContentEncoding() const {
        return ContentEncoding;
    }
    void setContentEncoding(const std::string& content_encoding) {
        ContentEncoding = content_encoding;
    }

    std::string getContentLanguage() const {
        return ContentLanguage;
    }
    void setContentLanguage(const std::string& content_language) {
        ContentLanguage = content_language;
    }

    std::string getContentType() const {
        return ContentType;
    }
    void setContentType(const std::string& content_type) {
        ContentType = content_type;
    }

    std::time_t getExpires() const {
        return Expires;
    }
    void setExpires(const std::time_t expires) {
        Expires = expires;
    }

    const int64_t& getRetryAfter() const {
        return retryAfter_;
    }

    TransferEncoding getTransferEncoding() const {
        return transferEncoding_;
    }

    void setTransferEncoding(const TransferEncoding encoding) {
        transferEncoding_ = encoding;
        // 规范约束：Chunked 编码与 Content-Length 互斥
        if (encoding == TransferEncoding::Chunked) {
            if (contentLength_ > 0) {
                contentLength_ = 0;  // 自动重置 Content-Length 避免冲突
            }
        }
    }

protected:
    void input2Headers() override {
        // addHeader(http::HEADER_CONTENT_LENGTH, std::to_string(contentLength_));
        // 不设置content-length，通过libcurl参数设置

        addHeader(http::HEADER_CONTENT_MD5, contentMD5_);
        addHeader(HEADER_CONTENT_SHA256, contentSHA256_);
        addHeader(http::HEADER_CACHE_CONTROL, CacheControl);
        addHeader(http::HEADER_CONTENT_DISPOSITION, CryptoUtils::UrlEncodeChinese(ContentDisposition));
        addHeader(http::HEADER_CONTENT_ENCODING, ContentEncoding);
        addHeader(http::HEADER_CONTENT_LANGUAGE, ContentLanguage);
        addHeader(http::HEADER_CONTENT_TYPE, ContentType);
        addHeader(http::HEADER_EXPIRES, TimeUtils::transTimeToGmtTime(Expires));

        addHeader(http::HEADER_TRANSFER_ENCODING, transferEncodingToString(transferEncoding_));
    }

    void headers2Output(HttpResponse& response) override {
        Headers::headers2Output(response);

        findIntHeader(http::HEADER_CONTENT_LENGTH, contentLength_);
        findStringHeader(http::HEADER_CONTENT_MD5, contentMD5_);
        findStringHeader(HEADER_CONTENT_SHA256, contentSHA256_);
        findStringHeader(http::HEADER_CACHE_CONTROL, CacheControl);
        findStringHeader(http::HEADER_CONTENT_DISPOSITION, ContentDisposition);
        findStringHeader(http::HEADER_CONTENT_ENCODING, ContentEncoding);
        findStringHeader(http::HEADER_CONTENT_LANGUAGE, ContentLanguage);
        findStringHeader(http::HEADER_CONTENT_TYPE, ContentType);
        findTimeHeader(http::HEADER_EXPIRES, Expires);
        findIntHeader(http::HEADER_Retry_After, retryAfter_);

        std::string teStr;
        findStringHeader(http::HEADER_TRANSFER_ENCODING, teStr);
        if (teStr.empty() && contentLength_ > 0) {
            transferEncoding_ = TransferEncoding::ContentLength;
        } else {
            transferEncoding_ = stringToTransferEncoding(teStr);
        }
    }

private:
    // 新增：枚举转字符串（用于设置请求头）
    static std::string transferEncodingToString(TransferEncoding encoding) {
        switch (encoding) {
            case TransferEncoding::Identity:
                return "identity";
            case TransferEncoding::Chunked:
                return "chunked";
            case TransferEncoding::None:
            default:
                return "";  // 未设置时不添加头域
        }
    }

    // 新增：字符串转枚举（用于解析响应头）
    static TransferEncoding stringToTransferEncoding(const std::string& str) {
        if (str.empty()) {
            return TransferEncoding::Identity;  // 未指定时默认使用 Identity
        }

        // 转小写避免大小写敏感问题（HTTP 头域值不区分大小写）
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (lowerStr == "chunked") {
            return TransferEncoding::Chunked;
        } else if (lowerStr == "identity") {
            return TransferEncoding::Identity;
        } else {
            Logger::getInstance().warn("Unsupported Transfer-Encoding value: ", str, ", default to Identity");
            return TransferEncoding::Identity;  // 未知值默认按 Identity 处理
        }
    }

private:
    std::time_t requestDate_ = 0;
    int64_t contentLength_ = 0;
    std::string contentMD5_;
    std::string contentSHA256_;
    std::string CacheControl;
    std::string ContentDisposition;
    std::string ContentEncoding;
    std::string ContentLanguage;
    std::string ContentType;
    std::time_t Expires = 0;
    int64_t retryAfter_ = 0;

    // Transfer-Encoding 枚举成员变量（默认需有content-length）
    TransferEncoding transferEncoding_ = TransferEncoding::ContentLength;
};
}  // namespace VolcengineTos
