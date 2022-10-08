#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "ListedBucket.h"
#include "ListedOwner.h"
namespace VolcengineTos {
class ListBucketsOutput {
public:
    void fromJsonString(const std::string& output);
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<ListedBucket>& getBuckets() const {
        return buckets_;
    }
    const ListedOwner& getOwner() const {
        return owner_;
    }

private:
    RequestInfo requestInfo_;
    std::vector<ListedBucket> buckets_;
    ListedOwner owner_;
};
}  // namespace VolcengineTos
