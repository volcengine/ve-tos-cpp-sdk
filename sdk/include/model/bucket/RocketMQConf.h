#pragma once

#include <string>
#include <utility>

namespace VolcengineTos {
class RocketMQConf {
public:
    RocketMQConf() = default;
    virtual ~RocketMQConf() = default;
    RocketMQConf(std::string instanceId, std::string topic, std::string accessKeyId)
            : instanceID_(std::move(instanceId)), topic_(std::move(topic)), accessKeyID_(std::move(accessKeyId)) {
    }
    const std::string& getInstanceId() const {
        return instanceID_;
    }
    void setInstanceId(const std::string& instanceId) {
        instanceID_ = instanceId;
    }
    const std::string& getTopic() const {
        return topic_;
    }
    void setTopic(const std::string& topic) {
        topic_ = topic;
    }
    const std::string& getAccessKeyId() const {
        return accessKeyID_;
    }
    void setAccessKeyId(const std::string& accessKeyId) {
        accessKeyID_ = accessKeyId;
    }

private:
    std::string instanceID_;
    std::string topic_;
    std::string accessKeyID_;
};
}  // namespace VolcengineTos
