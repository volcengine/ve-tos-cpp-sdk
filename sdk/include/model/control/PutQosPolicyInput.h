#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "model/GenericInput.h"


namespace VolcengineTos {
class PutQosPolicyInput : public GenericInput {
public:
    explicit PutQosPolicyInput(std::string accountID, std::string policy) : accountID_(std::move(accountID)),policy_(std::move(policy)) {
    }
    PutQosPolicyInput() = default;
    virtual ~PutQosPolicyInput() = default;
    const std::string& getAccountID() const {
        return accountID_;
    }
    void setAccountID(const std::string& accountID) {
        accountID_ = accountID;
    }

    const std::string& getPolicy() const {
        return policy_;
    }
    void setPolicy(const std::string& policy) {
        policy_ = policy;
    }


private:
    std::string accountID_;
    std::string policy_;
};
}  // namespace VolcengineTos
