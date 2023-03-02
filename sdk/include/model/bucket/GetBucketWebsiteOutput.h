#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "RedirectAllRequestsTo.h"
#include "IndexDocument.h"
#include "ErrorDocument.h"
#include "RoutingRules.h"

namespace VolcengineTos {
class GetBucketWebsiteOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::shared_ptr<RedirectAllRequestsTo>& getRedirectAllRequestsTo() const {
        return redirectAllRequestsTo_;
    }
    void setRedirectAllRequestsTo(const std::shared_ptr<RedirectAllRequestsTo>& redirectAllRequestsTo) {
        redirectAllRequestsTo_ = redirectAllRequestsTo;
    }
    const std::shared_ptr<IndexDocument>& getIndexDocument() const {
        return indexDocument_;
    }
    void setIndexDocument(const std::shared_ptr<IndexDocument>& indexDocument) {
        indexDocument_ = indexDocument;
    }
    const std::shared_ptr<ErrorDocument>& getErrorDocument() const {
        return errorDocument_;
    }
    void setErrorDocument(const std::shared_ptr<ErrorDocument>& errorDocument) {
        errorDocument_ = errorDocument;
    }
    const std::vector<RoutingRule>& getRoutingRules() const {
        return routingRules_;
    }
    void setRoutingRules(const std::vector<RoutingRule>& routingRules) {
        routingRules_ = routingRules;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::shared_ptr<RedirectAllRequestsTo> redirectAllRequestsTo_ = nullptr;
    std::shared_ptr<IndexDocument> indexDocument_ = nullptr;
    std::shared_ptr<ErrorDocument> errorDocument_ = nullptr;
    std::vector<RoutingRule> routingRules_;
};
}  // namespace VolcengineTos
