#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "../src/external/json/json.hpp"
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

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);

        if (j.contains("RedirectAllRequestsTo")) {
            auto r = j.at("RedirectAllRequestsTo");
            RedirectAllRequestsTo redirectAllRequestsTo;
            if (r.contains("HostName")) {
                redirectAllRequestsTo.setHostName(r.at("HostName").get<std::string>());
            }
            if (r.contains("Protocol")) {
                redirectAllRequestsTo.setProtocol(r.at("Protocol").get<std::string>());
            }
            redirectAllRequestsTo_ = std::make_shared<RedirectAllRequestsTo>(redirectAllRequestsTo);
        }
        if (j.contains("IndexDocument")) {
            auto i = j.at("IndexDocument");
            IndexDocument indexDocument;
            if (i.contains("Suffix")) {
                indexDocument.setSuffix(i.at("Suffix").get<std::string>());
            }
            if (i.contains("ForbiddenSubDir")) {
                indexDocument.setForbiddenSubDir(i.at("ForbiddenSubDir").get<bool>());
            }
            indexDocument_ = std::make_shared<IndexDocument>(indexDocument);
        }
        if (j.contains("ErrorDocument")) {
            auto e = j.at("ErrorDocument");
            ErrorDocument errorDocument;
            if (e.contains("Key")) {
                errorDocument.setKey(e.at("Key").get<std::string>());
            }
            errorDocument_ = std::make_shared<ErrorDocument>(errorDocument);
        }
        if (j.contains("RoutingRules")) {
            auto routingRules = j.at("RoutingRules");
            for (const auto& r : routingRules) {
                RoutingRule rule;
                if (r.contains("Condition")) {
                    RoutingRuleCondition condition_;
                    auto condition = r.at("Condition");
                    if (condition.contains("HttpErrorCodeReturnedEquals")) {
                        condition_.setHttpErrorCodeReturnedEquals(
                                condition.at("HttpErrorCodeReturnedEquals").get<int>());
                    }
                    if (condition.contains("KeyPrefixEquals")) {
                        condition_.setKeyPrefixEquals(condition.at("KeyPrefixEquals").get<std::string>());
                    }
                    rule.setCondition(condition_);
                }
                if (r.contains("Redirect")) {
                    RoutingRuleRedirect routingRuleRedirect_;
                    auto routingRuleRedirect = r.at("Redirect");

                    if (routingRuleRedirect.contains("HostName")) {
                        routingRuleRedirect_.setHostName(routingRuleRedirect.at("HostName").get<std::string>());
                    }
                    if (routingRuleRedirect.contains("HttpRedirectCode")) {
                        routingRuleRedirect_.setHttpRedirectCode(routingRuleRedirect.at("HttpRedirectCode").get<int>());
                    }
                    if (routingRuleRedirect.contains("Protocol")) {
                        auto protocol = StringtoProtocolType[routingRuleRedirect.at("Protocol").get<std::string>()];
                        routingRuleRedirect_.setProtocolType(protocol);
                    }
                    if (routingRuleRedirect.contains("ReplaceKeyPrefixWith")) {
                        routingRuleRedirect_.setReplaceKeyPrefixWith(
                                routingRuleRedirect.at("ReplaceKeyPrefixWith").get<std::string>());
                    }
                    if (routingRuleRedirect.contains("ReplaceKeyWith")) {
                        routingRuleRedirect_.setReplaceKeyWith(
                                routingRuleRedirect.at("ReplaceKeyWith").get<std::string>());
                    }
                    rule.setRedirect(routingRuleRedirect_);
                }
                routingRules_.emplace_back(rule);
            }
        }
    }

private:
    RequestInfo requestInfo_;
    std::shared_ptr<RedirectAllRequestsTo> redirectAllRequestsTo_ = nullptr;
    std::shared_ptr<IndexDocument> indexDocument_ = nullptr;
    std::shared_ptr<ErrorDocument> errorDocument_ = nullptr;
    std::vector<RoutingRule> routingRules_;
};
}  // namespace VolcengineTos
