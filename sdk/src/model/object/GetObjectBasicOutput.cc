#include "model/object/GetObjectBasicOutput.h"
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
void VolcengineTos::GetObjectBasicOutput::fromResponse(TosResponse& res) {
    requestInfo_ = res.GetRequestInfo();

    contentRange_ = res.findHeader(http::HEADER_CONTENT_RANGE);
    eTag_ = res.findHeader(http::HEADER_ETAG);
    // 注意由于是头部信息的 modified，所以是 GMT 格式的
    lastModified_ = TimeUtils::transGMTFormatStringToTime(res.findHeader(http::HEADER_LAST_MODIFIED));
    deleteMarker_ = res.findHeader(HEADER_DELETE_MARKER) == "true";
    ssecAlgorithm_ = res.findHeader(HEADER_SSE_CUSTOMER_ALGORITHM);
    ssecKeyMD5_ = res.findHeader(HEADER_SSE_CUSTOMER_KEY_MD5);
    versionID_ = res.findHeader(HEADER_VERSIONID);
    websiteRedirectLocation_ = res.findHeader(HEADER_WEBSITE_REDIRECT_LOCATION);
    objectType_ = res.findHeader(HEADER_OBJECT_TYPE);
    hashCrc64ecma_ = stoull(res.findHeader(HEADER_CRC64));
    storageClass_ = StringtoStorageClassType[res.findHeader(HEADER_STORAGE_CLASS)];

    meta_ = userMeta(res.getHeaders());
    contentLength_ = res.getContentLength();
    contentType_ = res.findHeader(http::HEADER_CONTENT_TYPE);
    cacheControl_ = res.findHeader(http::HEADER_CACHE_CONTROL);
    contentDisposition_ = res.findHeader(http::HEADER_CONTENT_DISPOSITION);
    contentEncoding_ = res.findHeader(http::HEADER_CONTENT_ENCODING);
    contentLanguage_ = res.findHeader(http::HEADER_CONTENT_LANGUAGE);
    expires_ = TimeUtils::transGMTFormatStringToTime(res.findHeader(http::HEADER_EXPIRES));
}
// bool VolcengineTos::GetObjectBasicOutput::operator==(const VolcengineTos::GetObjectBasicOutput& rhs) const {
//     return contentRange_ == rhs.contentRange_ && eTags_ == rhs.eTags_ && lastModified_ == rhs.lastModified_ &&
//            deleteMarker_ == rhs.deleteMarker_ && ssecAlgorithm_ == rhs.ssecAlgorithm_ &&
//            ssecKeyMD5_ == rhs.ssecKeyMD5_ && versionID_ == rhs.versionID_ &&
//            websiteRedirectLocation_ == rhs.websiteRedirectLocation_ && objectType_ == rhs.objectType_ &&
//            hashCrc64ecma_ == rhs.hashCrc64ecma_ && storageClass_ == rhs.storageClass_ && meta_ == rhs.meta_ &&
//            contentLength_ == rhs.contentLength_ && contentType_ == rhs.contentType_ &&
//            cacheControl_ == rhs.cacheControl_ && contentDisposition_ == rhs.contentDisposition_ &&
//            contentEncoding_ == rhs.contentEncoding_ && contentLanguage_ == rhs.contentLanguage_ &&
//            expires_ == rhs.expires_;
// }
// bool VolcengineTos::GetObjectBasicOutput::operator!=(const VolcengineTos::GetObjectBasicOutput& rhs) const {
//     return !(rhs == *this);
// }
