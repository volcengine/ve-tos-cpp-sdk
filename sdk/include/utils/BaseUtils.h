#pragma once
#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifdef _WIN32
const char PATH_DELIMITER = '\\';
const wchar_t WPATH_DELIMITER = L'\\';
#else
const char PATH_DELIMITER = '/';
const wchar_t WPATH_DELIMITER = L'/';
#endif
#include <sys/stat.h>
namespace VolcengineTos {

static const char base64_url_alphabet[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
static const char base64_url[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

static const char* iso8601Layout = "%Y%m%dT%H%M%SZ";
static const char* yyyyMMdd = "%Y%m%d";
static const char* serverTimeFormat = "%Y-%m-%dT%H:%M:%SZ";

class TimeUtils {
public:
    static std::string transTimeToFormat(const std::time_t& t, const char* format);
    static std::string transTimeToGmtTime(const std::time_t& t);
    static std::time_t transGMTFormatStringToTime(const std::string& t);
    static std::string transLastModifiedTimeToString(std::time_t& t);
    static std::time_t transLastModifiedStringToTime(const std::string& t);
    static void sleepSecondTimes(long time);
    static void sleepMilliSecondTimes(long time);
};
class StringUtils {
public:
    static bool startsWithIgnoreCase(const std::string& src_str, const std::string& prefix);

    static std::string toLower(const std::string& input);

    static std::string stringToHex(const unsigned char* input, int length);

    static std::string stringReplace(const std::string& input, const std::string& substr, const std::string& newstr);

    static bool isValidUTF8(const std::string& input);
};
class MapUtils {
public:
    static std::string findValueByKeyIgnoreCase(const std::map<std::string, std::string>& map, const std::string& key);
};
class CryptoUtils {
public:
    static std::string md5Sum(const std::string& input);
    static std::string md5SumURLEncoding(const std::string& input);
    static unsigned char ToHex(unsigned char x);
    static unsigned char FromHex(unsigned char x);
    static std::string UrlEncodeChinese(const std::string& str);
    static std::string UrlDecodeChinese(const std::string& str);
    //    static std::string UrlEncode(const std::string& str);
    //    static std::string UrlDecode(const std::string& str);
    static std::string base64Encode(const unsigned char* input, size_t inputLen);
    static std::string base64EncodeURL(const unsigned char* input, size_t inputLen);
};

class FileUtils {
public:
    static std::string getWorkPath() {
        std::string currentPath = __FILE__;
        auto last_pos = currentPath.rfind("ve-tos-cpp-sdk");
        currentPath = currentPath.substr(0, last_pos);
        currentPath.append("ve-tos-cpp-sdk/");
        return currentPath;
    }
    // 当文件路径最后部分也想创建为文件夹时，endWithFileName = false
    static bool CreateDirectory(const std::string& folder, bool endWithFileName);
};

class NetUtils {
public:
    static bool isNotIP(std::string ip) {
        // 先排除掉 ipv6 和带 port 的情况
        auto position = ip.find(':');
        if (position != std::string::npos) {
            return false;
        }
        // 再检查是不是 ipv4
        if (checkV4(ip))
            return false;
        return true;
    }

private:
    static bool checkV4(std::string& s);
};
}  // namespace VolcengineTos
