#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteQosPolicyInput : public GenericInput {
public:
    explicit DeleteQosPolicyInput(std::string accountID) : accountID_(std::move(accountID)) {
    }
    DeleteQosPolicyInput() = default;
    virtual ~DeleteQosPolicyInput() = default;
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
