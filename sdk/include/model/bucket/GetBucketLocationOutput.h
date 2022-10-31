#pragma once
#include "model/RequestInfo.h"
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class GetBucketLocationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getExtranetEndpoint() const {
        return extranetEndpoint_;
    }
    void setExtranetEndpoint(const std::string& extranetEndpoint) {
        extranetEndpoint_ = extranetEndpoint;
    }
    const std::string& getIntranetEndpoint() const {
        return intranetEndpoint_;
    }
    void setIntranetEndpoint(const std::string& intranetEndpoint) {
        intranetEndpoint_ = intranetEndpoint;
    }
    const std::string& getRegion() const {
        return region_;
    }
    void setRegion(const std::string& region) {
        region_ = region;
    }
    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Region")) {
            setRegion(j.at("Region").get<std::string>());
        }
        if (j.contains("ExtranetEndpoint")) {
            setExtranetEndpoint(j.at("ExtranetEndpoint").get<std::string>());
        }
        if (j.contains("IntranetEndpoint")) {
            setIntranetEndpoint(j.at("IntranetEndpoint").get<std::string>());
        }
    }

private:
    RequestInfo requestInfo_;
    std::string region_;
    std::string extranetEndpoint_;
    std::string intranetEndpoint_;
};
}  // namespace VolcengineTos
