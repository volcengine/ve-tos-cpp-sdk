#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "../src/external/json/json.hpp"
#include "RedirectAllRequestsTo.h"
#include "ErrorDocument.h"
#include "IndexDocument.h"
#include "RoutingRules.h"

namespace VolcengineTos {
class PutBucketWebsiteInput {
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
    std::string toJsonString() const {
        nlohmann::json j;

        nlohmann::json re;
        if (redirectAllRequestsTo_ != nullptr) {
            auto re_ = *redirectAllRequestsTo_;
            if (!re_.getHostName().empty()) {
                re["HostName"] = re_.getHostName();
            }
            if (!re_.getProtocol().empty()) {
                re["Protocol"] = re_.getProtocol();
            }
        }
        if (!re.empty()) {
            j["RedirectAllRequestsTo"] = re;
        }
        nlohmann::json in;
        if (indexDocument_ != nullptr) {
            auto in_ = *indexDocument_;
            if (!in_.getSuffix().empty()) {
                in["Suffix"] = in_.getSuffix();
            }
            in["ForbiddenSubDir"] = in_.isForbiddenSubDir();
        }
        if (!in.empty()) {
            j["IndexDocument"] = in;
        }
        nlohmann::json er;
        if (errorDocument_ != nullptr) {
            auto er_ = *errorDocument_;
            if (!er_.getKey().empty()) {
                er["Key"] = er_.getKey();
            }
        }
        if (!er.empty()) {
            j["ErrorDocument"] = er;
        }

        nlohmann::json rule;
        nlohmann::json ruleArray = nlohmann::json::array();
        for (auto& r : routingRules_) {
            nlohmann::json rule;
            const auto& condition_ = r.getCondition();
            nlohmann::json condition;
            if (!condition_.getKeyPrefixEquals().empty()) {
                condition["KeyPrefixEquals"] = condition_.getKeyPrefixEquals();
            }
            if (condition_.getHttpErrorCodeReturnedEquals() != 0) {
                condition["HttpErrorCodeReturnedEquals"] = condition_.getHttpErrorCodeReturnedEquals();
            }
            if (!condition.empty()) {
                rule["Condition"] = condition;
            }
            const auto& redirect_ = r.getRedirect();
            nlohmann::json redirect;
            if (!redirect_.getHostName().empty()) {
                redirect["HostName"] = redirect_.getHostName();
            }
            if (redirect_.getHttpRedirectCode() != 0) {
                redirect["HttpRedirectCode"] = redirect_.getHttpRedirectCode();
            }
            if (redirect_.getProtocolType() != ProtocolType::NotSet) {
                redirect["Protocol"] = ProtocolTypetoString[redirect_.getProtocolType()];
            }
            if (!redirect_.getReplaceKeyPrefixWith().empty()) {
                redirect["ReplaceKeyPrefixWith"] = redirect_.getReplaceKeyPrefixWith();
            }
            if (!redirect_.getReplaceKeyWith().empty()) {
                redirect["ReplaceKeyWith"] = redirect_.getReplaceKeyWith();
            }
            if (!redirect.empty()) {
                rule["Redirect"] = redirect;
            }
            ruleArray.emplace_back(rule);
        }
        if (!ruleArray.empty()) {
            j["RoutingRules"] = ruleArray;
        }

        return j.dump();
    }

private:
    std::string bucket_;
    std::shared_ptr<RedirectAllRequestsTo> redirectAllRequestsTo_ = nullptr;
    std::shared_ptr<IndexDocument> indexDocument_ = nullptr;
    std::shared_ptr<ErrorDocument> errorDocument_ = nullptr;
    std::vector<RoutingRule> routingRules_;
};
}  // namespace VolcengineTos
