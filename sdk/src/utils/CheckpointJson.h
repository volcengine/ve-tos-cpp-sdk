#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include "external/json/json.hpp"

namespace VolcengineTos {
namespace CheckpointJson {

inline const char* bytesKey() {
    return "__tos_checkpoint_bytes_v1";
}

inline void encodeInvalidStrings(nlohmann::json& value) {
    if (value.is_string()) {
        try {
            value.dump();
        } catch (const nlohmann::json::type_error& error) {
            if (error.id != 316) {
                throw;
            }
            auto bytes = nlohmann::json::array();
            for (unsigned char byte : value.get_ref<const std::string&>()) {
                bytes.push_back(byte);
            }
            value = nlohmann::json{{bytesKey(), std::move(bytes)}};
        }
    } else if (value.is_structured()) {
        for (auto& child : value) {
            encodeInvalidStrings(child);
        }
    }
}

inline std::string dump(const nlohmann::json& value) {
    try {
        return value.dump();
    } catch (const nlohmann::json::type_error& error) {
        if (error.id != 316) {
            throw;
        }
    }
    // Valid UTF-8 keeps the legacy format; only invalid strings get a byte envelope.
    auto encoded = value;
    encodeInvalidStrings(encoded);
    return encoded.dump();
}

inline void decodeStrings(nlohmann::json& value) {
    if (value.is_object() && value.contains(bytesKey())) {
        const auto& bytes = value.at(bytesKey());
        if (value.size() != 1 || !bytes.is_array()) {
            throw std::invalid_argument("invalid checkpoint byte envelope");
        }
        std::string decoded;
        decoded.reserve(bytes.size());
        for (const auto& byte : bytes) {
            if (!byte.is_number_unsigned() || byte.get<uint64_t>() > 255) {
                throw std::invalid_argument("invalid checkpoint byte value");
            }
            decoded.push_back(static_cast<char>(byte.get<unsigned char>()));
        }
        value = std::move(decoded);
    } else if (value.is_structured()) {
        for (auto& child : value) {
            decodeStrings(child);
        }
    }
}

inline nlohmann::json parse(const std::string& content) {
    auto value = nlohmann::json::parse(content, [](int depth, nlohmann::json::parse_event_t, nlohmann::json&) {
        if (depth > 64) {
            throw std::invalid_argument("checkpoint JSON nesting limit exceeded");
        }
        return true;
    });
    decodeStrings(value);
    return value;
}

}  // namespace CheckpointJson
}  // namespace VolcengineTos
