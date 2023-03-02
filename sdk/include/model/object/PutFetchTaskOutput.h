#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class PutFetchTaskOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getTaskId() const {
        return taskID_;
    }
    void setTaskId(const std::string& taskId) {
        taskID_ = taskId;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string taskID_;
};
}  // namespace VolcengineTos
