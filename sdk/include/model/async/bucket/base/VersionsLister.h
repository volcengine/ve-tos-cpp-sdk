#pragma once

#include "model/async/base/PiplineOutputInterface.h"
#include "model/async/base/Queries.h"

#include <cstdint>
#include <string>

namespace VolcengineTos {
class VersionsListerBase : virtual public Queries, virtual public PiplineOutputInterface {
public:
    VersionsListerBase() = default;
    ~VersionsListerBase() override = default;

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

    const std::string& getKeyMarker() const {
        return keyMarker_;
    }
    void setKeyMarker(const std::string& value) {
        keyMarker_ = value;
    }

    const std::string& getVersionIdMarker() const {
        return versionIDMarker_;
    }
    void setVersionIdMarker(const std::string& value) {
        versionIDMarker_ = value;
    }

    int64_t getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(const int64_t maxKeys) {
        maxKeys_ = maxKeys;
    }

protected:
    void input2Queries() override {
        addQuery("prefix", prefix_);
        addQuery("delimiter", delimiter_);
        addQuery("key-marker", keyMarker_);
        addQuery("version-id-marker", versionIDMarker_);
        if (getMaxKeys() != 0) {
            addQuery("max-keys", std::to_string(maxKeys_));
        }
    }

    void json2Output(json& j) override {
        if (j.contains("KeyMarker")) {
            keyMarker_ = j["KeyMarker"].get<std::string>();
        }
        if (j.contains("VersionIdMarker")) {
            versionIDMarker_ = j["VersionIdMarker"].get<std::string>();
        }
        if (j.contains("MaxKeys")) {
            maxKeys_ = j["MaxKeys"].get<int64_t>();
        }
        if (j.contains("Prefix")) {
            prefix_ = j["Prefix"].get<std::string>();
        }
        if (j.contains("Delimiter")) {
            delimiter_ = j["Delimiter"].get<std::string>();
        }
    }

private:
    std::string prefix_;
    std::string delimiter_;

    std::string keyMarker_;
    std::string versionIDMarker_;

    int64_t maxKeys_ = 0;
};
}  // namespace VolcengineTos
