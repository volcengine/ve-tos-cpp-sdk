#include <sstream>
#include <photon/net/http/client.h>
#include <photon/common/alog.h>


#include "transport/http/HttpClient.h"
#include "common/Common.h"


using namespace VolcengineTos;

std::shared_ptr<HttpResponse> PhotonHttpClient::doRequest(const std::shared_ptr<HttpRequest>& request) {
    auto response = std::make_shared<HttpResponse>();

    photon::net::http::Verb verb;
    if (request->method() == http::MethodHead) {
        verb = photon::net::http::Verb::HEAD;
    } else if (request->method() == http::MethodPut) {
        verb = photon::net::http::Verb::PUT;
    } else if (request->method() == http::MethodPost) {
        verb = photon::net::http::Verb::POST;
    } else if (request->method() == http::MethodDelete) {
        verb = photon::net::http::Verb::DELETE;
    } else {
        verb = photon::net::http::Verb::GET;
    }

    std::string url = request->url().toString();
    // TODO: limit request size
    auto op = client_->new_operation(verb, url);
    DEFER(delete op);

    uint64_t timeout = 1000UL * std::max(requestTimeout_, connectTimeout_);
    if (timeout == 0) {
        timeout = -1;
    }
    op->timeout = Timeout(timeout);

    for (auto& p: request->Headers()) {
        if (p.second.empty())
            continue;
        op->req.headers.insert(p.first, p.second);
    }

    int ret = op->call();

    size_t n = op->resp.body_size();
    if (n > 0) {
        auto s = std::make_shared<std::stringstream>();
        void* buf = malloc(n);
        op->resp.read(buf, n);
        s->write((char*) buf, (std::streamsize) n);
        response->setBody(s);
        free(buf);
    }

    if (ret != 0 && errno == ENOENT) {
        response->setStatus(http::Refused);
        response->setStatusMsg("connection refused");
        response->setCurlErrCode(errno);
    } else if (ret != 0) {
        response->setStatus(http::otherErr);
        response->setStatusMsg("connection error");
        response->setCurlErrCode(errno);
    } else {
        response->setStatus(http::Success);
    }

    for (auto iter = op->resp.headers.begin(); iter != op->resp.headers.end(); ++iter) {
        response->setHeader(std::string(iter.first()), std::string(iter.second()));
    }

    response->setStatusCode(op->status_code);

    // TODO：CRC and TransferedBytes

    return response;
}
