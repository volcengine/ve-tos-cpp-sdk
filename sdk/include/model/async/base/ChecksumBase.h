#pragma once

#include "common/Common.h"
#include "Headers.h"

#include <cstdint>
#include <string>

namespace VolcengineTos {

class ChecksumBase : virtual public Headers {
public:
    ChecksumBase() = default;
    ~ChecksumBase() override = default;

    const std::string& getEtag() const {
        return eTag_;
    }
    void setEtag(const std::string& etag) {
        eTag_ = etag;
    }
    uint64_t getPreHashCrc64Ecma() const {
        return preHashCrc64ecma_;
    }
    void setPreHashCrc64Ecma(const uint64_t hashCrc64ecma) {
        preHashCrc64ecma_ = hashCrc64ecma;
    }
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(const uint64_t hashCrc64ecma) {
        hashCrc64ecma_ = hashCrc64ecma;
    }
    uint64_t getCalHashCrc64Ecma() const {
        return calHashCrc64ecma_;
    }
    void setCalHashCrc64Ecma(const uint64_t hashCrc64ecma) {
        calHashCrc64ecma_ = hashCrc64ecma;
    }
    uint64_t getHashCrc32Ecma() const {
        return hashCrc32ecma_;
    }
    void setHashCrc32Ecma(const uint64_t hashCrc32ecma) {
        hashCrc32ecma_ = hashCrc32ecma;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        eTag_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), http::HEADER_ETAG);
        const auto hashCrc64ecmaString = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_CRC64);
        if (!hashCrc64ecmaString.empty()) {
            hashCrc64ecma_ = std::stoull(hashCrc64ecmaString);
        }
        const auto hashCrc32ecmaString = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_CRC32);
        if (!hashCrc32ecmaString.empty()) {
            hashCrc32ecma_ = std::stoull(hashCrc32ecmaString);
        }
        calHashCrc64ecma_ = response.getHashCrc64Result();
    }

    void json2Output(json& j) override {
        safeGetJsonValue(j, "ETag", eTag_);
        safeGetJsonValue(j, "CRC32", hashCrc32ecma_);
        safeGetJsonValue(j, "CRC64", hashCrc64ecma_);
    }

private:
    std::string eTag_;
    uint64_t hashCrc64ecma_ = 0;
    uint64_t preHashCrc64ecma_ = 0;

    uint64_t hashCrc32ecma_ = 0;
    uint64_t calHashCrc64ecma_ = 0;
};

}  // namespace VolcengineTos
