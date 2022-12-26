#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "Filter.h"

namespace VolcengineTos {
class CloudFunctionConfiguration {
public:
    CloudFunctionConfiguration() = default;
    virtual ~CloudFunctionConfiguration() = default;
    CloudFunctionConfiguration(std::string id, std::vector<std::string> events, const Filter& filter,
                               std::string cloudFunction)
            : id_(std::move(id)),
              events_(std::move(events)),
              filter_(filter),
              cloudFunction_(std::move(cloudFunction)) {
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
    const std::string& getCloudFunction() const {
        return cloudFunction_;
    }
    void setCloudFunction(const std::string& cloudFunction) {
        cloudFunction_ = cloudFunction;
    }

private:
    std::string id_;
    std::vector<std::string> events_;
    Filter filter_;
    std::string cloudFunction_;
};
}  // namespace VolcengineTos
