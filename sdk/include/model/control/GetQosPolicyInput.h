#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetQosPolicyInput : public GenericInput {
public:
    explicit GetQosPolicyInput(std::string accountID) : accountID_(std::move(accountID)) {
    }
    GetQosPolicyInput() = default;
    virtual ~GetQosPolicyInput() = default;
    const std::string& getAccountID() const {
        return accountID_;
    }
    void setAccountID(const std::string& accountID) {
        accountID_ = accountID;
    }

private:
    std::string accountID_;
};
}  // namespace VolcengineTos
