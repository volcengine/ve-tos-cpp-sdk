#include <utility>

#include "auth/EcsCredentials.h"
#include "../transport/SimpleTransport.h"
#include "RequestBuilder.h"
#include "../src/external/json/json.hpp"

VolcengineTos::EcsCredentials::EcsCredentials() {
    auto conf = TransportConfig();
    conf.setEnableVerifySsl(false);
    transport_ = std::make_shared<SimpleTransport>(conf);
}

VolcengineTos::EcsCredentials::EcsCredentials(std::string role) {
    role_ = std::move(role);
    auto conf = TransportConfig();
    conf.setEnableVerifySsl(false);
    transport_ = std::make_shared<SimpleTransport>(conf);
}

VolcengineTos::EcsCredentials::EcsCredentials(std::string role, std::string url) {
    role_ = std::move(role);
    url_ = std::move(url);
    auto conf = TransportConfig();
    conf.setEnableVerifySsl(false);
    transport_ = std::make_shared<SimpleTransport>(conf);
}

VolcengineTos::Credential VolcengineTos::EcsCredentials::credential() {
    std::time_t now = time(nullptr);
    if (now > expiration_) {
        updateCredential();
    }

    std::lock_guard<std::mutex> lock(update_);
    return credential_;
}

void VolcengineTos::EcsCredentials::updateCredential() {
    // 并发场景下会有多个请求等在这里，需要二次检查
    std::lock_guard<std::mutex> lock(update_);
    std::time_t now = time(nullptr);
    if (now > expiration_) {
        reload();
    }
}

void VolcengineTos::EcsCredentials::reload() {
    auto path = "/volcstack/latest/iam/security_credentials/" + role_;
    std::string host = "100.96.0.96";
    std::string schema = "http";
    // 从 ECS 服务器获取100.96.0.96

    auto url = Url(url_);
    if (!url_.empty()) {
        path = url.path() + role_;
        schema = url.scheme();
        host = url.host();
    }

    auto req = std::make_shared<TosRequest>(schema, "GET", host, path, std::map<std::string, std::string>{},
                                            std::map<std::string, std::string>{});
    auto res = transport_->roundTrip(req);
    if (res->getStatusCode() != 200) {
        return;
    }
    std::ostringstream ss;
    ss << res->getContent()->rdbuf();

    std::string expiredTime;
    std::string currentTime;
    std::string accessKeyId;
    std::string secretAccessKey;
    std::string sessionToken;

    auto j = nlohmann::json::parse(ss.str());
    if (j.contains("ExpiredTime"))
        j.at("ExpiredTime").get_to(expiredTime);
    if (j.contains("CurrentTime"))
        j.at("CurrentTime").get_to(currentTime);
    if (j.contains("AccessKeyId"))
        j.at("AccessKeyId").get_to(accessKeyId);
    if (j.contains("SecretAccessKey"))
        j.at("SecretAccessKey").get_to(secretAccessKey);
    if (j.contains("SessionToken"))
        j.at("SessionToken").get_to(sessionToken);

    if (expiredTime.empty() || accessKeyId.empty() || secretAccessKey.empty() || sessionToken.empty()) {
        return;
    }
    // 后续修改为增加读写锁
    Credential cred(accessKeyId, secretAccessKey, sessionToken);

    credential_ = cred;

    // 获取当前时间 + 20min，并设置 expiration_ 为该时间点
    time_t time = TimeUtils::transEcsExpiredTimeStringToTime(expiredTime);
    if (time == -1) {
        return;
    }
    time = time - 20 * 60;

    expiration_.store(time);
}