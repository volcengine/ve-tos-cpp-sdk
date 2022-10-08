#pragma once

#include <utility>
#include "auth/Signer.h"
#include "common/Common.h"
namespace VolcengineTos {
class HttpRange {
public:
    HttpRange() = default;
    ~HttpRange() = default;
    HttpRange(int64_t start, int64_t end) : start_(start), end_(end) {
    }

    int64_t getStart() const {
        return start_;
    }
    void setStart(int64_t start) {
        start_ = start;
    }
    int64_t getEnd() const {
        return end_;
    }
    void setEnd(int64_t end) {
        end_ = end;
    }

    bool operator==(const HttpRange& rhs) const {
        return start_ == rhs.start_ && end_ == rhs.end_;
    }
    bool operator!=(const HttpRange& rhs) const {
        return !(rhs == *this);
    }
    std::string toString() const {
        std::string ret;
        ret.append("bytes=").append(std::to_string(start_)).append("-").append(std::to_string(end_));
        return ret;
    }
    bool isNull() const {
        return (start_ == 0 && end_ == 0) || start_ > end_;
    }

private:
    int64_t start_ = 0;
    int64_t end_ = 0;
};
class RequestBuilder {
public:
    RequestBuilder(std::shared_ptr<Signer> signer, std::string scheme, std::string host, std::string bucket,
                   std::string object, int urlMode, std::map<std::string, std::string> headers,
                   std::map<std::string, std::string> query)
            : signer_(std::move(signer)),
              scheme_(std::move(scheme)),
              host_(std::move(host)),
              bucket_(std::move(bucket)),
              object_(std::move(object)),
              URLMode_(urlMode),
              headers_(std::move(headers)),
              query_(std::move(query)) {
    }

    long getContentLength() const {
        return contentLength_;
    }
    void setContentLength(long contentLength) {
        contentLength_ = contentLength;
    }
    const HttpRange& getRange() const {
        return range_;
    }
    void setRange(const HttpRange& range) {
        range_ = range;
    }
    const std::map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    std::string findHeader(const std::string& key) {
        return MapUtils::findValueByKeyIgnoreCase(headers_, key);
    }
    void setHeaders(const std::map<std::string, std::string>& headers) {
        headers_ = headers;
    }
    const std::map<std::string, std::string>& getQuery() const {
        return query_;
    }
    void setQuery(const std::map<std::string, std::string>& query) {
        query_ = query;
    }
    bool isAutoRecognizeContentType() const {
        return autoRecognizeContentType_;
    }
    void setAutoRecognizeContentType(bool autoRecognizeContentType) {
        autoRecognizeContentType_ = autoRecognizeContentType;
    }
    void withHeader(const std::string& key, const std::string& value) {
        if (!value.empty()) {
            headers_[key] = value;
        }
    }
    void withQuery(const std::string& key, const std::string& value) {
        query_[key] = value;
    }
    void withQueryCheckEmpty(const std::string& key, const std::string& value) {
        if (!value.empty()) {
            query_[key] = value;
        }
    }
    std::shared_ptr<TosRequest> Build(const std::string& method);
    std::shared_ptr<TosRequest> Build(const std::string& method, std::shared_ptr<std::iostream> content);
    std::shared_ptr<TosRequest> BuildWithCopySource(const std::string& method, const std::string& srcBucket,
                                                    const std::string& srcObject);
    std::shared_ptr<TosRequest> build(const std::string& method);

private:
    std::shared_ptr<Signer> signer_;
    std::string scheme_;
    std::string host_;
    std::string bucket_;
    std::string object_;
    int URLMode_ = 0;
    long contentLength_ = 0;
    HttpRange range_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> query_;
    bool autoRecognizeContentType_ = true;
};
}  // namespace VolcengineTos
