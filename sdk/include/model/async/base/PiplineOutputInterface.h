#pragma once
#include "external/json/json.hpp"
#include "transport/http/HttpResponse.h"

#include <cstdint>
#include <string>

namespace VolcengineTos {
using json = nlohmann::json;
class PiplineOutputInterface {
public:
    virtual ~PiplineOutputInterface() = default;

    virtual void headers2Output(HttpResponse& response) {
    }

    virtual void json2Output(json& j) {
    }

    template <typename T>
    static void safeGetJsonValue(const json& j, const std::string& key, T& target);
};

// 模板特化实现
// 特化 int64_t 类型（处理 CRC32，过滤 ""、字符串、null）
template <>
inline void PiplineOutputInterface::safeGetJsonValue<int64_t>(const json& j, const std::string& key, int64_t& target) {
    if (!j.contains(key))
        return;
    const auto& node = j[key];
    if (node.is_null())
        return;
    // 仅允许整数类型，过滤字符串（包括 ""）
    if (node.is_number_integer()) {
        target = node.get<int64_t>();
    }
}

// 特化 std::string 类型（处理 ETag，过滤 null、空字符串 ""）
template <>
inline void PiplineOutputInterface::safeGetJsonValue<std::string>(const json& j, const std::string& key,
                                                                  std::string& target) {
    if (!j.contains(key))
        return;
    const auto& node = j[key];
    if (node.is_null())
        return;
    // 仅允许非空字符串
    if (node.is_string()) {
        std::string temp = node.get<std::string>();
        if (!temp.empty()) {  // 过滤 "" 空字符串
            target = temp;
        }
    }
}

// （可选）特化 uint64_t 类型（处理 CRC64）
template <>
inline void PiplineOutputInterface::safeGetJsonValue<uint64_t>(const json& j, const std::string& key,
                                                               uint64_t& target) {
    if (!j.contains(key))
        return;
    const auto& node = j[key];
    if (node.is_null())
        return;
    // 仅允许无符号整数，过滤字符串（包括 ""）
    if (node.is_number_unsigned()) {
        target = node.get<uint64_t>();
    }
}
}  // namespace VolcengineTos
