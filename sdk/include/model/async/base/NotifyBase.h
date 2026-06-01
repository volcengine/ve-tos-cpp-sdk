#pragma once

#include "common/Common.h"
#include "model/async/base/Headers.h"

#include <string>

namespace VolcengineTos {
class NotifyBase : virtual public Headers {
public:
    NotifyBase() = default;
    ~NotifyBase() override = default;

    const std::string& getNotificationCustomParameters() const {
        return notificationCustomParameters_;
    }
    void setNotificationCustomParameters(const std::string& notificationCustomParameters) {
        notificationCustomParameters_ = notificationCustomParameters;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_NOTIFY_CUSTOM_PARAM, notificationCustomParameters_);
    }

private:
    std::string notificationCustomParameters_;
};
}  // namespace VolcengineTos
