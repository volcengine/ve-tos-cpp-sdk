#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "Destination.h"
#include "Progress.h"

namespace VolcengineTos {
class ReplicationRule {
public:
    ReplicationRule() = default;
    virtual ~ReplicationRule() = default;
    ReplicationRule(std::string id, StatusType status, std::vector<std::string> prefixSet,
                    const Destination& destination, StatusType historicalObjectReplication)
            : id_(std::move(id)),
              status_(status),
              prefixSet_(std::move(prefixSet)),
              destination_(destination),
              historicalObjectReplication_(historicalObjectReplication) {
    }
    ReplicationRule(std::string id, StatusType status, std::vector<std::string> prefixSet,
                    const Destination& destination, StatusType historicalObjectReplication,
                    std::shared_ptr<Progress> progress)
            : id_(std::move(id)),
              status_(status),
              prefixSet_(std::move(prefixSet)),
              destination_(destination),
              historicalObjectReplication_(historicalObjectReplication),
              progress_(std::move(progress)) {
    }
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    StatusType getStatus() const {
        return status_;
    }
    void setStatus(StatusType status) {
        status_ = status;
    }
    const std::vector<std::string>& getPrefixSet() const {
        return prefixSet_;
    }
    void setPrefixSet(const std::vector<std::string>& prefixSet) {
        prefixSet_ = prefixSet;
    }
    const Destination& getDestination() const {
        return destination_;
    }
    void setDestination(const Destination& destination) {
        destination_ = destination;
    }
    StatusType getHistoricalObjectReplication() const {
        return historicalObjectReplication_;
    }
    void setHistoricalObjectReplication(StatusType historicalObjectReplication) {
        historicalObjectReplication_ = historicalObjectReplication;
    }
    const std::shared_ptr<Progress>& getProgress() const {
        return progress_;
    }
    void setProgress(const std::shared_ptr<Progress>& progress) {
        progress_ = progress;
    }

private:
    std::string id_;
    StatusType status_ = StatusType::NotSet;
    std::vector<std::string> prefixSet_;
    Destination destination_;
    StatusType historicalObjectReplication_ = StatusType::NotSet;
    std::shared_ptr<Progress> progress_;
};
}  // namespace VolcengineTos
