#pragma once

#include <string>
#include <utility>
#include <iostream>
#include "Type.h"
#include <sstream>
namespace VolcengineTos {
class PreSignedPolicyURLOutput {
public:
    PreSignedPolicyURLOutput(std::string bucket, std::string signedQuery, std::string host, std::string scheme,
                             bool isCustomDomain)
            : bucket_(std::move(bucket)),
              signedQuery_(std::move(signedQuery)),
              host_(std::move(host)),
              scheme_(std::move(scheme)),
              isCustomDomain_(isCustomDomain) {
    }
    PreSignedPolicyURLOutput() = default;
    virtual ~PreSignedPolicyURLOutput() = default;
    const std::string& getSignedQuery() const {
        return signedQuery_;
    }
    void setSignedQuery(const std::string& signedQuery) {
        signedQuery_ = signedQuery;
    }

    std::string GetSignedURLForList(const std::map<std::string, std::string>& additioQuery) {
        std::ostringstream out;
        auto host = bucket_ + "." + host_;
        if (isCustomDomain_) {
            host = host_;
        }
        out << scheme_ << "://" << host << "/?" << signedQuery_ << queryToString(additioQuery);
        return out.str();
    }
    std::string GetSignedURLForGetOrHead(const std::string& key,
                                         const std::map<std::string, std::string>& additioQuery) {
        std::ostringstream out;
        auto host = bucket_ + "." + host_;
        if (isCustomDomain_) {
            host = host_;
        }
        out << scheme_ << "://" << host << "/" << key << "?" << signedQuery_ << queryToString(additioQuery);
        return out.str();
    }

private:
    static std::string queryToString(const std::map<std::string, std::string>& additioQuery);

private:
    std::string signedQuery_;
    std::string bucket_;
    std::string host_;
    std::string scheme_;
    std::string path_;
    bool isCustomDomain_ = false;
};
}  // namespace VolcengineTos
