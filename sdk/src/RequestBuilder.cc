#include "RequestBuilder.h"
#include "common/Common.h"
#include "auth/SignV4.h"
using namespace VolcengineTos;

// 合并 GenericInput RequestHeader 和请求中的 header
void mergeRequestHeaderAndHeader(const std::map<std::string, std::string> &reqHeader, std::map<std::string, std::string> &header) {
    if (reqHeader.empty())
        return;

    std::map<std::string, std::string> lower_header;
    for (const auto& pair : header) {
        lower_header[StringUtils::toLower(pair.first)] = pair.second;
    }

    auto iter = reqHeader.begin();
    for (; iter != reqHeader.end(); iter++) {
        if (iter->first.empty()) {
            continue;
        }

        std::string key(iter->first);
        std::string kk = StringUtils::toLower(key);

        if (kk == "content-length" || kk == "host" || kk == "x-tos-date" ||
            kk == "range" || kk == "transfer-encoding" || kk == "authorization" ||
            kk == "date") {
            continue;
        }

        if (lower_header.find(kk) == lower_header.end()) {
            header.insert({iter->first, iter->second});  // 如果不存在，插入键值对
        }
    }
}

std::shared_ptr<TosRequest> RequestBuilder::build(const std::string& method) {
    std::string host, path;
    if (isCustomDomain_){
        host = host_;
        path = "/"+object_;
    }else{
        if (bucket_.empty()) {
            host = host_;
        } else {
            host = bucket_;
            host += ".";
            host += host_;
            path = "/";
            path += object_;
        }
    }

    mergeRequestHeaderAndHeader(requestHeader_, headers_);
    auto req = std::make_shared<TosRequest>(scheme_, method, host, path, headers_, query_);
    req->setRequestDate(requestDate_);
    return req;
}

std::shared_ptr<TosRequest> RequestBuilder::buildControlRequest(const std::string& method) {
    std::string host = accountID_ + "." + controlHost_;
    mergeRequestHeaderAndHeader(requestHeader_, headers_);
    auto req = std::make_shared<TosRequest>(scheme_, method, host, "/qospolicy", headers_, query_);
    req->setRequestDate(requestDate_);
    return req;
}

std::shared_ptr<TosRequest> RequestBuilder::buildSignedURL(const std::string& method) {
    std::string host, path;
    if (bucket_.empty() || isCustomDomain_) {
        host = host_;
        path = "/";
        path += object_;
    } else {
        host = bucket_;
        host += ".";
        host += host_;
        path = "/";
        path += object_;
    }

    mergeRequestHeaderAndHeader(requestHeader_, headers_);
    auto req = std::make_shared<TosRequest>(scheme_, method, host, path, headers_, query_);
    req->setRequestDate(requestDate_);
    return req;
}
std::shared_ptr<TosRequest> RequestBuilder::Build(const std::string& method) {
    auto req = RequestBuilder::build(method);
    auto sigHeader = signer_->signHeader(req);
    auto iter = sigHeader.begin();
    for (; iter != sigHeader.end(); iter++) {
        req->setSingleHeader(iter->first, iter->second);
    }
    return req;
}

std::shared_ptr<TosRequest> RequestBuilder::BuildControlRequest(const std::string& method) {
    auto req = RequestBuilder::buildControlRequest(method);
    req->setSingleHeader(HEADER_ACCOUNT_ID,accountID_);
    auto sigHeader = signer_->signHeader(req);
    auto iter = sigHeader.begin();
    for (; iter != sigHeader.end(); iter++) {
        req->setSingleHeader(iter->first, iter->second);
    }
    return req;
}

std::shared_ptr<TosRequest> RequestBuilder::Build(const std::string& method, std::shared_ptr<std::iostream> content) {
    auto req = RequestBuilder::Build(method);
    if (content) {
        req->setContent(content);
        if (this->getContentLength() != 0)
            req->setContentLength(this->getContentLength());
        else
            req->resolveContentLength();
    }
    return req;
}

std::shared_ptr<TosRequest> RequestBuilder::BuildControlRequest(const std::string& method, std::shared_ptr<std::iostream> content) {
    auto req = RequestBuilder::BuildControlRequest(method);
    if (content) {
        req->setContent(content);
        if (this->getContentLength() != 0)
            req->setContentLength(this->getContentLength());
        else
            req->resolveContentLength();
    }
    return req;
}


std::string copySource(const std::string& bucket, const std::string& object_, const std::string& versionID) {
    std::string ret;
    auto object = SignV4::uriEncode(object_, false);
    if (versionID.empty()) {
        return ret.append("/").append(bucket).append("/").append(object);
    }
    return ret.append("/").append(bucket).append("/").append(object).append("?versionId=").append(versionID);
}

std::shared_ptr<TosRequest> RequestBuilder::BuildWithCopySource(const std::string& method, const std::string& srcBucket,
                                                                const std::string& srcObject) {
    auto req = build(method);

    std::string versionID = req->findQuery("versionId");
    req->eraseQuery("versionId");

    std::string cpSrcHeader = copySource(srcBucket, srcObject, versionID);
    req->setSingleHeader(VolcengineTos::HEADER_COPY_SOURCE, cpSrcHeader);

    auto sigHeader = signer_->signHeader(req);
    auto iter = sigHeader.begin();
    for (; iter != sigHeader.end(); iter++) {
        req->setSingleHeader(iter->first, iter->second);
    }

    return req;
}
