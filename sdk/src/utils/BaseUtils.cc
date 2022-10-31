#include <cstring>
#include <sstream>
#include <openssl/md5.h>
#include "utils/BaseUtils.h"
#include <ctime>
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace VolcengineTos;

std::string TimeUtils::transTimeToFormat(const std::time_t& t, const char* format) {
    char timebuf[32] = {'\0'};
    std::tm gmttm{};
#ifdef _WIN32
    gmtime_s(&gmttm, &t);
#else
    gmtime_r(&t, &gmttm);
#endif
    std::strftime(timebuf, sizeof(timebuf), format, &gmttm);
    std::string ret(timebuf);
    return ret;
}

std::string TimeUtils::transTimeToGmtTime(const std::time_t& t) {
    if (t == 0) {
        return "";
    }
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif
#if defined(__GNUG__) && __GNUC__ < 5
    static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char tmbuff[26];
    snprintf(tmbuff, sizeof(tmbuff), "%.3s, %.2d %.3s %d %.2d:%.2d:%.2d", wday_name[tm.tm_wday], tm.tm_mday,
             mon_name[tm.tm_mon], 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
    date << tmbuff << " GMT";
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
#endif
    return date.str();
}
std::time_t TimeUtils::transGMTFormatStringToTime(const std::string& t) {
    std::istringstream ss(t);
    std::tm tm;
    std::time_t tt = -1;
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");

#ifdef _WIN32
    tt = _mkgmtime64(&tm);
#else
    tt = timegm(&tm);
#endif  // _WIN32

    return tt < 0 ? -1 : tt;
}
std::string TimeUtils::transLastModifiedTimeToString(std::time_t& t) {
    std::stringstream date;
    std::tm tm;
#ifdef _WIN32
    ::gmtime_s(&tm, &t);
#else
    ::gmtime_r(&t, &tm);
#endif
#if defined(__GNUG__) && __GNUC__ < 5
    char tmbuff[26];
    strftime(tmbuff, 26, "%Y-%m-%dT%H:%M:%S.000Z", &tm);
    date << tmbuff;
#else
    date.imbue(std::locale::classic());
    date << std::put_time(&tm, "%Y-%m-%dT%X.000Z");
#endif
    return date.str();
}

std::time_t TimeUtils::transLastModifiedStringToTime(const std::string& t) {
    const char* date = t.c_str();
    std::tm tm;
    std::time_t tt = -1;
    int ms;
    auto result = sscanf(date, "%4d-%2d-%2dT%2d:%2d:%2d.%dZ", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour,
                         &tm.tm_min, &tm.tm_sec, &ms);
    if (result == 7) {
        tm.tm_year = tm.tm_year - 1900;
        tm.tm_mon = tm.tm_mon - 1;
#ifdef _WIN32
        tt = _mkgmtime64(&tm);
#else
        tt = timegm(&tm);
#endif  // _WIN32
    }
    return tt < 0 ? -1 : tt;
}

void TimeUtils::sleepSecondTimes(long time) {
#ifdef _WIN32
    Sleep((time * 1000));
#else
    sleep(time);
#endif
}
void TimeUtils::sleepMilliSecondTimes(long time) {
#ifdef _WIN32
    Sleep(time);
#else
    usleep(time * 1000);
#endif
}
bool StringUtils::startsWithIgnoreCase(const std::string& src_str, const std::string& prefix) {
    return src_str.size() >= prefix.size() && strncasecmp(src_str.c_str(), prefix.c_str(), prefix.size()) == 0;
}

std::string StringUtils::toLower(const std::string& input) {
    char* inp = (char*)input.c_str();
    std::stringstream ret;
    for (int i = 0; i < input.length(); ++i) {
        if (inp[i] >= 'A' && inp[i] <= 'Z') {
            char low = inp[i] + 32;
            ret << low;
        } else {
            ret << inp[i];
        }
    }
    return ret.str();
}

std::string StringUtils::stringToHex(const unsigned char* input, int length) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string output;
    for (int i = 0; i < length; ++i) {
        output.push_back(hex_digits[input[i] >> 4]);
        output.push_back(hex_digits[input[i] & 15]);
    }
    return output;
}
std::string StringUtils::stringReplace(const std::string& input, const std::string& substr, const std::string& newstr) {
    std::string ret(input);
    std::string::size_type pos = 0;
    while ((pos = ret.find(substr)) != std::string::npos) {
        ret.replace(pos, substr.length(), newstr);
    }
    return ret;
}
bool StringUtils::isValidUTF8(const std::string& input) {
    int byteLength = 0;
    unsigned char uc;
    for (char i : input) {
        uc = i;
        // 判断不可见 ascii 字符
        if (uc < 32 || uc == 127)
            return false;
        if (byteLength == 0) {
            if (uc >= 0x80) {
                if (uc >= 0xFC && uc <= 0xFD)
                    byteLength = 6;
                else if (uc >= 0xF8)
                    byteLength = 5;
                else if (uc >= 0xF0)
                    byteLength = 4;
                else if (uc >= 0xE0)
                    byteLength = 3;
                else if (uc >= 0xC0)
                    byteLength = 2;
                else
                    return false;
                byteLength--;
            }
        } else {
            if ((uc & 0xC0) != 0x80)
                return false;
            if ((uc < 32))
                return false;
            byteLength--;
        }
    }
    if (byteLength > 0)
        return false;
    return true;
}

std::string MapUtils::findValueByKeyIgnoreCase(const std::map<std::string, std::string>& map, const std::string& key) {
    auto iter = map.find(key);
    if (iter != map.end()) {
        return iter->second;
    } else {
        // only check lowercase and ignore upper case
        iter = map.find(StringUtils::toLower(key));
        if (iter != map.end()) {
            return iter->second;
        } else {
            return {};
        }
    }
}

std::string CryptoUtils::md5Sum(const std::string& input) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, input.c_str(), input.size());
    unsigned char md5Res[MD5_DIGEST_LENGTH];
    MD5_Final(md5Res, &ctx);
    return base64Encode(md5Res, MD5_DIGEST_LENGTH);
}
std::string CryptoUtils::md5SumURLEncoding(const std::string& input) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, input.c_str(), input.size());
    unsigned char md5Res[MD5_DIGEST_LENGTH];
    MD5_Final(md5Res, &ctx);
    return base64EncodeURL(md5Res, MD5_DIGEST_LENGTH);
}
unsigned char CryptoUtils::ToHex(unsigned char x) {
    return x > 9 ? x + 55 : x + 48;
}

unsigned char CryptoUtils::FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    else
        assert(0);
    return y;
}
int IncludeChinese(char* str) {
    char c;
    while (1) {
        c = *str++;
        if (c == 0)
            break;
        if (c & 0x80)
            if (*str & 0x80)
                return 1;
    }
    return 0;
}
std::string CryptoUtils::UrlEncodeChinese(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    char arr[str.length() + 1];
    strcpy(arr, str.c_str());
    char c;
    char c_;
    for (size_t i = 0; i < length; i++) {
        c = arr[i];
        c_ = arr[i + 1];
        if (c_ == 0) {
            strTemp += str[i];
            break;
        }
        if ((c & 0x80) && (c_ & 0x80)) {
            strTemp += '%';
            strTemp += ToHex((unsigned char)arr[i] >> 4);
            strTemp += ToHex((unsigned char)arr[i] % 16);
            strTemp += '%';
            strTemp += ToHex((unsigned char)arr[i + 1] >> 4);
            strTemp += ToHex((unsigned char)arr[i + 1] % 16);
            i++;
        } else {
            strTemp += str[i];
        }
    }
    return strTemp;
}

std::string CryptoUtils::UrlDecodeChinese(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '%') {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        } else
            strTemp += str[i];
    }
    return strTemp;
}

// std::string CryptoUtils::UrlEncode(const std::string& str) {
//     std::string strTemp = "";
//     size_t length = str.length();
//     for (size_t i = 0; i < length; i++) {
//         if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] ==
//         '~'))
//             strTemp += str[i];
//         else if (str[i] == ' ')
//             strTemp += "+";
//         else {
//             strTemp += '%';
//             strTemp += ToHex((unsigned char)str[i] >> 4);
//             strTemp += ToHex((unsigned char)str[i] % 16);
//         }
//     }
//     return strTemp;
// }
//
// std::string CryptoUtils::UrlDecode(const std::string& str) {
//     std::string strTemp = "";
//     size_t length = str.length();
//     for (size_t i = 0; i < length; i++) {
//         if (str[i] == '+')
//             strTemp += ' ';
//         else if (str[i] == '%') {
//             assert(i + 2 < length);
//             unsigned char high = FromHex((unsigned char)str[++i]);
//             unsigned char low = FromHex((unsigned char)str[++i]);
//             strTemp += high * 16 + low;
//         } else
//             strTemp += str[i];
//     }
//     return strTemp;
// }

std::string CryptoUtils::base64Encode(const unsigned char* input, size_t inputLen) {
    std::string ret;
    size_t len_encoded = (inputLen + 2) / 3 * 4;
    ret.reserve(len_encoded);

    unsigned int pos = 0;
    while (pos < inputLen) {
        ret.push_back(base64_url_alphabet[(input[pos + 0] & 0xfc) >> 2]);
        if (pos + 1 < inputLen) {
            ret.push_back(base64_url_alphabet[((input[pos + 0] & 0x03) << 4) + ((input[pos + 1] & 0xf0) >> 4)]);
            if (pos + 2 < inputLen) {
                ret.push_back(base64_url_alphabet[((input[pos + 1] & 0x0f) << 2) + ((input[pos + 2] & 0xc0) >> 6)]);
                ret.push_back(base64_url_alphabet[input[pos + 2] & 0x3f]);
            } else {
                ret.push_back(base64_url_alphabet[(input[pos + 1] & 0x0f) << 2]);
                ret.push_back('=');
            }
        } else {
            ret.push_back(base64_url_alphabet[(input[pos + 0] & 0x03) << 4]);
            ret.push_back('=');
            ret.push_back('=');
        }
        pos += 3;
    }
    return ret;
}

std::string CryptoUtils::base64EncodeURL(const unsigned char* input, size_t inputLen) {
    std::string ret;
    size_t len_encoded = (inputLen + 2) / 3 * 4;
    ret.reserve(len_encoded);

    unsigned int pos = 0;
    while (pos < inputLen) {
        ret.push_back(base64_url[(input[pos + 0] & 0xfc) >> 2]);
        if (pos + 1 < inputLen) {
            ret.push_back(base64_url[((input[pos + 0] & 0x03) << 4) + ((input[pos + 1] & 0xf0) >> 4)]);
            if (pos + 2 < inputLen) {
                ret.push_back(base64_url[((input[pos + 1] & 0x0f) << 2) + ((input[pos + 2] & 0xc0) >> 6)]);
                ret.push_back(base64_url[input[pos + 2] & 0x3f]);
            } else {
                ret.push_back(base64_url[(input[pos + 1] & 0x0f) << 2]);
                ret.push_back('=');
            }
        } else {
            ret.push_back(base64_url[(input[pos + 0] & 0x03) << 4]);
            ret.push_back('=');
            ret.push_back('=');
        }
        pos += 3;
    }
    return ret;
}

bool FileUtils::CreateDirectory(const std::string& folder, bool endWithFileName) {
    std::string folder_builder;
    std::string sub;
    sub.reserve(folder.size());
    for (auto it = folder.begin(); it != folder.end(); ++it) {
        const char c = *it;
        sub.push_back(c);
        // 当遇到分隔符或者到达文件最后位置时
        if (c == PATH_DELIMITER || (it == folder.end() - 1 && !endWithFileName)) {
            folder_builder.append(sub);
            // 文件不存在返回-1，文件存在返回0， //的场景会认为是同一个文件
            if (access(folder_builder.c_str(), 0) != 0) {
                // 该文件所有者、用户组拥有读，写，搜索操作权限；其他用户拥有可读和搜索权限
                // 和各个后端 SDK 权限对齐
                if (mkdir(folder_builder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}

bool NetUtils::checkV4(std::string& s) {
    int k = 0;         // 记录每个segment起始位置
    int pCnt = 0;      // 记录'.'的个数
    s.push_back('.');  // 方便atoi使用
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] == '.') {
            // 对两个 . . 范围内数据进行判断要在 0～255 之间，同时以 0 开头的情况仅可以有一个 0
            s[i] = '\0';                                       // 方便atoi使用
            if (s[k] == '\0'                                   // 连续..或第一个为.的情况
                || (s[k] == '0' && strlen(&s[k]) > 1)          // 以0开头的情况
                || !(atoi(&s[k]) <= 255 && atoi(&s[k]) >= 0))  // 不符合区间范围
            {
                return false;
            }
            k = i + 1;
            ++pCnt;
        } else if (!(s[i] >= '0' && s[i] <= '9')) {  // 包含非 0-9或'.' 的情况
            // 在这里会把正常 endpoint 筛选出来
            return false;
        }
    }
    if (pCnt != 3 + 1)
        return false;  //'.'不是3段,最后一个1是自己加的
    return true;
}