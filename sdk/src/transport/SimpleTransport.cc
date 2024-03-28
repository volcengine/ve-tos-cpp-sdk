#include "SimpleTransport.h"
using namespace VolcengineTos;

SimpleTransport::SimpleTransport(const TransportConfig& config) {
    HttpConfig conf{};
    conf.maxConnections = config.getMaxConnections();
    conf.socketTimeout = config.getSocketTimeout();
    conf.connectTimeout = config.getConnectTimeout();
    conf.requestTimeout = config.getRequestTimeout();
    conf.dialTimeout = config.getDialTimeout();
    conf.tcpKeepAlive = config.getKeepAlive();
    conf.enableVerifySSL = config.isEnableVerifySsl();

    client_ = std::make_shared<HttpClient>(conf);
}

std::shared_ptr<TosResponse> SimpleTransport::roundTrip(const std::shared_ptr<TosRequest>& request) {
    auto httpReq = std::make_shared<HttpRequest>(request->getMethod());

    if (request->getFileContent() != nullptr) {
        httpReq->setResponseOutput(request->getFileContent());
    }

    httpReq->setUrl(request->toUrl());
    httpReq->setHeaders(request->getHeaders());
    httpReq->setMethod(request->getMethod());
    httpReq->setBody(request->getContent());
    httpReq->setContentLength(request->getContentLength());
    auto httpResp = client_->doRequest(httpReq);
    auto res = std::make_shared<TosResponse>(httpResp->Body());
    res->setStatusCode(httpResp->statusCode());
    res->setStatusMsg(httpResp->statusMsg());
    res->setHeaders(httpResp->Headers());
    res->setHashCrc64Result(httpResp->getHashCrc64Result());
    res->setCurlErrCode(httpResp->getCurlErrCode());
    res->setIsHighLatency(httpResp->isHighLatencyReq());
    std::string cl(httpResp->getHeaderValueByKey(http::HEADER_CONTENT_LENGTH));
    if (cl.empty()) {
        res->setContentLength(0);
    } else {
        res->setContentLength(std::stoll(cl));
    }
    // Reference to stack memory associated with local variable 'res' returned
    return res;
}
