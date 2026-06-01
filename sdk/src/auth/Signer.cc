#include "auth/Signer.h"
#include "auth/StaticCredentials.h"
#include "transport/http/HttpRequest.h"
VolcengineTos::Signer::Signer() = default;
VolcengineTos::Signer::~Signer() = default;
VolcengineTos::Signer::Signer(const std::shared_ptr<Credentials>& credentials, const std::string& region) {
}

std::map<std::string, std::string> VolcengineTos::Signer::signHeader(const std::shared_ptr<TosRequest>& req) {
    return {};
}

void VolcengineTos::Signer::signHeader(const std::shared_ptr<HttpRequest>& req) {}

std::map<std::string, std::string> VolcengineTos::Signer::signQuery(const std::shared_ptr<TosRequest>& req,
                                                                    std::chrono::duration<int> ttl) {
    return {};
}
