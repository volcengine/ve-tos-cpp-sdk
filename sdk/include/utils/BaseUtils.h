#pragma once

#include <string>
#include <map>

namespace VolcengineTos {
static const char base64_url_alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                           'O','P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                           'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l','m', 'n',
                                           'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                           '0', '1', '2', '3', '4', '5', '6', '7', '8','9', '+', '/'};
class StringUtils {
public:
  static bool startsWithIgnoreCase(const std::string &src_str,
                                   const std::string &prefix);

  static std::string toLower(const std::string &input);

  static std::string stringToHex(const unsigned char * input, int length);

  static std::string stringReplace(const std::string &input, const std::string &substr, const std::string &newstr);
};
class MapUtils {
public:
  static std::string findValueByKeyIgnoreCase(const std::map<std::string, std::string> &map, const std::string &key);
};
class CryptoUtils {
public:
  static std::string md5Sum(const std::string & input);

private:
  static std::string base64Encode(const unsigned char* input, size_t inputLen);
};
} // namespace VolcengineTos