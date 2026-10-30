#include "TosError.h"
#include "../src/external/json/json.hpp"
using namespace VolcengineTos;
using namespace nlohmann;
void TosError::fromJsonString(const std::string& error) {
    auto j = json::parse(error, nullptr, false);
    if (!j.is_object()) {
        return;
    }
    if (j.contains("Code") && j.at("Code").is_string())
        j.at("Code").get_to(code_);
    if (j.contains("Message") && j.at("Message").is_string())
        j.at("Message").get_to(message_);
    if (j.contains("RequestId") && j.at("RequestId").is_string())
        j.at("RequestId").get_to(requestID_);
    if (j.contains("HostId") && j.at("HostId").is_string())
        j.at("HostId").get_to(hostID_);
    if (j.contains("EC") && j.at("EC").is_string())
        j.at("EC").get_to(EC_);
}
