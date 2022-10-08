#include "model/object/HeadObjectV2Output.h"
#include "utils/BaseUtils.h"
#include <cstring>
static std::map<std::string, std::string> userMeta(const std::map<std::string, std::string>& headers) {
    std::map<std::string, std::string> meta;
    for (const auto& header : headers) {
        std::string header_first = VolcengineTos::CryptoUtils::UrlDecodeChinese(header.first);
        std::string header_second = VolcengineTos::CryptoUtils::UrlDecodeChinese(header.second);
        if (VolcengineTos::StringUtils::startsWithIgnoreCase(header_first, VolcengineTos::HEADER_META_PREFIX)) {
            auto kk = header_first.substr(std::strlen(VolcengineTos::HEADER_META_PREFIX), header_first.size());
            meta[kk] = header_second;
        }
    }
    return meta;
}
void VolcengineTos::HeadObjectV2Output::fromResponse(TosResponse& res) {
    eTags_ = res.findHeader(http::HEADER_ETAG);
    lastModified_ = TimeUtils::transGMTFormatStringToTime(res.findHeader(http::HEADER_LAST_MODIFIED));
    deleteMarker_ = res.findHeader(HEADER_DELETE_MARKER) == "true";
    ssecAlgorithm_ = res.findHeader(HEADER_SSE_CUSTOMER_ALGORITHM);
    ssecKeyMD5_ = res.findHeader(HEADER_SSE_CUSTOMER_KEY_MD5);
    versionID_ = res.findHeader(HEADER_VERSIONID);
    websiteRedirectLocation_ = res.findHeader(HEADER_WEBSITE_REDIRECT_LOCATION);
    objectType_ = res.findHeader(HEADER_OBJECT_TYPE);
    storageClass_ = StringtoStorageClassType[res.findHeader(HEADER_STORAGE_CLASS)];
    if (!res.findHeader(HEADER_CRC64).empty()) {
        hashCrc64ecma_ = stoull(res.findHeader(HEADER_CRC64));
    }

    meta_ = userMeta(res.getHeaders());
    contentLength_ = res.getContentLength();
    contentType_ = res.findHeader(http::HEADER_CONTENT_TYPE);
    cacheControl_ = res.findHeader(http::HEADER_CACHE_CONTROL);
    contentDisposition_ = res.findHeader(http::HEADER_CONTENT_DISPOSITION);
    contentEncoding_ = res.findHeader(http::HEADER_CONTENT_ENCODING);
    contentLanguage_ = res.findHeader(http::HEADER_CONTENT_LANGUAGE);
    expires_ = TimeUtils::transGMTFormatStringToTime(res.findHeader(http::HEADER_EXPIRES));
}
