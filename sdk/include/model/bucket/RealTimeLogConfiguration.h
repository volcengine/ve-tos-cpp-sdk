#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "AccessLogConfiguration.h"

namespace VolcengineTos {
class RealTimeLogConfiguration {
public:
    RealTimeLogConfiguration() = default;
    virtual ~RealTimeLogConfiguration() = default;
    RealTimeLogConfiguration(std::string role, const AccessLogConfiguration& configuration)
            : role_(std::move(role)), configuration_(configuration) {
    }
    const std::string& getRole() const {
        return role_;
    }
    void setRole(const std::string& role) {
        role_ = role;
    }
    const AccessLogConfiguration& getConfiguration() const {
        return configuration_;
    }
    void setConfiguration(const AccessLogConfiguration& configuration) {
        configuration_ = configuration;
    }

private:
    std::string role_;
    AccessLogConfiguration configuration_;
};
}  // namespace VolcengineTos
