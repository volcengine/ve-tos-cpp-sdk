#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "Filter.h"
#include "RocketMQConf.h"

namespace VolcengineTos {
class RocketMQConfiguration {
public:
    RocketMQConfiguration() = default;
    virtual ~RocketMQConfiguration() = default;
    RocketMQConfiguration(std::string id, std::string role, std::vector<std::string> events, const Filter& filter,
                          const RocketMQConf& rocketMq)
            : id_(std::move(id)),
              role_(std::move(role)),
              events_(std::move(events)),
              filter_(filter),
              rocketMQ_(rocketMq) {
    }
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    const std::vector<std::string>& getEvents() const {
        return events_;
    }
    void setEvents(const std::vector<std::string>& events) {
        events_ = events;
    }
    const Filter& getFilter() const {
        return filter_;
    }
    void setFilter(const Filter& filter) {
        filter_ = filter;
    }
    const std::string& getRole() const {
        return role_;
    }
    void setRole(const std::string& role) {
        role_ = role;
    }
    const RocketMQConf& getRocketMq() const {
        return rocketMQ_;
    }
    void setRocketMq(const RocketMQConf& rocketMq) {
        rocketMQ_ = rocketMq;
    }

private:
    std::string id_;
    std::string role_;
    std::vector<std::string> events_;
    Filter filter_;
    RocketMQConf rocketMQ_;
};
}  // namespace VolcengineTos
