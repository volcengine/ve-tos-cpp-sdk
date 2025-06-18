#pragma once
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <utility>
#include "transport/http/Url.h"
#include "utils/BaseUtils.h"
#include "Type.h"
namespace VolcengineTos {
class TosRequest {
public:
    TosRequest() = default;
    TosRequest(std::string scheme, std::string method, std::string host, std::string path,
               std::map<std::string, std::string> headers, std::map<std::string, std::string> queries)
            : scheme_(std::move(scheme)),
              method_(std::move(method)),
              host_(std::move(host)),
              path_(std::move(path)),
              headers_(std::move(headers)),
              queries_(std::move(queries)) {
    }
    ~TosRequest() = default;
    Url toUrl();
    void resolveContentLength();
    const std::string& getScheme() const {
        return scheme_;
    }
    void setScheme(const std::string& scheme) {
        scheme_ = scheme;
    }
    const std::string& getMethod() const {
        return method_;
    }
    void setMethod(const std::string& method) {
        method_ = method;
    }
    const std::string& getHost() const {
        return host_;
    }
    void setHost(const std::string& host) {
        host_ = host;
    }
    const std::string& getPath() const {
        return path_;
    }
    void setPath(const std::string& path) {
        path_ = path;
    }
    const std::string& getTimeout() const {
        return timeout_;
    }
    void setTimeout(const std::string& timeout) {
        timeout_ = timeout;
    }
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentLength) {
        contentLength_ = contentLength;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }
    const std::map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    void setSingleHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }
    void setHeaders(const std::map<std::string, std::string>& headers) {
        headers_ = headers;
    }
    const std::map<std::string, std::string>& getQueries() const {
        return queries_;
    }
    void setQueries(const std::map<std::string, std::string>& queries) {
        queries_ = queries;
    }
    void eraseQuery(const std::string& key) {
        queries_.erase(key);
    }
    std::string findQuery(const std::string& key) {
        return MapUtils::findValueByKeyIgnoreCase(queries_, key);
    }
    void setSingleQuery(const std::string& key, const std::string& value) {
        queries_[key] = value;
    }
    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        dataTransferListener_ = datatransferlistener;
    }
    bool isCheckCrc64() const {
        return checkCrc64_;
    }
    void setCheckCrc64(bool checkcrc64) {
        checkCrc64_ = checkcrc64;
    }
    const std::shared_ptr<RateLimiter>& getRataLimiter() const {
        return rataLimiter_;
    }
    void setRataLimiter(const std::shared_ptr<RateLimiter>& ratalimiter) {
        rataLimiter_ = ratalimiter;
    }
    int getMaxRetryCount() const {
        return maxRetryCount_;
    }
    void setMaxRetryCount(int maxretrycount) {
        maxRetryCount_ = maxretrycount;
    }
    const std::string& getFuncName() const {
        return funcName_;
    }
    void setFuncName(const std::string& funcname) {
        funcName_ = funcname;
    }
    int64_t getContentOffset() const {
        return contentOffset_;
    }
    void setContentOffset(int64_t contentoffset) {
        contentOffset_ = contentoffset;
    }
    uint64_t getPreHashCrc64Ecma() const {
        return preHashCrc64ecma_;
    }
    void setPreHashCrc64Ecma(uint64_t prehashcrc64ecma) {
        preHashCrc64ecma_ = prehashcrc64ecma;
    }
    const std::shared_ptr<std::iostream>& getFileContent() const {
        return fileContent_;
    }
    void setFileContent(const std::shared_ptr<std::iostream>& fileContent) {
        fileContent_ = fileContent;
    }
    bool isCheckHighLatency() const {
        return checkHighLatency_;
    }
    void setCheckHighLatency(bool checkHighLatency) {
        checkHighLatency_ = checkHighLatency;
    }

    std::time_t getRequestDate() const {
        return requestDate_;
    }
    void setRequestDate(std::time_t requestDate) {
        requestDate_ = requestDate;
    }

private:
    std::string scheme_;
    std::string method_;
    std::string host_;
    std::string path_;
    std::string timeout_;
    int64_t contentLength_ = 0;
    std::shared_ptr<std::iostream> content_;
    std::shared_ptr<std::iostream> fileContent_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> queries_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    std::shared_ptr<RateLimiter> rataLimiter_ = nullptr;
    bool checkCrc64_ = false;
    int maxRetryCount_ = 0;
    std::string funcName_;
    int64_t contentOffset_ = 0;
    uint64_t preHashCrc64ecma_ = 0;
    bool checkHighLatency_ = false;
    std::time_t requestDate_ = 0;
};
}  // namespace VolcengineTos
