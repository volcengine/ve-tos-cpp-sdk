#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "RedirectAllRequestsTo.h"
#include "ErrorDocument.h"
#include "IndexDocument.h"
#include "RoutingRules.h"
#include "model/GenericInput.h"


namespace VolcengineTos {
class PutBucketWebsiteInput : public GenericInput {
public:
    explicit PutBucketWebsiteInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketWebsiteInput(std::string bucket, std::shared_ptr<RedirectAllRequestsTo> redirectAllRequestsTo,
                          std::shared_ptr<IndexDocument> indexDocument, std::shared_ptr<ErrorDocument> errorDocument,
                          std::vector<RoutingRule> routingRules)
            : bucket_(std::move(bucket)),
              redirectAllRequestsTo_(std::move(redirectAllRequestsTo)),
              indexDocument_(std::move(indexDocument)),
              errorDocument_(std::move(errorDocument)),
              routingRules_(std::move(routingRules)) {
    }
    PutBucketWebsiteInput() = default;
    virtual ~PutBucketWebsiteInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
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
    void setIndexDocument(const IndexDocument& indexDocument) {
        indexDocument_ = std::make_shared<IndexDocument>(indexDocument);
    }
    void setRedirectAllRequestsTo(const RedirectAllRequestsTo& redirectAllRequestsTo) {
        redirectAllRequestsTo_ = std::make_shared<RedirectAllRequestsTo>(redirectAllRequestsTo);
    }
    void setErrorDocument(const ErrorDocument& errorDocument) {
        errorDocument_ = std::make_shared<ErrorDocument>(errorDocument);
    }
    const std::vector<RoutingRule>& getRoutingRules() const {
        return routingRules_;
    }
    void setRoutingRules(const std::vector<RoutingRule>& routingRules) {
        routingRules_ = routingRules;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::shared_ptr<RedirectAllRequestsTo> redirectAllRequestsTo_ = nullptr;
    std::shared_ptr<IndexDocument> indexDocument_ = nullptr;
    std::shared_ptr<ErrorDocument> errorDocument_ = nullptr;
    std::vector<RoutingRule> routingRules_;
};
}  // namespace VolcengineTos
