#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class PostSignatureCondition {
public:
    PostSignatureCondition(std::string key, std::string value, std::shared_ptr<std::string> Operator)
            : key_(std::move(key)), value_(std::move(value)), operator_(std::move(Operator)) {
    }
    PostSignatureCondition(std::string key, std::string value) : key_(std::move(key)), value_(std::move(value)) {
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getValue() const {
        return value_;
    }
    void setValue(const std::string& value) {
        value_ = value;
    }
    const std::shared_ptr<std::string>& getOperator() const {
        return operator_;
    }
    void setOperator(const std::shared_ptr<std::string>& anOperator) {
        operator_ = anOperator;
    }

private:
    std::string key_;
    std::string value_;
    std::shared_ptr<std::string> operator_ = nullptr;
};
}  // namespace VolcengineTos
