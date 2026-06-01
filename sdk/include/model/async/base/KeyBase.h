#pragma once
#include "PiplineInputInterface.h"
#include "PiplineOutputInterface.h"

#include <string>
#include <utility>
namespace VolcengineTos {
static std::string isValidKey(const std::string& key);

class KeyBase : virtual public PiplineOutputInterface, public virtual PiplineInputInterface {
public:
    KeyBase() = default;
    ~KeyBase() override = default;

    explicit KeyBase(std::string key) : key_(std::move(key)) {
    }

    void setKey(const std::string& key) {
        key_ = key;
    }

    const std::string& getKey() const {
        return key_;
    }

protected:
    std::string valid() override {
        return isValidKey(key_);
    }

    void json2Output(json& j) override {
        if (j.contains("Key")) {
            key_ = j["Key"].get<std::string>();
        }
    }

private:
    std::string key_;
};

static std::string isValidKey(const std::string& key) {
    if (key.empty() || key.length() > 1024) {
        return "invalid object name, the length must be [1, 1024]";
    }
    return "";
}

}  // namespace VolcengineTos
