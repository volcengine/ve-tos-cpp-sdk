#include "DefaultTransport.h"
using namespace VolcengineTos;

DefaultTransport::DefaultTransport(const TransportConfig& config) {
    HttpConfig conf{};
    conf.maxConnections = config.getMaxConnections();
    conf.socketTimeout = config.getSocketTimeout();
    conf.connectTimeout = config.getConnectTimeout();
    conf.requestTimeout = config.getRequestTimeout();
    conf.dialTimeout = config.getDialTimeout();
    conf.tcpKeepAlive = config.getKeepAlive();
    conf.enableVerifySSL = config.isEnableVerifySsl();
    conf.proxyHost = config.getProxyHost();
    conf.proxyPort = config.getProxyPort();
    conf.proxyUsername = config.getProxyUsername();
    conf.proxyPassword = config.getProxyPassword();
    conf.dnsCacheTime = config.getDnsCacheTime();
    conf.caPath = config.getCaPath();
    conf.caFile = config.getCaFile();
    conf.highLatencyLogThreshold = config.getHighLatencyLogThreshold();

    client_ = std::make_shared<HttpClient>(conf);
}

std::shared_ptr<TosResponse> DefaultTransport::roundTrip(const std::shared_ptr<TosRequest>& request) {
    auto httpReq = std::make_shared<HttpRequest>(request->getMethod());

    if (request->getFileContent() != nullptr) {
        httpReq->setResponseOutput(request->getFileContent());
    }

    httpReq->setUrl(request->toUrl());
    httpReq->setHeaders(request->getHeaders());
    httpReq->setMethod(request->getMethod());
    httpReq->setBody(request->getContent());
    httpReq->setContentLength(request->getContentLength());
    httpReq->setDataTransferListener(request->getDataTransferListener());
    httpReq->setRateLimiter(request->getRataLimiter());
    httpReq->setCheckCrc64(request->isCheckCrc64());
    httpReq->setPreHashCrc64Ecma(request->getPreHashCrc64Ecma());
    httpReq->setCheckHighLatency(request->isCheckHighLatency());
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
        res->setContentLength(std::stol(cl));
    }
    // Reference to stack memory associated with local variable 'res' returned
    return res;
}
