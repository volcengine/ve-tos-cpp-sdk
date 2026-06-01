#pragma once

#include "model/async/base/PiplineOutputInterface.h"
#include "model/async/base/Queries.h"

#include <cstdint>
#include <string>

namespace VolcengineTos {
class ListerBase : virtual public Queries, virtual public PiplineOutputInterface {
public:
    ListerBase() = default;
    ~ListerBase() override = default;

    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }

    const std::string& getMarker() const {
        return marker_;
    }
    void setMarker(const std::string& marker) {
        marker_ = marker;
    }
    int64_t getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(const int64_t maxKeys) {
        maxKeys_ = maxKeys;
    }

protected:
    void input2Queries() override {
        addQuery("prefix", getPrefix());
        addQuery("delimiter", getDelimiter());
        addQuery("marker", getMarker());
        if (getMaxKeys() != 0) {
            addQuery("max-keys", std::to_string(getMaxKeys()));
        }
    }

    void json2Output(json& j) override {
        if (j.contains("Prefix")) {
            prefix_ = j["Prefix"].get<std::string>();
        }
        if (j.contains("Marker")) {
            marker_ = j["Marker"].get<std::string>();
        }
        if (j.contains("MaxKeys")) {
            maxKeys_ = j["MaxKeys"].get<int64_t>();
        }
        if (j.contains("Delimiter")) {
            delimiter_ = j["Delimiter"].get<std::string>();
        }
    }

private:
    std::string prefix_;
    std::string delimiter_;

    std::string marker_;

    int64_t maxKeys_ = 0;

    // bool isTruncated_ = false;  //?
};
}  // namespace VolcengineTos
