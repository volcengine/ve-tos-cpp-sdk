#include <cstring>
#include <sstream>
#include <openssl/md5.h>
#include "utils/BaseUtils.h"

using namespace VolcengineTos;

bool StringUtils::startsWithIgnoreCase(const std::string &src_str,
                                      const std::string &prefix) {
  return src_str.size() >= prefix.size() &&
         strncasecmp(src_str.c_str(), prefix.c_str(), prefix.size()) == 0;
  }

std::string StringUtils::toLower(const std::string &input) {
  char *inp = (char*)input.c_str();
  std::stringstream ret;
  for (int i = 0; i < input.length(); ++i) {
    if (inp[i] >= 'A' && inp[i] <= 'Z') {
      char low = inp[i] + 32;
      ret << low;
    } else{
      ret << inp[i];
    }
  }
  return ret.str();
}

std::string StringUtils::stringToHex(const unsigned char * input, int length) {
  static const char hex_digits[] = "0123456789abcdef";
  std::string output;
  for (int i = 0; i < length; ++i) {
    output.push_back(hex_digits[input[i] >> 4]);
    output.push_back(hex_digits[input[i] & 15]);
  }
  return output;
}
std::string StringUtils::stringReplace(const std::string &input, const std::string &substr, const std::string &newstr) {
  std::string ret(input);
  std::string::size_type pos = 0;
  while((pos = ret.find(substr)) != std::string::npos) {
    ret.replace(pos, substr.length(), newstr);
  }
  return ret;
}
bool StringUtils::isValidUTF8(const std::string &input) {
  int byteLength = 0;
  unsigned char uc;
  for (char i : input) {
    uc = i;
    // 判断不可见 ascii 字符
    if (uc < 32 || uc == 127) return false;
    if (byteLength == 0) {
      if (uc >= 0x80) {
        if (uc >= 0xFC && uc <= 0xFD) byteLength = 6;
        else if (uc >= 0xF8) byteLength = 5;
        else if (uc >= 0xF0) byteLength = 4;
        else if (uc >= 0xE0) byteLength = 3;
        else if (uc >= 0xC0) byteLength = 2;
        else return false;
        byteLength--;
      }
    } else{
      if ((uc & 0xC0) != 0x80) return false;
      if ((uc < 32)) return false;
      byteLength--;
    }
  }
  if (byteLength > 0 ) return false;
  return true;
}

std::string MapUtils::findValueByKeyIgnoreCase(
    const std::map<std::string, std::string> &map, const std::string &key) {
  auto iter = map.find(key);
  if (iter != map.end()){
    return iter->second;
  } else {
    // only check lowercase and ignore upper case
    iter = map.find(StringUtils::toLower(key));
    if (iter != map.end()) {
      return iter->second;
    }
    else return {};
  }
}

std::string CryptoUtils::md5Sum(const std::string &input) {
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, input.c_str(), input.size());
  unsigned char md5Res[MD5_DIGEST_LENGTH];
  MD5_Final(md5Res, &ctx);
  return base64Encode(md5Res, MD5_DIGEST_LENGTH);
}

std::string CryptoUtils::base64Encode(const unsigned char* input, size_t inputLen) {
  std::string ret;
  size_t len_encoded = (inputLen +2) / 3 * 4;
  ret.reserve(len_encoded);

  unsigned int pos = 0;
  while (pos < inputLen) {
    ret.push_back(base64_url_alphabet[(input[pos + 0] & 0xfc) >> 2]);
    if (pos+1 < inputLen) {
      ret.push_back(base64_url_alphabet[((input[pos + 0] & 0x03) << 4) + ((input[pos + 1] & 0xf0) >> 4)]);
      if (pos+2 < inputLen) {
        ret.push_back(base64_url_alphabet[((input[pos + 1] & 0x0f) << 2) + ((input[pos + 2] & 0xc0) >> 6)]);
        ret.push_back(base64_url_alphabet[  input[pos + 2] & 0x3f]);
      }
      else {
        ret.push_back(base64_url_alphabet[(input[pos + 1] & 0x0f) << 2]);
        ret.push_back('=');
      }
    }
    else {
      ret.push_back(base64_url_alphabet[(input[pos + 0] & 0x03) << 4]);
      ret.push_back('=');
      ret.push_back('=');
    }
    pos += 3;
  }
  return ret;
}
