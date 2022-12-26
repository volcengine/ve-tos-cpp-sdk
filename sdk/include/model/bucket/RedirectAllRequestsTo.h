#pragma once

#include <string>
#include <utility>
#include "Type.h"
namespace VolcengineTos {
class RedirectAllRequestsTo {
public:
    RedirectAllRequestsTo(std::string hostName, std::string Protocol)
            : hostName_(std::move(hostName)), protocol_(std::move(Protocol)) {
    }
    RedirectAllRequestsTo() = default;
    virtual ~RedirectAllRequestsTo() = default;
    const std::string& getHostName() const {
        return hostName_;
    }
    void setHostName(const std::string& hostName) {
        hostName_ = hostName;
    }
    const std::string& getProtocol() const {
        return protocol_;
    }
    void setProtocol(const std::string& Protocol) {
        protocol_ = Protocol;
    }

private:
    std::string hostName_;
    std::string protocol_;
};
}  // namespace VolcengineTos
