#pragma once

#include <string>
#include <utility>
#include "Type.h"

namespace VolcengineTos {
class AccessLogConfiguration {
public:
    AccessLogConfiguration(bool useServiceTopic, std::string tlsProjectId, std::string tlsTopicId)
            : useServiceTopic_(useServiceTopic),
              TLSProjectId_(std::move(tlsProjectId)),
              TLSTopicId_(std::move(tlsTopicId)) {
    }
    AccessLogConfiguration() = default;
    virtual ~AccessLogConfiguration() = default;
    bool isUseServiceTopic() const {
        return useServiceTopic_;
    }
    void setUseServiceTopic(bool useServiceTopic) {
        useServiceTopic_ = useServiceTopic;
    }
    const std::string& getTlsProjectId() const {
        return TLSProjectId_;
    }
    void setTlsProjectId(const std::string& tlsProjectId) {
        TLSProjectId_ = tlsProjectId;
    }
    const std::string& getTlsTopicId() const {
        return TLSTopicId_;
    }
    void setTlsTopicId(const std::string& tlsTopicId) {
        TLSTopicId_ = tlsTopicId;
    }

private:
    bool useServiceTopic_ = false;
    std::string TLSProjectId_;
    std::string TLSTopicId_;
};
}  // namespace VolcengineTos
