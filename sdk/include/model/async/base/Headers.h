#pragma once
#include "common/Common.h"
#include "PiplineInputInterface.h"
#include "PiplineOutputInterface.h"
#include "logger/logger.h"
#include "transport/http/HttpResponse.h"
#include "utils/BaseUtils.h"

#include <cstdint>
#include <ctime>
#include <exception>
#include <map>
#include <string>
namespace VolcengineTos {

class Headers : virtual public PiplineInputInterface, virtual public PiplineOutputInterface {
public:
    Headers() = default;
    ~Headers() override = default;

    const std::map<std::string, std::string>& getHeaders() const {
        return headers_;
    }

    void setHeaders(const std::map<std::string, std::string>& requestHeader) {
        headers_ = requestHeader;
    }

    const std::map<std::string, std::string>& addHeader(const std::string& key, const std::string& value) {
        if (!key.empty() && !value.empty()) {
            headers_[key] = value;
        }
        return headers_;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        const auto headers = response.Headers();
        headers_ = headers;
    }

    /**
     * @brief 查找字符串类型头部（忽略键大小写）
     * @param key 要查找的头部键
     * @param result 输出参数：存储找到的头部值（仅成功时有效）
     * @return bool：true=找到键，false=未找到键
     * 说明：若找到的键对应空字符串，仍返回 true（空字符串是合法值）
     */
    bool findStringHeader(const std::string& key, std::string& result) const {
        std::string value = MapUtils::findValueByKeyIgnoreCase(headers_, key);
        if (value.empty()) {
            // 日志：未找到指定头部键
            Logger::getInstance().debug("Find string header failed: key not found - ", key.c_str());
            return false;  // 未找到键（约定：未找到时返回空字符串）
        }
        result = value;
        return true;
    }

    /**
     * @brief 查找整数类型头部（忽略键大小写）
     * @param key 要查找的头部键
     * @param result 输出参数：存储转换后的 int64_t 值（仅成功时有效）
     * @return bool：true=找到且转换成功，false=未找到/转换失败
     * 处理场景：未找到键、值为空、非数字字符串、数值超出 int64_t 范围
     */
    bool findIntHeader(const std::string& key, int64_t& result) const {
        // 1. 先查找键，未找到直接返回
        std::string value = MapUtils::findValueByKeyIgnoreCase(headers_, key);
        if (value.empty()) {
            Logger::getInstance().debug("Find int header failed: key not found - ", key.c_str());
            return false;
        }

        // 2. 尝试转换为 int64_t，捕获所有转换异常
        try {
            result = std::stoll(value);  // C++11 支持，转换字符串到 long long（兼容 int64_t）
            return true;
        } catch (const std::invalid_argument&) {
            // 非数字字符串（如 "abc"、""）
            Logger::getInstance().debug("Find int header failed: invalid number format - key: ", key.c_str(),
                                        ", value: ", value.c_str());
            return false;
        } catch (const std::out_of_range&) {
            // 数值超出 int64_t 范围（如 "9999999999999999999999"）
            Logger::getInstance().debug("Find int header failed: number out of int64 range - key: ", key.c_str(),
                                        ", value: ", value.c_str());
            return false;
        } catch (const std::exception& e) {
            // 其他未知异常（兜底，保留异常详情）
            Logger::getInstance().debug("Find int header failed: unknown error - key: ", key.c_str(),
                                        ", value: ", value.c_str(), ", error: ", e.what());
            return false;
        }
    }

    bool findUintHeader(const std::string& key, u_int64_t& result) const {
        // 1. 先查找键，未找到直接返回
        std::string value = MapUtils::findValueByKeyIgnoreCase(headers_, key);
        if (value.empty()) {
            Logger::getInstance().debug("Find int header failed: key not found - ", key.c_str());
            return false;
        }

        // 2. 尝试转换为 int64_t，捕获所有转换异常
        try {
            result = std::stoull(value);  // C++11 支持，转换字符串到 long long（兼容 int64_t）
            return true;
        } catch (const std::invalid_argument&) {
            // 非数字字符串（如 "abc"、""）
            Logger::getInstance().debug("Find int header failed: invalid number format - key: ", key.c_str(),
                                        ", value: ", value.c_str());
            return false;
        } catch (const std::out_of_range&) {
            // 数值超出 int64_t 范围（如 "9999999999999999999999"）
            Logger::getInstance().debug("Find int header failed: number out of int64 range - key: ", key.c_str(),
                                        ", value: ", value.c_str());
            return false;
        } catch (const std::exception& e) {
            // 其他未知异常（兜底，保留异常详情）
            Logger::getInstance().debug("Find int header failed: unknown error - key: ", key.c_str(),
                                        ", value: ", value.c_str(), ", error: ", e.what());
            return false;
        }
    }

    /**
     * @brief 查找 GMT 时间类型头部（忽略键大小写）
     * @param key 要查找的头部键
     * @param result 输出参数：存储转换后的 time_t 值（仅成功时有效）
     * @return bool：true=找到且转换成功，false=未找到/转换失败
     * 处理场景：未找到键、值为空、时间格式非法
     */
    bool findTimeHeader(const std::string& key, std::time_t& result) const {
        // 1. 先查找键，未找到直接返回
        std::string value = MapUtils::findValueByKeyIgnoreCase(headers_, key);
        if (value.empty()) {
            Logger::getInstance().debug("Find time header failed: key not found - ", key.c_str());
            return false;
        }

        // 2. 尝试转换 GMT 字符串到 time_t，处理转换失败
        try {
            result = TimeUtils::transGMTFormatStringToTime(value);
            // 补充判断：若转换函数返回无效值（如 -1），视为失败（需结合 TimeUtils 实现调整）
            if (result < 0) {
                Logger::getInstance().debug("Find time header failed: invalid time value - key: ", key.c_str(),
                                            ", value: ", value.c_str());
                return false;
            }
            return true;
        } catch (const std::exception& e) {
            // 若 TimeUtils 转换失败会抛异常，直接捕获（保留异常详情）
            Logger::getInstance().debug("Find time header failed: invalid GMT format - key: ", key.c_str(),
                                        ", value: ", value.c_str(), ", error: ", e.what());
            return false;
        }
    }

private:
    std::map<std::string, std::string> headers_ = {};
};
}  // namespace VolcengineTos
