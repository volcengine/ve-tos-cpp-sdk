#include <utility>
#include "auth/SignV4.h"
#include "model/object/MultipartUpload.h"
#include "transport/http/HttpClient.h"
#include "TosClientImpl.h"
#include "TosClient.h"
#include "transport/DefaultTransport.h"
#include "transport/TransportConfig.h"
#include "utils/MimeType.h"
#include "model/object/UploadFileInfo.h"
#include "model/object/UploadFileCheckpoint.h"
#include "utils/crc64.h"
#include "model/object/UploadFileCheckpointV2.h"
#include "utils/LogUtils.h"
#include "model/object/DownloadFileCheckpoint.h"
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <thread>
#include <mutex>
#include <queue>

using namespace VolcengineTos;

// 初始化相关函数，分为携带Config和不带Config两种初始化方式
TosClientImpl::TosClientImpl(const std::string& endpoint, const std::string& region, const StaticCredentials& cred) {
    initWithoutConfig(endpoint, region);
    credentials_ = std::make_shared<StaticCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}
TosClientImpl::TosClientImpl(const std::string& endpoint, const std::string& region,
                             const FederationCredentials& cred) {
    initWithoutConfig(endpoint, region);
    credentials_ = std::make_shared<FederationCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}
void TosClientImpl::initWithoutConfig(const std::string& endpoint, const std::string& region) {
    if (endpoint.empty()) {
        if (supportedRegion_.count(region) != 0) {
            init(supportedRegion_[region], region);
        } else {
            init(endpoint, region);
            //            std::cout << "Please Check Region is Supported" << std::endl;
        }
    } else {
        init(endpoint, region);
    }
}
void TosClientImpl::init(const std::string& endpoint, const std::string& region) {
    TransportConfig conf;
    transport_ = std::make_shared<DefaultTransport>(conf);
    config_.setTransportConfig(conf);
    config_.setEndpoint(endpoint);
    config_.setRegion(region);
    initSchemeAndHost(endpoint, region);
}
TosClientImpl::TosClientImpl(const std::string& endpoint, const std::string& region, const StaticCredentials& cred,
                             const ClientConfig& config) {
    initWithConfig(endpoint, region, config);
    credentials_ = std::make_shared<StaticCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}
TosClientImpl::TosClientImpl(const std::string& endpoint, const std::string& region, const FederationCredentials& cred,
                             const ClientConfig& config) {
    initWithConfig(endpoint, region, config);
    credentials_ = std::make_shared<FederationCredentials>(cred);
    signer_ = std::make_shared<SignV4>(credentials_, region);
}
void TosClientImpl::initWithConfig(const std::string& endpoint, const std::string& region, const ClientConfig& config) {
    if (endpoint.empty()) {
        if (supportedRegion_.count(region) != 0) {
            init(supportedRegion_[region], region, config);
        } else {
            init(endpoint, region, config);
            //            std::cout << "Please Check Region is Supported" << std::endl;
        }
    } else {
        init(endpoint, region, config);
    }
}

void TosClientImpl::init(const std::string& endpoint, const std::string& region, const ClientConfig& config) {
    TransportConfig conf;
    // 涉及到 HttpClient 的参数放到这里
    // 需要同步修改 DefaultTransport(const TransportConfig& config) 以传参给 HttpConfig
    conf.setEnableVerifySsl(config.enableVerifySSL);
    conf.setRequestTimeout(config.requestTimeout);
    conf.setConnectTimeout(config.connectionTimeout);
    // 以下参数对应功能还没有实际实现，等后续特性支持时添加参数传递
    conf.setProxyHost(config.proxyHost);
    conf.setProxyPort(config.proxyPort);
    conf.setProxyUsername(config.proxyUsername);
    conf.setProxyPassword(config.proxyPassword);
    conf.setDnsCacheTime(config.dnsCacheTime);
    transport_ = std::make_shared<DefaultTransport>(conf);

    // 保存参数到 config_ 里
    config_.setTransportConfig(conf);
    config_.setEndpoint(endpoint);
    config_.setRegion(region);
    // 涉及到 req/roundTrip 的参数放到这里
    config_.setEnableCrc(config.enableCRC);
    config_.setAutoRecognizeContentType(config.autoRecognizeContentType);
    config_.setMaxRetryCount(config.maxRetryCount);
    initSchemeAndHost(endpoint, region);
}

void TosClientImpl::initSchemeAndHost(const std::string& endpoint, const std::string& region) {
    // set scheme and host
    if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTPS)) {
        scheme_ = http::SchemeHTTPS;
        host_ = endpoint.substr(std::strlen(http::SchemeHTTPS) + 3,
                                endpoint.length() - std::strlen(http::SchemeHTTPS) - 3);
    } else if (StringUtils::startsWithIgnoreCase(endpoint, http::SchemeHTTP)) {
        scheme_ = http::SchemeHTTP;
        host_ = endpoint.substr(std::strlen(http::SchemeHTTP) + 3,
                                endpoint.length() - std::strlen(http::SchemeHTTP) - 3);
    } else {
        scheme_ = http::SchemeHTTP;
        host_ = endpoint;
    }

    if (userAgent_.empty())
        userAgent_ = DefaultUserAgent();
    urlMode_ = URL_MODE_DEFAULT;
}

// 桶名/对象名校验相关函数
std::string isValidBucketName(const std::string& name) {
    if (name.empty() || name.length() < 3 || name.length() > 63) {
        return "invalid bucket name, the length must be [3, 63]";
    }
    for (char c : name) {
        if (!(('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-')) {
            return "invalid bucket name, the character set is illegal";
        }
    }
    if (name[0] == '-' || name[name.length() - 1] == '-') {
        return "invalid bucket name, the bucket name can be neither starting with ' - ' nor ending with ' - '";
    }
    return "";
}

std::string isValidKey(const std::string& key) {
    if (key.empty()) {
        return "invalid object name, the length must be [1, 696]";
    }
    if (!StringUtils::isValidUTF8(key)) {
        return "invalid object name, the character set is illegal";
    }
    if (key[0] == '/' || key[0] == '\\' || key.find("//") != key.npos) {
        return "invalid object name, the object name can not start with '/' or '\\'";
    }
    if (key[0] == '.' && key.length() == 1) {
        return "invalid object name, the object name can not use '.'";
    }
    if (key.length() > 696) {
        return "invalid object name, the length must be [1, 696]";
    }
    return "";
}

std::string isValidKeys(const std::vector<std::string>& keys) {
    for (auto& key : keys) {
        std::string ret = isValidKey(key);
        if (!ret.empty()) {
            return ret;
        }
    }
    return "";
}
std::string isValidBuckets(const std::vector<std::string>& bkts) {
    for (auto& bkt : bkts) {
        std::string ret = isValidBucketName(bkt);
        if (!ret.empty()) {
            return ret;
        }
    }
    return "";
}
std::string isValidNames(const std::string& bucket, const std::vector<std::string>& keys) {
    std::string checkBkt = isValidBucketName(bucket);
    if (checkBkt.empty()) {
        return isValidKeys(keys);
    } else {
        return checkBkt;
    }
}
std::string isValidNames(const std::vector<std::string>& buckets, const std::vector<std::string>& keys) {
    std::string checkBkt = isValidBuckets(buckets);
    if (checkBkt.empty()) {
        return isValidKeys(keys);
    } else {
        return checkBkt;
    }
}
// 设置传输过程中相关参数
void setContentType(RequestBuilder& rb, const std::string& objectKey) {
    std::string contentType = rb.findHeader(http::HEADER_CONTENT_TYPE);
    if (contentType.empty()) {
        // request does not attach content-type
        // auto recognize default, check if user disable it by withAutoRecognizeContentType(false)
        if (rb.isAutoRecognizeContentType()) {
            // set content type before upload
            contentType = MimeType::getMimetypeByObjectKey(objectKey);
            rb.withHeader(http::HEADER_CONTENT_TYPE, contentType);
        }
    }
}
int expectedCode(const RequestBuilder& rb) {
    return rb.getRange().isNull() ? 200 : 206;
}
int expectedCodeV2(const RequestBuilder& rb) {
    const auto& header_ = rb.getHeaders();
    bool exit_range_header =
            header_.count("Range") || header_.count("X-Tos-Copy-Source-Range") || rb.getQuery().count("partNumber");
    return exit_range_header ? 206 : 200;
}

// 将 config 中的参数传递到 req 中
void TosClientImpl::SetCrc64ParmToReq(const std::shared_ptr<TosRequest>& req) {
    if (config_.isEnableCrc()) {
        req->setCheckCrc64(true);
    }
}
// 将 process handler 传递到 req 中
void TosClientImpl::SetProcessHandlerToReq(const std::shared_ptr<TosRequest>& req, DataTransferListener& handler) {
    if (handler.dataTransferStatusChange_ != nullptr) {
        req->setDataTransferListener(handler);
    }
}
void TosClientImpl::SetRateLimiterToReq(const std::shared_ptr<TosRequest>& req,
                                        const std::shared_ptr<RateLimiter>& limiter) {
    if (limiter != nullptr) {
        req->setRataLimiter(limiter);
    }
}
RequestBuilder TosClientImpl::ParamFromConfToRb(RequestBuilder& rb) {
    return rb;
}

// 各个接口具体实现
static void createBucketSetOptionHeader(RequestBuilder& rb, const CreateBucketInput& input) {
    rb.withHeader(HEADER_ACL, input.getAcl());
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE, input.getGrantWrite());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
    rb.withHeader(HEADER_STORAGE_CLASS, input.getStorageClass());
}

Outcome<TosError, CreateBucketOutput> TosClientImpl::createBucket(const CreateBucketInput& input) {
    Outcome<TosError, CreateBucketOutput> res;
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(input.getBucket(), "");
    createBucketSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodPut, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CreateBucketOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setLocation(tosRes.result()->findHeader(http::HEADER_LOCATION));
    res.setSuccess(true);
    res.setR(output);
    return res;
}

static void createBucketSetOptionHeader(RequestBuilder& rb, const CreateBucketV2Input& input) {
    rb.withHeader(HEADER_ACL, ACLTypetoString[input.getAcl()]);
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE, input.getGrantWrite());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
    rb.withHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[input.getStorageClass()]);
    // rb.withHeader(HEADER_AZ_REDUNDANCY, AzRedundancytoString[input.getAzRedundancy()]);
}
Outcome<TosError, CreateBucketV2Output> TosClientImpl::createBucket(const CreateBucketV2Input& input) {
    Outcome<TosError, CreateBucketV2Output> res;
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), "");
    createBucketSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodPut, nullptr);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CreateBucketV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());

    output.setLocation(tosRes.result()->findHeader(http::HEADER_LOCATION));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, HeadBucketOutput> TosClientImpl::headBucket(const std::string& bucket) {
    Outcome<TosError, HeadBucketOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setSuccess(false);
        res.setE(error);
        return res;
    }
    auto req = newBuilder(bucket, "").Build(http::MethodHead, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    HeadBucketOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setRegion(tosRes.result()->findHeader(HEADER_BUCKET_REGION));
    output.setStorageClass(tosRes.result()->findHeader(HEADER_STORAGE_CLASS));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, HeadBucketV2Output> TosClientImpl::headBucket(const HeadBucketV2Input& input) {
    Outcome<TosError, HeadBucketV2Output> res;
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setSuccess(false);
        res.setE(error);
        return res;
    }
    auto req = newBuilder(input.getBucket(), "").Build(http::MethodHead, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    HeadBucketV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setRegion(tosRes.result()->findHeader(HEADER_BUCKET_REGION));
    output.setStorageClass(StringtoStorageClassType[tosRes.result()->findHeader(HEADER_STORAGE_CLASS)]);
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, DeleteBucketOutput> TosClientImpl::deleteBucket(const std::string& bucket) {
    Outcome<TosError, DeleteBucketOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setSuccess(false);
        res.setE(error);
        return res;
    }
    auto req = newBuilder(bucket, "").Build(http::MethodDelete);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    DeleteBucketOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, DeleteBucketOutput> TosClientImpl::deleteBucket(const DeleteBucketInput& input) {
    Outcome<TosError, DeleteBucketOutput> res;
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setSuccess(false);
        res.setE(error);
        return res;
    }
    auto req = newBuilder(input.getBucket(), "").Build(http::MethodDelete);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    DeleteBucketOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, ListBucketsOutput> TosClientImpl::listBuckets(const ListBucketsInput& input) {
    Outcome<TosError, ListBucketsOutput> res;
    auto req = newBuilder("", "").Build(http::MethodGet);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListBucketsOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());

    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, PutBucketPolicyOutput> TosClientImpl::putBucketPolicy(const std::string& bucket,
                                                                        const std::string& policy) {
    Outcome<TosError, PutBucketPolicyOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(bucket, "");
    rb.withQuery("policy", "");
    auto ss = std::make_shared<std::stringstream>(policy);
    auto req = rb.Build(http::MethodPut, ss);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    PutBucketPolicyOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, GetBucketPolicyOutput> TosClientImpl::getBucketPolicy(const std::string& bucket) {
    Outcome<TosError, GetBucketPolicyOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(bucket, "");
    rb.withQuery("policy", "");
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    GetBucketPolicyOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.setPolicy(ss.str());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, DeleteBucketPolicyOutput> TosClientImpl::deleteBucketPolicy(const std::string& bucket) {
    Outcome<TosError, DeleteBucketPolicyOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(bucket, "");
    rb.withQuery("policy", "");
    auto req = rb.Build(http::MethodDelete, nullptr);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    DeleteBucketPolicyOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, GetObjectOutput> TosClientImpl::getObject(const std::string& bucket, const std::string& objectKey) {
    Outcome<TosError, GetObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    this->getObject(rb, res);
    return res;
}
Outcome<TosError, GetObjectOutput> TosClientImpl::getObject(const std::string& bucket, const std::string& objectKey,
                                                            const RequestOptionBuilder& builder) {
    Outcome<TosError, GetObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->getObject(rb, res);
    return res;
}

int64_t TosClientImpl::calContentLength(const std::shared_ptr<std::iostream>& content) {
    int64_t currentPos = content->tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        content->clear();
    }
    content->seekg(0, content->end);
    int64_t size = content->tellg();
    content->seekg(currentPos, content->beg);
    return size;
}

static std::string isValidSSEC(const std::string& SSECAlgorithm, const std::string& SSECKey,
                               const std::string& SSECKeyMd5) {
    if (SSECAlgorithm.empty() && SSECKey.empty() && SSECKeyMd5.empty()) {
        return "";
    }
    if (SSECAlgorithm.empty() || SSECKey.empty() || SSECKeyMd5.empty()) {
        return "invalid encryption-decryption algorithm";
    }
    return "";
}
std::string isValidRange(int64_t start, int64_t end) {
    if (start == 0 && end == 0) {
        return "";
    }
    if (start < 0 || end < 0) {
        return "invalid range format";
    }

    if (end != 0 && end < start) {
        return "invalid range format";
    }
    return "";
}

static void setSSECHeader(const std::string& SSECAlgorithm, const std::string& SSECKey, const std::string& SSECKeyMd5,
                          RequestBuilder& rb) {
    if (SSECAlgorithm.empty() && SSECKey.empty() && SSECKeyMd5.empty()) {
        return;
    }
    rb.withHeader(HEADER_SSE_CUSTOMER_ALGORITHM, SSECAlgorithm);
    rb.withHeader(HEADER_SSE_CUSTOMER_KEY, SSECKey);
    rb.withHeader(HEADER_SSE_CUSTOMER_KEY_MD5, SSECKeyMd5);
}
static void getObjectSetOptionHeader(RequestBuilder& rb, const GetObjectV2Input& input) {
    // range Header
    if (!input.getRange().empty()) {
        rb.withHeader(http::HEADER_RANGE, input.getRange());
    } else if (input.getRangeStart() != 0 || input.getRangeEnd() != 0) {
        HttpRange range_;
        range_.setStart(input.getRangeStart());
        range_.setEnd(input.getRangeEnd());
        rb.withHeader(http::HEADER_RANGE, range_.toString());
    }

    rb.withHeader(http::HEADER_IF_MATCH, input.getIfMatch());
    rb.withHeader(http::HEADER_IF_MODIFIED_SINCE, TimeUtils::transTimeToGmtTime(input.getIfModifiedSince()));
    rb.withHeader(http::HEADER_IF_NONE_MATCH, input.getIfNoneMatch());
    rb.withHeader(http::HEADER_IF_UNMODIFIED_SINCE, TimeUtils::transTimeToGmtTime(input.getIfUnmodifiedSince()));
    setSSECHeader(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5(), rb);

    rb.withQueryCheckEmpty("response-cache-control", input.getResponseCacheControl());
    rb.withQueryCheckEmpty("response-content-disposition", input.getResponseContentDisposition());
    rb.withQueryCheckEmpty("response-content-encoding", input.getResponseContentEncoding());
    rb.withQueryCheckEmpty("response-content-language", input.getResponseContentLanguage());
    rb.withQueryCheckEmpty("response-content-type", input.getResponseContentType());
    rb.withQueryCheckEmpty("response-expires", TimeUtils::transTimeToGmtTime(input.getResponseExpires()));
    rb.withQueryCheckEmpty("versionId", input.getVersionId());
    if (input.getPartNumber() != 0) {
        rb.withQueryCheckEmpty("partNumber", std::to_string(input.getPartNumber()));
    }
}
Outcome<TosError, GetObjectV2Output> TosClientImpl::getObject(const GetObjectV2Input& input,
                                                              std::shared_ptr<uint64_t> hashCrc64ecma) {
    Outcome<TosError, GetObjectV2Output> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidSSEC(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidRange(input.getRangeStart(), input.getRangeEnd());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(input.getBucket(), input.getKey());
    getObjectSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodGet, nullptr);
    // 设置进度条回调
    auto handler = input.getDataTransferListener();
    SetProcessHandlerToReq(req, handler);
    // 设置客户端限速
    auto limiter = input.getRateLimiter();
    SetRateLimiterToReq(req, limiter);
    // 设置Crc64校验信息, 由于downloadFile 需要使用计算出来的 crc64 所以打开校验
    // downloadFile 场景
    if (hashCrc64ecma != nullptr) {
        req->setCheckCrc64(true);
    }
    if (config_.isEnableCrc() && rb.getHeaders().count("Range") == 0) {
        req->setCheckCrc64(true);
    }
    auto tosRes = roundTrip(req, expectedCodeV2(rb));
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    GetObjectV2Output output;
    output.setObjectMetaFromResponse(*tosRes.result());
    output.setContent(tosRes.result()->getContent());
    if (hashCrc64ecma != nullptr) {
        *hashCrc64ecma = tosRes.result()->getHashCrc64Result();
    }
    // crc64校验
    if (config_.isEnableCrc() && rb.getHeaders().count("Range") == 0 && rb.getQuery().count("partNumber") == 0) {
        auto hashcrc64 = std::stoull(tosRes.result()->findHeader(HEADER_CRC64));
        if (tosRes.result()->getHashCrc64Result() != hashcrc64) {
            TosError error;
            error.setIsClientError(true);
            error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    // 断流校验由 libcurl 支持，对应错误码 18
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, GetObjectToFileOutput> TosClientImpl::getObjectToFile(const GetObjectToFileInput& input) {
    Outcome<TosError, GetObjectToFileOutput> res;
    if (input.getFilePath().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("The file path is empty");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    const GetObjectV2Input& basic_input = input.getGetObjectInput();
    auto res_ = this->getObject(basic_input, nullptr);
    if (!res_.isSuccess()) {
        res.setE(res_.error());
        res.setSuccess(false);
        return res;
    }
    GetObjectToFileOutput output;
    output.setGetObjectBasicOutput(res_.result().getGetObjectBasicOutput());
    // write to file
    auto content = std::make_shared<std::fstream>(
            input.getFilePath(), std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
    auto resContent = res_.result().getContent();
    *content << resContent->rdbuf();
    content->close();
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, HeadObjectOutput> TosClientImpl::headObject(const std::string& bucket, const std::string& objectKey) {
    Outcome<TosError, HeadObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    this->headObject(rb, res);
    return res;
}
Outcome<TosError, HeadObjectOutput> TosClientImpl::headObject(const std::string& bucket, const std::string& objectKey,
                                                              const RequestOptionBuilder& builder) {
    Outcome<TosError, HeadObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->headObject(rb, res);
    return res;
}
static void headObjectSetOptionHeader(RequestBuilder& rb, const HeadObjectV2Input& input) {
    rb.withHeader(http::HEADER_IF_MATCH, input.getIfMatch());
    rb.withHeader(http::HEADER_IF_MODIFIED_SINCE, TimeUtils::transTimeToGmtTime(input.getIfModifiedSince()));
    rb.withHeader(http::HEADER_IF_NONE_MATCH, input.getIfNoneMatch());
    rb.withHeader(http::HEADER_IF_UNMODIFIED_SINCE, TimeUtils::transTimeToGmtTime(input.getIfUnmodifiedSince()));
    setSSECHeader(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5(), rb);
    rb.withQueryCheckEmpty("versionId", input.getVersionId());
}
Outcome<TosError, HeadObjectV2Output> TosClientImpl::headObject(const HeadObjectV2Input& input) {
    Outcome<TosError, HeadObjectV2Output> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidSSEC(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    headObjectSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodHead, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    HeadObjectV2Output output;
    output.fromResponse(*tosRes.result());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, DeleteObjectOutput> TosClientImpl::deleteObject(const std::string& bucket,
                                                                  const std::string& objectKey) {
    Outcome<TosError, DeleteObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    this->deleteObject(rb, res);
    return res;
}
Outcome<TosError, DeleteObjectOutput> TosClientImpl::deleteObject(const std::string& bucket,
                                                                  const std::string& objectKey,
                                                                  const RequestOptionBuilder& builder) {
    Outcome<TosError, DeleteObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->deleteObject(rb, res);
    return res;
}
Outcome<TosError, DeleteObjectOutput> TosClientImpl::deleteObject(const DeleteObjectInput& input) {
    Outcome<TosError, DeleteObjectOutput> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());

    rb.withQueryCheckEmpty("versionId", input.getVersionID());

    auto req = rb.Build(http::MethodDelete, nullptr);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    DeleteObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setDeleteMarker(tosRes.result()->findHeader(HEADER_DELETE_MARKER) == "true");
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClientImpl::deleteMultiObjects(const std::string& bucket,
                                                                              DeleteMultiObjectsInput& input) {
    Outcome<TosError, DeleteMultiObjectsOutput> res;
    if (input.getObjectTobeDeleteds().empty()) {
        TosError error;
        error.setMessage("Parameter objects should not be empty");
        error.setIsClientError(true);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string data = input.toJsonString();
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    auto rb = newBuilder(bucket, "");
    this->deleteMultiObjects(rb, data, dataMd5, res);
    return res;
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClientImpl::deleteMultiObjects(const std::string& bucket,
                                                                              DeleteMultiObjectsInput& input,
                                                                              const RequestOptionBuilder& builder) {
    Outcome<TosError, DeleteMultiObjectsOutput> res;
    if (input.getObjectTobeDeleteds().empty()) {
        TosError error;
        error.setMessage("tos: Parameter objects should not be empty");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string data = input.toJsonString();
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    auto rb = newBuilder(bucket, "", builder);
    this->deleteMultiObjects(rb, data, dataMd5, res);
    return res;
}
Outcome<TosError, DeleteMultiObjectsOutput> TosClientImpl::deleteMultiObjects(DeleteMultiObjectsInput& input) {
    Outcome<TosError, DeleteMultiObjectsOutput> res;
    if (input.getObjectTobeDeleteds().empty()) {
        TosError error;
        error.setMessage("tos: Parameter objects should not be empty");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    std::string data = input.toJsonString();
    std::string dataMd5 = CryptoUtils::md5Sum(data);
    auto rb = newBuilder(input.getBucket(), "");

    rb.withHeader(http::HEADER_CONTENT_MD5, dataMd5);
    rb.withQuery("delete", "");
    //    std::cout << data << std::endl;
    //    std::cout << dataMd5 << std::endl;
    auto req = rb.Build(http::MethodPost, std::make_shared<std::stringstream>(data));
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    std::stringstream out;
    out << tosRes.result()->getContent()->rdbuf();
    DeleteMultiObjectsOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.fromJsonString(out.str());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, PutObjectOutput> TosClientImpl::putObject(const std::string& bucket, const std::string& objectKey,
                                                            const std::shared_ptr<std::iostream>& content) {
    Outcome<TosError, PutObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    setContentType(rb, objectKey);
    auto req = rb.Build(http::MethodPut, content);
    this->putObject(req, res);
    return res;
}
Outcome<TosError, PutObjectOutput> TosClientImpl::putObject(const std::string& bucket, const std::string& objectKey,
                                                            const std::shared_ptr<std::iostream>& content,
                                                            const RequestOptionBuilder& builder) {
    Outcome<TosError, PutObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    setContentType(rb, objectKey);
    auto req = rb.Build(http::MethodPut, content);
    this->putObject(req, res);
    return res;
}

static void setMetaHeader(const std::map<std::string, std::string>& meta, RequestBuilder& rb) {
    for (auto& meta_ : meta) {
        rb.withHeader(VolcengineTos::HEADER_META_PREFIX + CryptoUtils::UrlEncodeChinese(meta_.first),
                      CryptoUtils::UrlEncodeChinese(meta_.second));
    }
}
static void putObjectSetOptionHeader(RequestBuilder& rb, const PutObjectBasicInput& basic_input) {
    rb.withHeader(http::HEADER_CONTENT_MD5, basic_input.getContentMd5());
    rb.withHeader(http::HEADER_CONTENT_TYPE, basic_input.getContentType());
    rb.withHeader(http::HEADER_CACHE_CONTROL, basic_input.getCacheControl());

    rb.withHeader(http::HEADER_EXPIRES, TimeUtils::transTimeToGmtTime(basic_input.getExpires()));
    auto UrlEncode_disposition = CryptoUtils::UrlEncodeChinese(basic_input.getContentDisposition());
    rb.withHeader(http::HEADER_CONTENT_DISPOSITION, UrlEncode_disposition);

    rb.withHeader(http::HEADER_CONTENT_ENCODING, basic_input.getContentEncoding());
    rb.withHeader(http::HEADER_CONTENT_LANGUAGE, basic_input.getContentLanguage());
    rb.withHeader(HEADER_ACL, ACLTypetoString[basic_input.getAcl()]);
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, basic_input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, basic_input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, basic_input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, basic_input.getGrantWriteAcp());
    setMetaHeader(basic_input.getMeta(), rb);
    setSSECHeader(basic_input.getSsecAlgorithm(), basic_input.getSsecKey(), basic_input.getSsecKeyMd5(), rb);
    rb.withHeader(HEADER_WEBSITE_REDIRECT_LOCATION, basic_input.getWebsiteRedirectLocation());
    rb.withHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[basic_input.getStorageClass()]);
    rb.withHeader(HEADER_SSE, basic_input.getServerSideEncryption());
}
Outcome<TosError, PutObjectV2Output> TosClientImpl::putObject(const PutObjectV2Input& input) {
    Outcome<TosError, PutObjectV2Output> res;
    const auto& putObjectBasicInput_ = input.getPutObjectBasicInput();
    std::string check = isValidNames(putObjectBasicInput_.getBucket(), {putObjectBasicInput_.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidSSEC(putObjectBasicInput_.getSsecAlgorithm(), putObjectBasicInput_.getSsecKey(),
                        putObjectBasicInput_.getSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(putObjectBasicInput_.getBucket(), putObjectBasicInput_.getKey());

    if (config_.isAutoRecognizeContentType()) {
        setContentType(rb, putObjectBasicInput_.getKey());
    }

    putObjectSetOptionHeader(rb, putObjectBasicInput_);
    auto req = rb.Build(http::MethodPut, input.getContent());
    // 设置回调
    auto handler = putObjectBasicInput_.getDataTransferListener();
    auto limiter = putObjectBasicInput_.getRateLimiter();
    SetProcessHandlerToReq(req, handler);
    SetRateLimiterToReq(req, limiter);
    SetCrc64ParmToReq(req);
    // 设置funcName
    req->setFuncName(__func__);
    req->setContentOffset(req->getContent()->tellg());
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    if (config_.isEnableCrc()) {
        auto hashcrc64 = std::stoull(tosRes.result()->findHeader(HEADER_CRC64));
        if (tosRes.result()->getHashCrc64Result() != hashcrc64) {
            TosError error;
            error.setIsClientError(true);
            error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    PutObjectV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setETag(tosRes.result()->findHeader(http::HEADER_ETAG));
    output.setSsecAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSsecKeyMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setHashCrc64ecma(std::stoull(tosRes.result()->findHeader(HEADER_CRC64)));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
std::string isValidFilePath(const std::string& filePath) {
    struct stat ufs {};
    if (stat(filePath.c_str(), &ufs) == 0) {
        if (ufs.st_mode & S_IFDIR) {
            return "invalid file path, the file does not exist";
        } else {
            return "";
        }
    }
    return "invalid file path, the file does not exist";
}
Outcome<TosError, PutObjectFromFileOutput> TosClientImpl::putObjectFromFile(const PutObjectFromFileInput& input) {
    Outcome<TosError, PutObjectFromFileOutput> res;
    std::string check = isValidFilePath(input.getFilePath());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto content = std::make_shared<std::fstream>(input.getFilePath(), std::ios::in | std::ios_base::binary);
    if (!content->good()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("open file failed");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    content->seekg(0, content->beg);
    PutObjectV2Input input_(input.getPutObjectBasicInput(), content);
    auto res_ = this->putObject(input_);
    if (!res_.isSuccess()) {
        res.setE(res_.error());
        res.setSuccess(false);
        return res;
    }
    PutObjectFromFileOutput output_file;
    output_file.setPutObjectV2Output(res_.result());
    res.setSuccess(true);
    res.setR(output_file);
    return res;
}

Outcome<TosError, int> validateInput(const std::string& bucket, const std::string& key, const int64_t partSize,
                                     const int taskNum) {
    Outcome<TosError, int> ret;
    TosError error;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        error.setIsClientError(true);
        error.setMessage(check);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    check = isValidKey(key);
    if (!check.empty()) {
        error.setIsClientError(true);
        error.setMessage(check);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    if (partSize < 5 * 1024 * 1024 || partSize > 5 * 1024 * 1024 * 1024L) {
        error.setMessage("invalid part size, the size must be [5242880, 5368709120]");
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    int taskNum_ = taskNum;
    if (taskNum_ > 1000)
        taskNum_ = 1000;
    if (taskNum_ < 1)
        taskNum_ = 1;
    ret.setR(taskNum_);
    ret.setSuccess(true);
    return ret;
}
std::string getCheckpointPath(const std::string& bucket, const std::string& key, const std::string& checkPointFile,
                              const std::string uploadFilePath) {
    std::stringstream ret;
    std::string objKey(StringUtils::stringReplace(key, "/", "_"));
    if (checkPointFile.empty()) {
        ret << uploadFilePath << "." << bucket << "." << objKey << ".upload";
        return ret.str();
    } else {
        struct stat cfs {};
        if (stat(checkPointFile.c_str(), &cfs) == 0) {
            if (cfs.st_mode & S_IFDIR) {
                ret << checkPointFile << "/" << bucket << "." << objKey << ".upload";
                return ret.str();
            } else {
                ret << checkPointFile;
                return ret.str();
            }
        }
    }
    return "";
}
Outcome<TosError, UploadFileInfo> getUploadFileInfo(const std::string& uploadFilePath) {
    Outcome<TosError, UploadFileInfo> ret;
    UploadFileInfo ufi;
    TosError e;
    struct stat ufs {};
    if (stat(uploadFilePath.c_str(), &ufs) == 0) {
        if (ufs.st_mode & S_IFDIR) {
            e.setMessage("invalid file path, the file does not exist");
            ret.setE(e);
            ret.setSuccess(false);
            return ret;
        } else {
            ufi.setFilePath(uploadFilePath);
#ifdef __APPLE__
            ufi.setLastModified(ufs.st_mtimespec.tv_sec);
#else
            ufi.setLastModified(ufs.st_mtime);
#endif
            ufi.setFileSize(ufs.st_size);
            ret.setR(ufi);
            ret.setSuccess(true);
        }
    } else {
        e.setMessage("invalid file path, the file does not exist");
        e.setIsClientError(true);
        ret.setE(e);
        ret.setSuccess(false);
        return ret;
    }
    return ret;
}
Outcome<TosError, UploadFileInfoV2> getUploadFileInfoV2(const std::string& uploadFilePath) {
    Outcome<TosError, UploadFileInfoV2> ret;
    UploadFileInfoV2 ufi;
    TosError e;
    struct stat ufs {};
    if (stat(uploadFilePath.c_str(), &ufs) == 0) {
        if (ufs.st_mode & S_IFDIR) {
            e.setMessage("invalid file path, the file does not exist");
            e.setIsClientError(true);
            ret.setE(e);
            ret.setSuccess(false);
            return ret;
        } else {
#ifdef __APPLE__
            ufi.setLastModified(ufs.st_mtimespec.tv_sec);
#else
            ufi.setLastModified(ufs.st_mtime);
#endif
            ufi.setFileSize(ufs.st_size);
            ret.setR(ufi);
            ret.setSuccess(true);
        }
    } else {
        e.setMessage("invalid file path, the file does not exist");
        e.setIsClientError(true);
        ret.setE(e);
        ret.setSuccess(false);
        return ret;
    }
    return ret;
}
UploadFileCheckpoint loadCheckpointFromFile(const std::string& checkpointFilePath) {
    UploadFileCheckpoint ufc;
    ufc.load();
    ufc.setCheckpointFilePath(checkpointFilePath);
    return ufc;
}
UploadFileCheckpointV2 loadCheckpointV2FromFile(const std::string& checkpointFilePath) {
    UploadFileCheckpointV2 ufc;
    ufc.load(checkpointFilePath);
    return ufc;
}
bool deleteCheckpointFile(const std::string& checkpointFilePath) {
    return remove(checkpointFilePath.c_str());
}
Outcome<TosError, std::vector<UploadFilePartInfo>> getPartInfoFromFile(int64_t uploadFileSize, int64_t partSize) {
    Outcome<TosError, std::vector<UploadFilePartInfo>> ret;
    auto partNum = uploadFileSize / partSize;
    auto lastPartSize = uploadFileSize % partSize;
    TosError error;
    if (lastPartSize != 0)
        partNum++;
    if (partNum > 10000) {
        error.setMessage("The split file parts number is larger than 10000, please increase your part size");
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    std::vector<UploadFilePartInfo> partInfoList;
    for (int i = 0; i < partNum; ++i) {
        UploadFilePartInfo info;
        info.setPartNum(i + 1);
        info.setOffset(i * partSize);
        if (i < partNum - 1) {
            info.setPartSize(partSize);
        } else {
            info.setPartSize(lastPartSize);
        }
        partInfoList.emplace_back(info);
    }
    ret.setR(partInfoList);
    ret.setSuccess(true);
    return ret;
}
Outcome<TosError, std::vector<UploadFilePartInfoV2>> getPartInfoFromFileV2(int64_t uploadFileSize, int64_t partSize) {
    Outcome<TosError, std::vector<UploadFilePartInfoV2>> ret;
    auto partNum = uploadFileSize / partSize;
    auto lastPartSize = uploadFileSize % partSize;
    TosError error;
    if (lastPartSize != 0)
        partNum++;
    if (partNum > 10000) {
        error.setMessage("unsupported part number, the maximum is 10000");
        error.setIsClientError(true);
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    std::vector<UploadFilePartInfoV2> partInfoList;
    for (int i = 0; i < partNum; ++i) {
        UploadFilePartInfoV2 info;
        info.setPartNum(i + 1);
        info.setOffset(i * partSize);
        if (i < partNum - 1) {
            info.setPartSize(partSize);
        } else {
            info.setPartSize(lastPartSize);
        }
        partInfoList.emplace_back(info);
    }
    ret.setR(partInfoList);
    ret.setSuccess(true);
    return ret;
}
Outcome<TosError, UploadFileCheckpoint> TosClientImpl::initCheckpoint(const std::string& bucket,
                                                                      const UploadFileInput& input,
                                                                      const UploadFileInfo& info,
                                                                      const std::string& checkpointFilePath,
                                                                      const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadFileCheckpoint> ret;
    TosError error;
    auto partInfo = getPartInfoFromFile(info.getFileSize(), input.getPartSize());
    if (!partInfo.isSuccess()) {
        error.setMessage(partInfo.error().getMessage());
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    UploadFileCheckpoint checkpoint;
    checkpoint.setBucket(bucket);
    checkpoint.setKey(input.getObjectKey());
    checkpoint.setFileInfo(info);
    checkpoint.setUploadFilePartInfoList(partInfo.result());
    checkpoint.setCheckpointFilePath(checkpointFilePath);
    auto output = this->createMultipartUpload(bucket, input.getObjectKey());
    if (!output.isSuccess()) {
        error.setMessage(output.error().getMessage());
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    checkpoint.setUploadId(output.result().getUploadId());
    ret.setSuccess(true);
    ret.setR(checkpoint);
    return ret;
}

void initUploadEvent(const UploadFileV2Input& input, const std::string& checkpointFilePath, const std::string& uploadId,
                     std::shared_ptr<UploadEvent> event) {
    event->type_ = 0;
    event->error_ = false;
    event->key_ = input.getCreateMultipartUploadInput().getKey();
    event->bucket_ = input.getCreateMultipartUploadInput().getBucket();
    event->uploadId_ = std::make_shared<std::string>(uploadId);
    event->checkpointFile_ = std::make_shared<std::string>(checkpointFilePath);
    UploadPartInfo partInfo{};
    partInfo.partNumber_ = 0;
    partInfo.eTag_ = nullptr;
    partInfo.partSize_ = input.getPartSize();
    partInfo.hashCrc64ecma_ = nullptr;
    partInfo.offset_ = 0;
    event->uploadPartInfo_ = std::make_shared<UploadPartInfo>(partInfo);
}

void uploadEventCreateMultipartUploadSucceed(const std::shared_ptr<UploadEvent>& event,
                                             const UploadEventChange& eventChange) {
    event->type_ = 1;
    event->uploadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventCreateMultipartUploadFailed(const std::shared_ptr<UploadEvent>& event,
                                            const UploadEventChange& eventChange) {
    event->type_ = 2;
    event->error_ = true;
    event->uploadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventUploadPartSucceed(const std::shared_ptr<UploadEvent>& event, const UploadEventChange& eventChange,
                                  const UploadPartInfo& partInfo) {
    event->type_ = 3;
    event->uploadPartInfo_ = std::make_shared<UploadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventUploadPartFailed(const std::shared_ptr<UploadEvent>& event, const UploadEventChange& eventChange,
                                 const UploadPartInfo& partInfo) {
    event->type_ = 4;
    event->error_ = true;
    event->uploadPartInfo_ = std::make_shared<UploadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventUploadPartAborted(const std::shared_ptr<UploadEvent>& event, const UploadEventChange& eventChange,
                                  const UploadPartInfo& partInfo) {
    event->type_ = 5;
    event->error_ = true;
    event->uploadPartInfo_ = std::make_shared<UploadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventCompleteMultipartUploadSucceed(const std::shared_ptr<UploadEvent>& event,
                                               const UploadEventChange& eventChange) {
    event->type_ = 6;
    event->uploadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void uploadEventCompleteMultipartUploadFailed(const std::shared_ptr<UploadEvent>& event,
                                              const UploadEventChange& eventChange) {
    event->type_ = 7;
    event->error_ = true;
    event->uploadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
Outcome<TosError, UploadFileCheckpointV2> TosClientImpl::initCheckpoint(
        const std::string& bucket, const std::string& key, const UploadFileV2Input& input, const UploadFileInfoV2& info,
        const std::string& checkpointFilePath, std::shared_ptr<UploadEvent> event) {
    Outcome<TosError, UploadFileCheckpointV2> ret;
    TosError error;

    auto partInfo = getPartInfoFromFileV2(info.getFileSize(), input.getPartSize());
    if (!partInfo.isSuccess()) {
        error.setMessage(partInfo.error().getMessage());
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    UploadFileCheckpointV2 checkpoint;
    checkpoint.setBucket(bucket);
    checkpoint.setKey(key);
    checkpoint.setPartSize(input.getPartSize());
    checkpoint.setSseKeyMd5(input.getCreateMultipartUploadInput().getSsecKeyMd5());
    checkpoint.setSseAlgorithm(input.getCreateMultipartUploadInput().getSsecAlgorithm());
    checkpoint.setEncodingType(input.getCreateMultipartUploadInput().getEncodingType());
    checkpoint.setFilePath(input.getFilePath());
    checkpoint.setFileInfo(info);
    checkpoint.setPartsInfo(partInfo.result());
    auto output = this->createMultipartUpload(input.getCreateMultipartUploadInput());
    if (!output.isSuccess()) {
        uploadEventCreateMultipartUploadFailed(event, input.getUploadEventListener().eventChange_);
        error.setMessage(output.error().getMessage());
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    *event->uploadId_ = output.result().getUploadId();
    uploadEventCreateMultipartUploadSucceed(event, input.getUploadEventListener().eventChange_);
    checkpoint.setUploadId(output.result().getUploadId());

    ret.setSuccess(true);
    ret.setR(checkpoint);

    return ret;
}
Outcome<TosError, UploadFileCheckpoint> TosClientImpl::getCheckpoint(const std::string& bucket,
                                                                     const UploadFileInput& input,
                                                                     const UploadFileInfo& fileInfo,
                                                                     const std::string& checkpointFilePath,
                                                                     const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadFileCheckpoint> ret;
    UploadFileCheckpoint checkpoint;
    if (input.isEnableCheckpoint()) {
        checkpoint = loadCheckpointFromFile(checkpointFilePath);
    } else {
        deleteCheckpointFile(checkpointFilePath);
    }
    bool valid = checkpoint.isValid(fileInfo.getFileSize(), fileInfo.getLastModified(), bucket, input.getObjectKey(),
                                    input.getUploadFilePath());
    if (!valid) {
        deleteCheckpointFile(checkpointFilePath);
        return this->initCheckpoint(bucket, input, fileInfo, checkpointFilePath, builder);
    }
    ret.setR(checkpoint);
    ret.setSuccess(true);
    return ret;
}
std::shared_ptr<std::fstream> getContentFromFile(const std::string& filePath) {
    return std::make_shared<std::fstream>(filePath, std::ios::in | std::ios::binary);
}

Outcome<TosError, UploadFileOutput> TosClientImpl::uploadPartConcurrent(const UploadFileInput& input,
                                                                        UploadFileCheckpoint checkpoint,
                                                                        const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadFileOutput> ret;
    TosError error;
    std::vector<Outcome<TosError, UploadPartOutput>> uploadedOutputs;
    uploadedOutputs.reserve(checkpoint.getUploadFilePartInfoList().size());
    std::vector<UploadFilePartInfo> toUpload = checkpoint.getUploadFilePartInfoList();
    std::vector<std::thread> threadPool;
    std::mutex lock_;

    for (int i = 0; i < input.getTaskNum(); i++) {
        auto res = std::thread([&]() {
            while (true) {
                UploadFilePartInfo part;
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (toUpload.empty())
                        break;
                    part = toUpload.front();
                    toUpload.erase(toUpload.begin());
                }

                if (part.isCompleted()) {
                    Outcome<TosError, UploadPartOutput> uploadedPart;
                    uploadedPart.setSuccess(true);
                    uploadedPart.setR(part.getPart());
                    {
                        std::lock_guard<std::mutex> lck(lock_);
                        uploadedOutputs.emplace_back(uploadedPart);
                    }
                    continue;
                }

                std::shared_ptr<std::iostream> content = getContentFromFile(checkpoint.getFileInfo().getFilePath());
                content->seekg(part.getOffset(), content->beg);

                UploadPartInput upi;
                upi.setKey(checkpoint.getKey());
                upi.setUploadId(checkpoint.getUploadId());
                upi.setPartNumber(part.getPartNum());
                upi.setPartSize(part.getPartSize());
                upi.setContent(content);
                auto res = this->uploadPart(checkpoint.getBucket(), upi, builder);

                if (res.isSuccess()) {
                    part.setIsCompleted(true);
                    part.setPart(res.result());
                    checkpoint.setUploadFilePartInfoByIdx(part, part.getPartNum() - 1);
                    if (input.isEnableCheckpoint()) {
                        {
                            std::lock_guard<std::mutex> lck(lock_);
                            checkpoint.dump();
                        }
                    }
                }
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    uploadedOutputs.emplace_back(res);
                }
            }
        });
        threadPool.emplace_back(std::move(res));
    }
    for (auto& worker : threadPool) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    std::vector<UploadPartOutput> uploadedList;
    for (auto& i : uploadedOutputs) {
        if (!i.isSuccess()) {
            AbortMultipartUploadInput abort;
            abort.setKey(checkpoint.getKey());
            abort.setUploadId(checkpoint.getUploadId());
            this->abortMultipartUpload(checkpoint.getBucket(), abort);
            // 静默abort，用户不感知
            ret.setSuccess(false);
            error.setMessage("upload part failed");
            ret.setE(error);
            return ret;
        }
        uploadedList.emplace_back(i.result());
    }
    CompleteMultipartUploadInput complete(checkpoint.getKey(), checkpoint.getUploadId(), uploadedList);
    auto output = this->completeMultipartUpload(checkpoint.getBucket(), complete);
    if (!output.isSuccess()) {
        ret.setSuccess(false);
        error.setMessage("complete multi part failed");
        ret.setE(error);
        return ret;
    }
    // 合并失败，根据情况选择是否删除
    if (input.isEnableCheckpoint()) {
        deleteCheckpointFile(checkpoint.getCheckpointFilePath());
    }
    UploadFileOutput ufo;
    ufo.setUploadId(checkpoint.getUploadId());
    ufo.setBucket(checkpoint.getBucket());
    ufo.setObjectKey(checkpoint.getKey());
    ufo.setOutput(output.result());
    ret.setSuccess(true);
    ret.setR(ufo);
    return ret;
}
Outcome<TosError, UploadFileOutput> TosClientImpl::uploadFile(const std::string& bucket, const UploadFileInput& input,
                                                              const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadFileOutput> res;
    TosError error;
    auto check = validateInput(bucket, input.getObjectKey(), input.getPartSize(), input.getTaskNum());
    if (!check.isSuccess()) {
        error.setMessage(check.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string checkpointFilePath;
    if (input.isEnableCheckpoint()) {
        checkpointFilePath =
                getCheckpointPath(bucket, input.getObjectKey(), input.getCheckpointFile(), input.getUploadFilePath());
        if (checkpointFilePath.empty()) {
            error.setMessage("The file is not found in the specific path " + input.getCheckpointFile());
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    auto ufi = getUploadFileInfo(input.getUploadFilePath());
    if (!ufi.isSuccess()) {
        error.setMessage(ufi.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto cp = getCheckpoint(bucket, input, ufi.result(), checkpointFilePath, builder);
    if (!cp.isSuccess()) {
        error.setMessage(cp.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    return uploadPartConcurrent(input, cp.result(), builder);
}
Outcome<TosError, UploadFileCheckpointV2> TosClientImpl::getCheckpoint(const UploadFileV2Input& input,
                                                                       const UploadFileInfoV2& fileInfo,
                                                                       const std::string& checkpointFilePath,
                                                                       std::shared_ptr<UploadEvent> event) {
    Outcome<TosError, UploadFileCheckpointV2> ret;
    UploadFileCheckpointV2 checkpoint;
    auto bucket = input.getCreateMultipartUploadInput().getBucket();
    auto key = input.getCreateMultipartUploadInput().getKey();
    if (input.isEnableCheckpoint()) {
        checkpoint = loadCheckpointV2FromFile(checkpointFilePath);
    } else {
        deleteCheckpointFile(checkpointFilePath);
    }
    bool valid = checkpoint.isValid(fileInfo.getFileSize(), fileInfo.getLastModified(), bucket, key);
    if (!valid) {
        deleteCheckpointFile(checkpointFilePath);
        return this->initCheckpoint(bucket, key, input, fileInfo, checkpointFilePath, event);
    }
    ret.setR(checkpoint);
    ret.setSuccess(true);
    return ret;
}

Outcome<TosError, UploadFileV2Output> TosClientImpl::uploadPartConcurrent(const UploadFileV2Input& input,
                                                                          UploadFileCheckpointV2 checkpoint,
                                                                          const std::string& checkpointFilePath,
                                                                          std::shared_ptr<UploadEvent> event) {
    Outcome<TosError, UploadFileV2Output> ret;
    TosError error;
    std::vector<UploadFilePartInfoV2> toUpload = checkpoint.getPartsInfo();
    std::vector<std::thread> threadPool;
    std::mutex lock_;
    int64_t partSize_ = input.getPartSize();
    auto eventChange = input.getUploadEventListener().eventChange_;
    auto cancel = input.getCancelHook();
    std::atomic<bool> isAbort(false);
    std::atomic<bool> isSuccess(true);
    auto logger = LogUtils::GetLogger();

    // 进度条相关参数
    UploadDownloadFileProcessStat processStat;
    auto pProcessStat = &processStat;
    auto process = input.getDataTransferListener();
    if (process.dataTransferStatusChange_ != nullptr) {
        std::mutex processLock;
        pProcessStat->dataTransferListener_.dataTransferStatusChange_ = process.dataTransferStatusChange_;
        pProcessStat->dataTransferListener_.userData_ = process.userData_;
        pProcessStat->userData = (void*)pProcessStat;
        pProcessStat->totalBytes_ = checkpoint.getFileInfo().getFileSize();
    }
    for (int i = 0; i < input.getTaskNum(); i++) {
        auto res = std::thread([&]() {
            while (true) {
                // 开始时检查是否需要中断任务
                if (cancel != nullptr) {
                    if (cancel->isCancel()) {
                        break;
                    }
                }
                // 发生严重错误，中断任务
                if (isAbort) {
                    break;
                }
                // 任务队列中取part
                UploadFilePartInfoV2 part;
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (toUpload.empty())
                        break;
                    part = toUpload.front();
                    toUpload.erase(toUpload.begin());
                }

                if (part.isCompleted()) {
                    continue;
                }
                // 基于 uploadPartFromFile 上传数据
                UploadPartFromFileInput upi(checkpoint.getBucket(), checkpoint.getKey(), checkpoint.getUploadId(),
                                            part.getPartNum(), input.getFilePath(), part.getOffset(),
                                            part.getPartSize());

                auto upiBasic = upi.getUploadPartBasicInput();
                // 设置限流
                upiBasic.setRateLimiter(input.getRateLimiter());
                // 设置回调
                if (process.dataTransferStatusChange_ != nullptr) {
                    upiBasic.setDataTransferListener({UploadDownloadFileProcessCallback, (void*)pProcessStat});
                }
                upi.setUploadPartBasicInput(upiBasic);
                auto partHashCrc64ecma = std::make_shared<uint64_t>(0);
                auto res = this->uploadPartFromFile(upi, partHashCrc64ecma);

                // 下载后检查是否需要中断任务
                if (cancel != nullptr) {
                    if (cancel->isCancel()) {
                        break;
                    }
                }

                if (res.isSuccess()) {
                    // 更新 checkpoint 信息, 把更新后的part放到队列中
                    part.setIsCompleted(true);
                    part.setETag(res.result().getUploadPartV2Output().getETag());
                    part.setHashCrc64Result(*partHashCrc64ecma);
                    checkpoint.setUploadFilePartInfoByIdx(part, part.getPartNum() - 1);
                    // 事件通知
                    UploadPartInfo partInfo{};
                    partInfo.partNumber_ = part.getPartNum();
                    partInfo.offset_ = part.getOffset();
                    partInfo.hashCrc64ecma_ = partHashCrc64ecma;
                    partInfo.partSize_ = partSize_;
                    partInfo.eTag_ = std::make_shared<std::string>(res.result().getUploadPartV2Output().getETag());
                    {
                        std::lock_guard<std::mutex> lck(lock_);
                        uploadEventUploadPartSucceed(event, eventChange, partInfo);
                        if (input.isEnableCheckpoint()) {
                            checkpoint.dump(checkpointFilePath);
                        }
                    }

                } else {
                    // 失败
                    auto statusCode = res.error().getStatusCode();
                    if (statusCode == 403 || statusCode == 404 || statusCode == 405) {
                        // 事件通知
                        UploadPartInfo partInfo{};
                        partInfo.partNumber_ = part.getPartNum();
                        partInfo.offset_ = part.getOffset();
                        partInfo.hashCrc64ecma_ = partHashCrc64ecma;
                        partInfo.partSize_ = partSize_;
                        partInfo.eTag_ = std::make_shared<std::string>(res.result().getUploadPartV2Output().getETag());
                        {
                            std::lock_guard<std::mutex> lck(lock_);
                            uploadEventUploadPartAborted(event, eventChange, partInfo);
                        }
                        // 出现 403、404、405 错误需要中断整个断点续传任务
                        isAbort = true;
                        break;
                    }
                    // 事件通知
                    UploadPartInfo partInfo{};
                    partInfo.partNumber_ = part.getPartNum();
                    partInfo.offset_ = part.getOffset();
                    partInfo.hashCrc64ecma_ = partHashCrc64ecma;
                    partInfo.partSize_ = partSize_;
                    partInfo.eTag_ = std::make_shared<std::string>(res.result().getUploadPartV2Output().getETag());
                    {
                        std::lock_guard<std::mutex> lck(lock_);
                        uploadEventUploadPartFailed(event, eventChange, partInfo);
                    }
                    isSuccess = false;
                }
            }
        });
        threadPool.emplace_back(std::move(res));
    }
    for (auto& worker : threadPool) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    // 需要 abort 掉任务的场景
    if (isAbort) {
        AbortMultipartUploadInput abort(checkpoint.getBucket(), checkpoint.getKey(), checkpoint.getUploadId());
        auto abortRes = this->abortMultipartUpload(abort);
        if (!abortRes.isSuccess()) {
            if (logger != nullptr) {
                logger->info("abort multipart upload failed");
            }
        }

        ret.setSuccess(false);
        error.setIsClientError(true);
        error.setMessage("the task is canceled");
        ret.setE(error);
        return ret;
    }
    if (cancel != nullptr) {
        // 发生错误
        if (cancel->isCancel()) {
            // 发生严重错误
            if (cancel->isAbortFunc()) {
                AbortMultipartUploadInput abort(checkpoint.getBucket(), checkpoint.getKey(), checkpoint.getUploadId());
                auto abortRes = this->abortMultipartUpload(abort);
                if (!abortRes.isSuccess()) {
                    if (logger != nullptr) {
                        logger->info("abort multipart upload failed");
                    }
                }
            }
            ret.setSuccess(false);
            error.setIsClientError(true);
            error.setMessage("the task is canceled");
            ret.setE(error);
            return ret;
        }
    }

    // 有的段下载发生错误，直接返回
    if (!isSuccess) {
        error.setMessage("some parts are upload incorrectly, you can try again");
        error.setIsClientError(true);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    std::vector<UploadedPartV2> parts = {};
    for (auto& i : checkpoint.getPartsInfo()) {
        UploadedPartV2 part(i.getPartNum(), i.getETag());
        parts.emplace_back(part);
    }
    CompleteMultipartUploadV2Input complete(checkpoint.getBucket(), checkpoint.getKey(), checkpoint.getUploadId(),
                                            parts);
    auto output = this->completeMultipartUpload(complete);
    if (!output.isSuccess()) {
        uploadEventCompleteMultipartUploadFailed(event, eventChange);
        ret.setSuccess(false);
        error.setMessage("complete multi part failed");
        ret.setE(error);
        return ret;
    }
    // 合并成功，删除 checkpoint 文件
    uploadEventCompleteMultipartUploadSucceed(event, eventChange);
    if (input.isEnableCheckpoint()) {
        deleteCheckpointFile(checkpointFilePath);
    }
    // 校验 CRC64
    auto uploadedParts = checkpoint.getPartsInfo();
    uint64_t localCRC64 = uploadedParts[0].getHashCrc64Result();
    for (size_t i = 1; i < uploadedParts.size(); i++) {
        localCRC64 =
                CRC64::CombineCRC(localCRC64, uploadedParts[i].getHashCrc64Result(), uploadedParts[i].getPartSize());
    }

    uint64_t resCrc64 = output.result().getHashCrc64ecma();
    if (localCRC64 != resCrc64) {
        error.setIsClientError(true);
        error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }

    UploadFileV2Output ufo;
    ufo.setRequestInfo(output.result().getRequestInfo());
    ufo.setBucket(checkpoint.getBucket());
    ufo.setKey(checkpoint.getKey());
    ufo.setUploadId(checkpoint.getUploadId());
    ufo.setETag(output.result().getETag());
    ufo.setLocation(output.result().getLocation());
    ufo.setVersionId(output.result().getVersionId());
    ufo.setHashCrc64Ecma(output.result().getHashCrc64ecma());
    ufo.setSsecAlgorithm(checkpoint.getSseAlgorithm());
    ufo.setSsecKeyMd5(checkpoint.getSseKeyMd5());
    ufo.setEncodingType(checkpoint.getEncodingType());
    ret.setSuccess(true);
    ret.setR(ufo);
    return ret;
}
Outcome<TosError, UploadFileV2Output> TosClientImpl::uploadFile(const UploadFileV2Input& input) {
    Outcome<TosError, UploadFileV2Output> res;
    TosError error;
    const auto& createMultipartInput = input.getCreateMultipartUploadInput();
    const auto& bucket = createMultipartInput.getBucket();
    const auto& key = createMultipartInput.getKey();
    auto check = validateInput(bucket, key, input.getPartSize(), input.getTaskNum());
    if (!check.isSuccess()) {
        error.setMessage(check.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::string checkpointFilePath;
    if (input.isEnableCheckpoint()) {
        checkpointFilePath = getCheckpointPath(bucket, key, input.getCheckpointFile(), input.getFilePath());
        if (checkpointFilePath.empty()) {
            error.setMessage("The file is not found in the specific path " + input.getCheckpointFile());
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    auto ufi = getUploadFileInfoV2(input.getFilePath());
    if (!ufi.isSuccess()) {
        error.setMessage(ufi.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto event = std::make_shared<UploadEvent>();
    initUploadEvent(input, checkpointFilePath, "", event);
    auto cp = getCheckpoint(input, ufi.result(), checkpointFilePath, event);
    if (!cp.isSuccess()) {
        error.setMessage(cp.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    return uploadPartConcurrent(input, cp.result(), checkpointFilePath, event);
}

void initDownloadEvent(const DownloadFileInput& input, const DownloadFileFileInfo& dfi,
                       const std::string& checkpointFilePath, std::shared_ptr<DownloadEvent> event) {
    event->type_ = 0;
    event->error_ = false;
    event->key_ = input.getHeadObjectV2Input().getKey();
    event->bucket_ = input.getHeadObjectV2Input().getBucket();
    event->versionId_ = input.getHeadObjectV2Input().getVersionId();
    event->filePath_ = dfi.getFilePath();
    auto filePath = dfi.getTempFilePath();
    event->tempFilePath_ = std::make_shared<std::string>(filePath);
    event->checkpointFile_ = std::make_shared<std::string>(checkpointFilePath);
    DownloadPartInfo partInfo{};
    partInfo.partNumber_ = 0;
    partInfo.rangeStart_ = 0;
    partInfo.rangeEnd_ = 0;
    event->downloadPartInfo_ = std::make_shared<DownloadPartInfo>(partInfo);
}

void downloadEventCreateTempFileSucceed(const std::shared_ptr<DownloadEvent>& event,
                                        const DownloadEventChange& eventChange) {
    event->type_ = 1;
    event->downloadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventCreateTempFileFailed(const std::shared_ptr<DownloadEvent>& event,
                                       const DownloadEventChange& eventChange) {
    event->type_ = 2;
    event->error_ = true;
    event->downloadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventDownloadPartSucceed(const std::shared_ptr<DownloadEvent>& event,
                                      const DownloadEventChange& eventChange, DownloadPartInfo partInfo) {
    event->type_ = 3;
    event->downloadPartInfo_ = std::make_shared<DownloadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventDownloadPartFailed(const std::shared_ptr<DownloadEvent>& event,
                                     const DownloadEventChange& eventChange, DownloadPartInfo partInfo) {
    event->type_ = 4;
    event->error_ = true;
    event->downloadPartInfo_ = std::make_shared<DownloadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventDownloadPartAborted(const std::shared_ptr<DownloadEvent>& event,
                                      const DownloadEventChange& eventChange, DownloadPartInfo partInfo) {
    event->type_ = 5;
    event->error_ = true;
    event->downloadPartInfo_ = std::make_shared<DownloadPartInfo>(partInfo);
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventRenameTempFileSucceed(const std::shared_ptr<DownloadEvent>& event,
                                        const DownloadEventChange& eventChange) {
    event->type_ = 6;
    event->downloadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}
void downloadEventRenameTempFileFailed(const std::shared_ptr<DownloadEvent>& event,
                                       const DownloadEventChange& eventChange) {
    event->type_ = 7;
    event->error_ = true;
    event->downloadPartInfo_ = nullptr;
    if (eventChange != nullptr) {
        eventChange(event);
    }
}

Outcome<TosError, std::vector<DownloadFilePartInfo>> getDownloadFilePartsInfo(int64_t objectSize, int64_t partSize) {
    Outcome<TosError, std::vector<DownloadFilePartInfo>> ret;
    auto partNum = objectSize / partSize;
    auto lastPartSize = objectSize % partSize;
    TosError error;
    if (lastPartSize != 0)
        partNum++;
    if (partNum > 10000) {
        error.setMessage("The split file parts number is larger than 10000, please increase your part size");
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    std::vector<DownloadFilePartInfo> partInfoList;
    for (int i = 0; i < partNum; i++) {
        DownloadFilePartInfo info;
        info.setPartNum(i + 1);
        info.setRangeStart(i * partSize);
        info.setRangeEnd((i + 1) * partSize - 1);
        partInfoList.emplace_back(info);
    }
    if (lastPartSize != 0) {
        partInfoList[partNum - 1].setRangeEnd((partNum - 1) * partSize + lastPartSize - 1);
    }
    ret.setR(partInfoList);
    ret.setSuccess(true);
    return ret;
}
Outcome<TosError, DownloadFileCheckpoint> TosClientImpl::initCheckpoint(const DownloadFileInput& input,
                                                                        const HeadObjectV2Output& headOutput,
                                                                        const DownloadFileFileInfo& info,
                                                                        const std::string& checkpointFilePath) {
    Outcome<TosError, DownloadFileCheckpoint> ret;
    TosError error;
    auto partsInfo = getDownloadFilePartsInfo(headOutput.getContentLength(), input.getPartSize());
    if (!partsInfo.isSuccess()) {
        error.setMessage(partsInfo.error().getMessage());
        ret.setSuccess(false);
        ret.setE(error);
        return ret;
    }
    auto headInput = input.getHeadObjectV2Input();
    DownloadFileCheckpoint checkpoint;
    checkpoint.setBucket(headInput.getBucket());
    checkpoint.setKey(headInput.getKey());
    checkpoint.setVersionId(headInput.getVersionId());
    checkpoint.setPartSize(input.getPartSize());
    checkpoint.setIfNoneMatch(headInput.getIfNoneMatch());
    checkpoint.setIfUnmodifiedSince(TimeUtils::transTimeToGmtTime(headInput.getIfUnmodifiedSince()));
    checkpoint.setSseAlgorithm(headInput.getSsecAlgorithm());
    checkpoint.setSseKeyMd5(headInput.getSsecKeyMd5());
    // 构建 objectInfo
    DownloadFileObjectInfo objectinfo;
    objectinfo.setETag(headOutput.getETags());
    objectinfo.setHashCrc64Ecma(headOutput.getHashCrc64Ecma());
    auto lastModified = headOutput.getLastModified();
    objectinfo.setLastModified(TimeUtils::transLastModifiedTimeToString(lastModified));
    objectinfo.setObjectSize(headOutput.getContentLength());
    checkpoint.setObjectInfo(objectinfo);
    // 设置FileInfo
    checkpoint.setFileInfo(info);
    // 设置PartsInfo
    checkpoint.setPartsInfo(partsInfo.result());

    ret.setSuccess(true);
    ret.setR(checkpoint);
    return ret;
}

std::string getDownloadCheckpointPath(const std::string& bucket, const std::string& key,
                                      const std::string& checkPointFile, const std::string filePath) {
    std::stringstream ret;
    std::string objKey(StringUtils::stringReplace(key, "/", "_"));
    if (checkPointFile.empty()) {
        ret << filePath << "." << bucket << "." << objKey << ".download";
        return ret.str();
    } else {
        struct stat cfs {};
        if (stat(checkPointFile.c_str(), &cfs) == 0) {
            if (cfs.st_mode & S_IFDIR) {
                ret << checkPointFile << "/" << bucket << "." << objKey << ".download";
                return ret.str();
            } else {
                ret << checkPointFile;
                return ret.str();
            }
        }
    }
    return "";
}
DownloadFileCheckpoint loadDownloadCheckpointFromFile(const std::string& checkpointFilePath) {
    DownloadFileCheckpoint dfc;
    dfc.load(checkpointFilePath);
    return dfc;
}
Outcome<TosError, DownloadFileCheckpoint> TosClientImpl::getCheckpoint(const DownloadFileInput& input,
                                                                       const std::string& checkpointFilePath,
                                                                       const DownloadFileFileInfo& fileInfo,
                                                                       const HeadObjectV2Output& headOutput,
                                                                       const std::shared_ptr<DownloadEvent>& event) {
    Outcome<TosError, DownloadFileCheckpoint> ret;
    DownloadFileCheckpoint checkpoint;
    if (input.isEnableCheckpoint()) {
        checkpoint = loadDownloadCheckpointFromFile(checkpointFilePath);
    } else {
        deleteCheckpointFile(checkpointFilePath);
    }
    const auto& headInput = input.getHeadObjectV2Input();
    bool valid = checkpoint.isValid(headInput, headOutput);
    // 清理临时文件
    if (!input.isEnableCheckpoint() || !valid) {
        const std::string& tempFilePath = fileInfo.getTempFilePath();
        std::fstream tempFile;
        tempFile.open(tempFilePath,
                      std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary);
        if (tempFile) {
            tempFile.close();
            downloadEventCreateTempFileSucceed(event, input.getDownloadEventListener().eventChange_);
        } else {
            downloadEventCreateTempFileFailed(event, input.getDownloadEventListener().eventChange_);
        }
    }
    if (!valid) {
        deleteCheckpointFile(checkpointFilePath);
        return this->initCheckpoint(input, headOutput, fileInfo, checkpointFilePath);
    }
    ret.setR(checkpoint);
    ret.setSuccess(true);
    return ret;
}
Outcome<TosError, DownloadFileFileInfo> getDownloadFileFileInfo(const DownloadFileInput& input) {
    Outcome<TosError, DownloadFileFileInfo> ret;
    TosError error;
    std::stringstream ret_filePath;
    std::stringstream ret_tempFilePath;
    std::string key = input.getHeadObjectV2Input().getKey();

    DownloadFileFileInfo fileinfo;

    struct stat dfs {};
    std::string filePath = input.getFilePath();
    // 设置文件路径
    if (stat(filePath.c_str(), &dfs) == 0) {
        if (dfs.st_mode & S_IFDIR) {
            ret_filePath << filePath << "/" << key;
            ret_tempFilePath << ret_filePath.str() << ".temp";
            fileinfo.setFilePath(ret_filePath.str());
            fileinfo.setTempFilePath(ret_tempFilePath.str());
            ret.setR(fileinfo);
            ret.setSuccess(true);
        } else {
            ret_filePath << filePath;
            ret_tempFilePath << ret_filePath.str() << ".temp";
            fileinfo.setFilePath(ret_filePath.str());
            fileinfo.setTempFilePath(ret_tempFilePath.str());
            ret.setR(fileinfo);
            ret.setSuccess(true);
        }
    } else {
        error.setMessage("invalid file path, the file does not exist");
        error.setIsClientError(true);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }

    return ret;
}

Outcome<TosError, DownloadFileOutput> TosClientImpl::downloadPartConcurrent(
        const DownloadFileInput& input, const HeadObjectV2Output& headOutput, DownloadFileCheckpoint checkpoint,
        const std::string& checkpointPath, const DownloadFileFileInfo& dfi, std::shared_ptr<DownloadEvent> event) {
    Outcome<TosError, DownloadFileOutput> ret;
    TosError error;
    std::vector<DownloadFilePartInfo> toDownload = checkpoint.getPartsInfo();
    std::vector<std::thread> threadPool;
    std::mutex lock_;
    int64_t partSize_ = input.getPartSize();
    auto eventChange = input.getDownloadEventListener().eventChange_;
    auto cancel = input.getCancelHook();
    std::string tempFilePath = dfi.getTempFilePath();
    std::atomic<bool> isAbort(false);
    std::atomic<bool> isSuccess(true);
    auto logger = LogUtils::GetLogger();
    // 进度条相关参数
    UploadDownloadFileProcessStat processStat;
    auto pProcessStat = &processStat;
    auto process = input.getDataTransferListener();
    if (process.dataTransferStatusChange_ != nullptr) {
        std::mutex processLock;
        pProcessStat->dataTransferListener_.dataTransferStatusChange_ = process.dataTransferStatusChange_;
        pProcessStat->dataTransferListener_.userData_ = process.userData_;
        pProcessStat->userData = (void*)pProcessStat;
        pProcessStat->totalBytes_ = headOutput.getContentLength();
    }
    for (int i = 0; i < input.getTaskNum(); i++) {
        auto res = std::thread([&]() {
            while (true) {
                // 开始时检查是否需要中断任务
                if (cancel != nullptr) {
                    if (cancel->isCancel()) {
                        break;
                    }
                }
                // 发生严重错误，中断任务
                if (isAbort) {
                    break;
                }
                // 任务队列中取part
                DownloadFilePartInfo part;
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (toDownload.empty())
                        break;
                    part = toDownload.front();
                    toDownload.erase(toDownload.begin());
                }
                if (part.isCompleted()) {
                    continue;
                }
                // 发送 GetObject 请求数据
                GetObjectV2Input input_obj_get;
                input_obj_get.setBucket(input.getHeadObjectV2Input().getBucket());
                input_obj_get.setKey(input.getHeadObjectV2Input().getKey());
                input_obj_get.setRangeStart(part.getRangeStart());
                input_obj_get.setRangeEnd(part.getRangeEnd());
                // 设置限流
                input_obj_get.setRateLimiter(input.getRateLimiter());
                // 设置回调
                if (process.dataTransferStatusChange_ != nullptr) {
                    input_obj_get.setDataTransferListener({UploadDownloadFileProcessCallback, (void*)pProcessStat});
                }
                auto partHashCrc64ecma = std::make_shared<uint64_t>(0);
                auto res = this->getObject(input_obj_get, partHashCrc64ecma);

                // 下载后检查是否需要中断任务
                if (cancel != nullptr) {
                    if (cancel->isCancel()) {
                        break;
                    }
                }
                if (res.isSuccess()) {
                    // 成功则写入数据到文件
                    {
                        std::lock_guard<std::mutex> lck(lock_);
                        std::fstream tempFile;
                        tempFile.open(tempFilePath, std::ios_base::out | std::ios_base::in | std::ios_base::binary);
                        if (tempFile) {
                            auto currentPos = partSize_ * (part.getPartNum() - 1);
                            tempFile.seekp(currentPos, tempFile.beg);
                            tempFile << res.result().getContent()->rdbuf();
                            // 读流失败
                            if (tempFile.bad() || tempFile.fail()) {
                                DownloadPartInfo partInfo{part.getPartNum(), part.getRangeStart(), part.getRangeEnd()};
                                downloadEventDownloadPartAborted(event, eventChange, partInfo);
                                if (logger != nullptr) {
                                    logger->info("failed to write stream to file");
                                }
                                isAbort = true;
                            } else {
                                // 下载段成功
                                DownloadPartInfo partInfo{part.getPartNum(), part.getRangeStart(), part.getRangeEnd()};
                                downloadEventDownloadPartSucceed(event, eventChange, partInfo);
                            }
                            tempFile.close();
                        } else {
                            // 打开文件失败
                            DownloadPartInfo partInfo{part.getPartNum(), part.getRangeStart(), part.getRangeEnd()};
                            downloadEventDownloadPartFailed(event, eventChange, partInfo);
                            isSuccess = false;
                        }
                    }
                    // 更新 checkpoint 信息, 把更新后的 part 放到 checkpoint 的 vector 中，赋值并发安全
                    part.setIsCompleted(true);
                    part.setHashCrc64Ecma(*partHashCrc64ecma);
                    checkpoint.setDownloadFilePartInfoByIdx(part, part.getPartNum() - 1);
                    if (input.isEnableCheckpoint()) {
                        {
                            // 更新 checkpoint 文件
                            std::lock_guard<std::mutex> lck(lock_);
                            checkpoint.dump(checkpointPath);
                        }
                    }
                } else {
                    // 下载段失败
                    auto statusCode = res.error().getStatusCode();
                    if (statusCode == 403 || statusCode == 404 || statusCode == 405) {
                        std::lock_guard<std::mutex> lck(lock_);
                        DownloadPartInfo partInfo{part.getPartNum(), part.getRangeStart(), part.getRangeEnd()};
                        downloadEventDownloadPartAborted(event, eventChange, partInfo);
                        // 出现 403、404、405 错误需要中断整个断点续传任务
                        isAbort = true;
                        break;
                    }
                    std::lock_guard<std::mutex> lck(lock_);
                    DownloadPartInfo partInfo{part.getPartNum(), part.getRangeStart(), part.getRangeEnd()};
                    downloadEventDownloadPartFailed(event, eventChange, partInfo);
                    isSuccess = false;
                }
            }
        });
        threadPool.emplace_back(std::move(res));
    }
    for (auto& worker : threadPool) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    if (isAbort) {
        if (input.isEnableCheckpoint()) {
            deleteCheckpointFile(checkpointPath);
        }
        remove(dfi.getTempFilePath().c_str());
        error.setMessage("the task is canceled");
        error.setIsClientError(true);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    // 最后后校验是否需要中断任务
    if (cancel != nullptr) {
        // 发生错误
        if (cancel->isCancel()) {
            // 发生 Abort 错误，需要删除上下文信息和临时文件
            bool cancelIsAbort = cancel->isAbortFunc();
            if (cancelIsAbort) {
                // 删除上下文和临时文件
                if (input.isEnableCheckpoint()) {
                    deleteCheckpointFile(checkpointPath);
                }
                remove(dfi.getTempFilePath().c_str());
            }
            error.setMessage("the task is canceled");
            error.setIsClientError(true);
            ret.setE(error);
            ret.setSuccess(false);
            return ret;
        }
    }
    // 有的段下载发生错误，直接返回
    if (!isSuccess) {
        error.setMessage("some parts are downloaded incorrectly, you can try again");
        error.setIsClientError(true);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }

    // 校验临时文件的Crc64与headOutput中的crc64是否一致
    auto headCrc64 = headOutput.getHashCrc64Ecma();
    // 由于插入时判定了位置，因此不需要重新排序
    auto partSorted = checkpoint.getPartsInfo();
    // 然后计算crc64
    uint64_t localCRC64 = partSorted[0].getHashCrc64Ecma();
    for (int i = 1; i < partSorted.size(); i++) {
        uint64_t size = partSorted[i].getRangeEnd() - partSorted[i].getRangeStart() + 1;
        localCRC64 = CRC64::CombineCRC(localCRC64, partSorted[i].getHashCrc64Ecma(), size);
    }
    // 校验不通过，需要删除临时文件
    // headCrc64 对于旧对象可能没有这个参数
    if (localCRC64 != headCrc64 && headCrc64 != 0) {
        if (input.isEnableCheckpoint()) {
            deleteCheckpointFile(checkpointPath);
        }
        remove(dfi.getTempFilePath().c_str());
        error.setIsClientError(true);
        error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    // 校验通过，删除 checkpoint 文件，重命名临时文件
    if (input.isEnableCheckpoint()) {
        deleteCheckpointFile(checkpointPath);
    }
    auto renameRes = rename(dfi.getTempFilePath().c_str(), dfi.getFilePath().c_str());
    if (renameRes == 0) {
        downloadEventRenameTempFileSucceed(event, eventChange);
    } else {
        downloadEventRenameTempFileSucceed(event, eventChange);
        error.setMessage("rename tempfile failed");
        error.setIsClientError(true);
        ret.setE(error);
        ret.setSuccess(false);
        return ret;
    }
    DownloadFileOutput downloadFileOutput;
    downloadFileOutput.setHeadObjectV2Output(headOutput);
    ret.setR(downloadFileOutput);
    ret.setSuccess(true);
    return ret;
}

Outcome<TosError, DownloadFileOutput> TosClientImpl::downloadFile(const DownloadFileInput& input) {
    Outcome<TosError, DownloadFileOutput> res;
    TosError error;
    const auto& headInput = input.getHeadObjectV2Input();
    auto check = validateInput(headInput.getBucket(), headInput.getKey(), input.getPartSize(), input.getTaskNum());
    if (!check.isSuccess()) {
        error.setMessage(check.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto checkObjectExists = this->headObject(headInput);
    if (!checkObjectExists.isSuccess()) {
        error.setMessage(checkObjectExists.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto dfi = getDownloadFileFileInfo(input);
    if (!dfi.isSuccess()) {
        error.setMessage(dfi.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    std::string checkpointFilePath;
    if (input.isEnableCheckpoint()) {
        checkpointFilePath = getDownloadCheckpointPath(input.getHeadObjectV2Input().getBucket(),
                                                       input.getHeadObjectV2Input().getKey(), input.getCheckpointFile(),
                                                       dfi.result().getFilePath());
        if (checkpointFilePath.empty()) {
            error.setIsClientError(true);
            error.setMessage("The file is not found in the specific path " + input.getCheckpointFile());
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    auto event = std::make_shared<DownloadEvent>();
    initDownloadEvent(input, dfi.result(), checkpointFilePath, event);
    // 检查 checkpoint 文件是否存在 + 检查 checkpoint 文件有效性 + 无效则创建临时文件
    auto cp = getCheckpoint(input, checkpointFilePath, dfi.result(), checkObjectExists.result(), event);
    if (!cp.isSuccess()) {
        error.setMessage(cp.error().getMessage());
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    return downloadPartConcurrent(input, checkObjectExists.result(), cp.result(), checkpointFilePath, dfi.result(),
                                  event);
}

Outcome<TosError, AppendObjectOutput> TosClientImpl::appendObject(const std::string& bucket,
                                                                  const std::string& objectKey,
                                                                  const std::shared_ptr<std::iostream>& content,
                                                                  int64_t offset) {
    Outcome<TosError, AppendObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    setContentType(rb, objectKey);
    rb.withQuery("append", "");
    rb.withQuery("offset", std::to_string(offset));
    auto req = rb.Build(http::MethodPost, content);
    this->appendObject(req, res);
    return res;
}
Outcome<TosError, AppendObjectOutput> TosClientImpl::appendObject(const std::string& bucket,
                                                                  const std::string& objectKey,
                                                                  const std::shared_ptr<std::iostream>& content,
                                                                  int64_t offset, const RequestOptionBuilder& builder) {
    Outcome<TosError, AppendObjectOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    setContentType(rb, objectKey);
    rb.withQuery("append", "");
    rb.withQuery("offset", std::to_string(offset));
    auto req = rb.Build(http::MethodPost, content);
    this->appendObject(req, res);
    return res;
}
static void appendObjectSetOptionHeader(RequestBuilder& rb, const AppendObjectV2Input& input) {
    rb.withHeader(http::HEADER_CONTENT_TYPE, input.getContentType());
    rb.withHeader(HEADER_ACL, ACLTypetoString[input.getAcl()]);
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
    setMetaHeader(input.getMeta(), rb);
    rb.withHeader(HEADER_WEBSITE_REDIRECT_LOCATION, input.getWebsiteRedirectLocation());
    rb.withHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[input.getStorageClass()]);
}
Outcome<TosError, AppendObjectV2Output> TosClientImpl::appendObject(const AppendObjectV2Input& input) {
    Outcome<TosError, AppendObjectV2Output> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    // offset有默认值因此不校验
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("append", "");
    rb.withQuery("offset", std::to_string(input.getOffset()));

    if (config_.isAutoRecognizeContentType()) {
        setContentType(rb, input.getKey());
    }

    appendObjectSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodPost, input.getContent());
    // 设置进度条回调
    auto handler = input.getDataTransferListener();
    SetProcessHandlerToReq(req, handler);
    // 客户端限速
    auto limiter = input.getRateLimiter();
    SetRateLimiterToReq(req, limiter);
    // 设置crc64校验
    SetCrc64ParmToReq(req);
    // 传递 preHashCrc64
    req->setPreHashCrc64Ecma(input.getPreHashCrc64Ecma());
    // 发送请求
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    // crc64 校验
    if (config_.isEnableCrc()) {
        auto hashcrc64 = std::stoull(tosRes.result()->findHeader(HEADER_CRC64));
        if (tosRes.result()->getHashCrc64Result() != hashcrc64) {
            TosError error;
            error.setIsClientError(true);
            error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    AppendObjectV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    auto nextOffset = tosRes.result()->findHeader(HEADER_NEXT_APPEND_OFFSET);
    output.setNextAppendOffset(std::stoll(nextOffset));
    output.setHashCrc64ecma(std::stoull(tosRes.result()->findHeader(HEADER_CRC64)));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, SetObjectMetaOutput> TosClientImpl::setObjectMeta(const std::string& bucket,
                                                                    const std::string& objectKey,
                                                                    const RequestOptionBuilder& builder) {
    Outcome<TosError, SetObjectMetaOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->setObjectMeta(rb, res);
    return res;
}
static void setObjectMetaSetOptionHeader(RequestBuilder& rb, const SetObjectMetaInput& input) {
    rb.withHeader(HEADER_VERSIONID, input.getVersionId());
    rb.withHeader(http::HEADER_CONTENT_TYPE, input.getContentType());
    rb.withHeader(http::HEADER_CACHE_CONTROL, input.getCacheControl());
    rb.withHeader(http::HEADER_EXPIRES, TimeUtils::transTimeToGmtTime(input.getExpires()));

    // rb.withHeader(http::HEADER_CONTENT_DISPOSITION, input.getContentDisposition());
    auto UrlEncode_disposition = CryptoUtils::UrlEncodeChinese(input.getContentDisposition());
    rb.withHeader(http::HEADER_CONTENT_DISPOSITION, UrlEncode_disposition);

    rb.withHeader(http::HEADER_CONTENT_ENCODING, input.getContentEncoding());
    rb.withHeader(http::HEADER_CONTENT_LANGUAGE, input.getContentLanguage());
    setMetaHeader(input.getMeta(), rb);
}

Outcome<TosError, SetObjectMetaOutput> TosClientImpl::setObjectMeta(const SetObjectMetaInput& input) {
    Outcome<TosError, SetObjectMetaOutput> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("metadata", "");
    setObjectMetaSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodPost, nullptr);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    SetObjectMetaOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, ListObjectsOutput> TosClientImpl::listObjects(const std::string& bucket,
                                                                const ListObjectsInput& input) {
    Outcome<TosError, ListObjectsOutput> res;

    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(bucket, "");
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("marker", input.getMarker());
    if (input.getMaxKeys() != 0) {
        rb.withQueryCheckEmpty("max-keys", std::to_string(input.getMaxKeys()));
    }
    if (input.isReverse() != false) {
        rb.withQueryCheckEmpty("reverse", std::to_string(input.isReverse()));
    }
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListObjectsOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, ListObjectsV2Output> TosClientImpl::listObjects(const ListObjectsV2Input& input) {
    Outcome<TosError, ListObjectsV2Output> res;

    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(input.getBucket(), "");

    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());
    if (input.getMaxKeys() != 0) {
        rb.withQueryCheckEmpty("max-keys", std::to_string(input.getMaxKeys()));
    }
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("marker", input.getMarker());
    if (input.isReverse() != false) {
        rb.withQueryCheckEmpty("reverse", std::to_string(input.isReverse()));
    }
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListObjectsV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, ListObjectVersionsOutput> TosClientImpl::listObjectVersions(const std::string& bucket,
                                                                              const ListObjectVersionsInput& input) {
    Outcome<TosError, ListObjectVersionsOutput> res;
    std::string check = isValidBucketName(bucket);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(bucket, "");
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("key-marker", input.getKeyMarker());
    if (input.getMaxKeys() != 0) {
        rb.withQueryCheckEmpty("max-keys", std::to_string(input.getMaxKeys()));
    }
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());
    rb.withQuery("versions", "");
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListObjectVersionsOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, ListObjectVersionsV2Output> TosClientImpl::listObjectVersions(
        const ListObjectVersionsV2Input& input) {
    Outcome<TosError, ListObjectVersionsV2Output> res;
    std::string check = isValidBucketName(input.getBucket());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(input.getBucket(), "");
    rb.withQuery("versions", "");
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("key-marker", input.getKeyMarker());
    if (input.getMaxKeys() != 0) {
        rb.withQueryCheckEmpty("max-keys", std::to_string(input.getMaxKeys()));
    }
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());
    rb.withQueryCheckEmpty("version-id-marker", input.getVersionIdMarker());
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListObjectVersionsV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObject(const std::string& bucket,
                                                              const std::string& srcObjectKey,
                                                              const std::string& dstObjectKey) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
    std::string check = isValidNames(bucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(bucket, dstObjectKey, bucket, srcObjectKey, res);
    return res;
}
Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObject(const std::string& bucket,
                                                              const std::string& srcObjectKey,
                                                              const std::string& dstObjectKey,
                                                              const RequestOptionBuilder& builder) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
    std::string check = isValidNames(bucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(bucket, dstObjectKey, bucket, srcObjectKey, builder, res);
    return res;
}

static std::string isValidCopySourceSSEC(const std::string& CopySourceSSECAlgorithm,
                                         const std::string& CopySourceSSECKey,
                                         const std::string& CopySourceSSECKeyMd5) {
    if (CopySourceSSECAlgorithm.empty() && CopySourceSSECKey.empty() && CopySourceSSECKeyMd5.empty()) {
        return "";
    }
    if (CopySourceSSECAlgorithm.empty() || CopySourceSSECKey.empty() || CopySourceSSECKeyMd5.empty()) {
        return "invalid encryption-decryption algorithm";
    }
}

static void setCopySourceSSECHeader(const std::string& CopySourceSSECAlgorithm, const std::string& CopySourceSSECKey,
                                    const std::string& CopySourceSSECKeyMd5, RequestBuilder& rb) {
    if (CopySourceSSECAlgorithm.empty() && CopySourceSSECKey.empty() && CopySourceSSECKeyMd5.empty()) {
        return;
    }
    rb.withHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_ALGORITHM, CopySourceSSECAlgorithm);
    rb.withHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY, CopySourceSSECKey);
    rb.withHeader(HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY_MD5, CopySourceSSECKeyMd5);
}
static void copyObjectSetOptionHeader(RequestBuilder& rb, const CopyObjectV2Input& input) {
    rb.withHeader(HEADER_ACL, ACLTypetoString[input.getAcl()]);
    rb.withHeader(HEADER_COPY_SOURCE_IF_MATCH, input.getCopySourceIfMatch());
    rb.withHeader(HEADER_COPY_SOURCE_IF_MODIFIED_SINCE,
                  TimeUtils::transTimeToGmtTime(input.getCopySourceIfModifiedSince()));
    rb.withHeader(HEADER_COPY_SOURCE_IF_NONE_MATCH, input.getCopySourceIfNoneMatch());
    rb.withHeader(HEADER_COPY_SOURCE_IF_UNMODIFIED_SINCE,
                  TimeUtils::transTimeToGmtTime(input.getCopySourceIfUnmodifiedSince()));
    setCopySourceSSECHeader(input.getCopySourceSsecAlgorithm(), input.getCopySourceSsecKey(),
                            input.getCopySourceSsecKeyMd5(), rb);
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
    rb.withHeader(HEADER_METADATA_DIRECTIVE, MetadataDirectiveTypetoString[input.getMetadataDirective()]);
    setMetaHeader(input.getMeta(), rb);
    rb.withHeader(HEADER_WEBSITE_REDIRECT_LOCATION, input.getWebsiteRedirectLocation());
    rb.withHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[input.getStorageClass()]);
    // rb.withHeader(HEADER_AZ_REDUNDANCY, AzRedundancytoString[input.getAzRedundancy()]);
}
Outcome<TosError, CopyObjectV2Output> TosClientImpl::copyObject(const CopyObjectV2Input& input) {
    Outcome<TosError, CopyObjectV2Output> res;
    std::vector<std::string> keys = {input.getKey(), input.getSrcKey()};
    std::vector<std::string> bkts = {input.getBucket(), input.getSrcBucket()};
    std::string check = isValidNames(bkts, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidCopySourceSSEC(input.getCopySourceSsecAlgorithm(), input.getCopySourceSsecKey(),
                                  input.getCopySourceSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());

    copyObjectSetOptionHeader(rb, input);
    auto req = rb.BuildWithCopySource(http::MethodPut, input.getSrcBucket(), input.getSrcKey());
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CopyObjectV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setCopySourceVersionId(tosRes.result()->findHeader(HEADER_COPY_SOURCE_VERSION_ID));
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObjectTo(const std::string& bucket, const std::string& dstBucket,
                                                                const std::string& dstObjectKey,
                                                                const std::string& srcObjectKey) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
    std::string check = isValidNames(dstBucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(dstBucket, dstObjectKey, bucket, srcObjectKey, res);
    return res;
}
Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObjectTo(const std::string& bucket, const std::string& dstBucket,
                                                                const std::string& dstObjectKey,
                                                                const std::string& srcObjectKey,
                                                                const RequestOptionBuilder& builder) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {dstObjectKey, srcObjectKey};
    std::string check = isValidNames(dstBucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(dstBucket, dstObjectKey, bucket, srcObjectKey, builder, res);
    return res;
}
Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObjectFrom(const std::string& bucket,
                                                                  const std::string& srcBucket,
                                                                  const std::string& srcObjectKey,
                                                                  const std::string& dstObjectKey) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {srcObjectKey, dstObjectKey};
    std::string check = isValidNames(srcBucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(bucket, dstObjectKey, srcBucket, srcObjectKey, res);
    return res;
}
Outcome<TosError, CopyObjectOutput> TosClientImpl::copyObjectFrom(const std::string& bucket,
                                                                  const std::string& srcBucket,
                                                                  const std::string& srcObjectKey,
                                                                  const std::string& dstObjectKey,
                                                                  const RequestOptionBuilder& builder) {
    Outcome<TosError, CopyObjectOutput> res;
    std::vector<std::string> keys = {srcObjectKey, dstObjectKey};
    std::string check = isValidNames(srcBucket, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    this->copyObject(bucket, dstObjectKey, srcBucket, srcObjectKey, builder, res);
    return res;
}
static std::string copyRange(long startOffset, long partSize) {
    std::string cr;
    if (startOffset == 0) {
        if (partSize != 0) {
            cr += "bytes=";
            cr += std::to_string(startOffset);
            cr += "-";
            cr += std::to_string(startOffset + partSize - 1);
        } else {
            cr += "bytes=";
            cr += std::to_string(startOffset);
            cr += "-";
        }
    } else if (partSize != 0) {
        cr += "bytes=0-";
        cr += std::to_string(partSize - 1);
    }
    return cr;
}
Outcome<TosError, UploadPartCopyOutput> TosClientImpl::uploadPartCopy(const std::string& bucket,
                                                                      const UploadPartCopyInput& input) {
    Outcome<TosError, UploadPartCopyOutput> res;
    std::vector<std::string> keys = {input.getSourceKey(), input.getDestinationKey()};
    std::vector<std::string> bkts = {input.getSourceBucket(), bucket};
    std::string check = isValidNames(bkts, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getPartNumber() == 0) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getDestinationKey());
    this->uploadPartCopy(rb, input, res);
    return res;
}
Outcome<TosError, UploadPartCopyOutput> TosClientImpl::uploadPartCopy(const std::string& bucket,
                                                                      const UploadPartCopyInput& input,
                                                                      const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadPartCopyOutput> res;
    std::vector<std::string> keys = {input.getSourceKey(), input.getDestinationKey()};
    std::vector<std::string> bkts = {input.getSourceBucket(), bucket};
    std::string check = isValidNames(bkts, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getPartNumber() == 0) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getDestinationKey(), builder);
    this->uploadPartCopy(rb, input, res);
    return res;
}
static void uploadPartCopySetOptionHeader(RequestBuilder& rb, const UploadPartCopyV2Input& input) {
    rb.withHeader(HEADER_COPY_SOURCE_IF_MATCH, input.getCopySourceIfMatch());
    rb.withHeader(HEADER_COPY_SOURCE_IF_MODIFIED_SINCE,
                  TimeUtils::transTimeToGmtTime(input.getCopySourceIfModifiedSince()));
    rb.withHeader(HEADER_COPY_SOURCE_IF_NONE_MATCH, input.getCopySourceIfNoneMatch());
    rb.withHeader(HEADER_COPY_SOURCE_IF_UNMODIFIED_SINCE,
                  TimeUtils::transTimeToGmtTime(input.getCopySourceIfUnmodifiedSince()));

    if (input.getCopySourceRangeStart() != 0 || input.getCopySourceRangeEnd() != 0) {
        HttpRange range_;
        range_.setStart(input.getCopySourceRangeStart());
        range_.setEnd(input.getCopySourceRangeEnd());
        rb.withHeader(HEADER_COPY_SOURCE_RANGE, range_.toString());
    }

    setCopySourceSSECHeader(input.getCopySourceSsecAlgorithm(), input.getCopySourceSsecKey(),
                            input.getCopySourceSsecKeyMd5(), rb);
}
Outcome<TosError, UploadPartCopyV2Output> TosClientImpl::uploadPartCopy(const UploadPartCopyV2Input& input) {
    Outcome<TosError, UploadPartCopyV2Output> res;
    std::vector<std::string> keys = {input.getKey(), input.getSrcKey()};
    std::vector<std::string> bkts = {input.getBucket(), input.getSrcBucket()};
    std::string check = isValidNames(bkts, keys);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidCopySourceSSEC(input.getCopySourceSsecAlgorithm(), input.getCopySourceSsecKey(),
                                  input.getCopySourceSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidRange(input.getCopySourceRangeStart(), input.getCopySourceRangeEnd());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    if (input.getUploadId().empty()) {
        TosError error;
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getPartNumber() == 0) {
        TosError error;
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("partNumber", std::to_string(input.getPartNumber()));
    rb.withQuery("uploadId", input.getUploadId());
    uploadPartCopySetOptionHeader(rb, input);
    auto req = rb.BuildWithCopySource(http::MethodPut, input.getSrcBucket(), input.getSrcKey());
    SetCrc64ParmToReq(req);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    UploadPartCopyV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setCopySourceVersionId(tosRes.result()->findHeader(HEADER_COPY_SOURCE_VERSION_ID));
    // PartNumber从input传入
    output.setPartNumber(input.getPartNumber());

    res.setSuccess(true);
    res.setR(output);
    return res;
}

void setAclGrant(const PutObjectAclInput& input, RequestBuilder& rb) {
    const auto& grant = input.getAclGrant();
    if (!grant.getAcl().empty())
        rb.withHeader(HEADER_ACL, grant.getAcl());
    if (!grant.getGrantFullControl().empty())
        rb.withHeader(HEADER_GRANT_FULL_CONTROL, grant.getGrantFullControl());
    if (!grant.getGrantRead().empty())
        rb.withHeader(HEADER_GRANT_READ, grant.getGrantRead());
    if (!grant.getGrantReadAcp().empty())
        rb.withHeader(HEADER_GRANT_READ_ACP, grant.getGrantReadAcp());
    if (!grant.getGrantWrite().empty())
        rb.withHeader(HEADER_GRANT_WRITE, grant.getGrantWrite());
    if (!grant.getGrantWriteAcp().empty())
        rb.withHeader(HEADER_GRANT_WRITE_ACP, grant.getGrantWriteAcp());
}
Outcome<TosError, PutObjectAclOutput> TosClientImpl::putObjectAcl(const std::string& bucket,
                                                                  const PutObjectAclInput& input) {
    Outcome<TosError, PutObjectAclOutput> res;
    std::string check = isValidNames(bucket, {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rules = input.getAclRules();
    std::shared_ptr<std::stringstream> ss = nullptr;
    std::string jsonRules(rules.toJsonString());
    if (jsonRules != "null") {
        ss = std::make_shared<std::stringstream>(jsonRules);
        //        std::cout << ss->str() << std::endl;
    }
    auto rb = newBuilder(bucket, input.getKey());
    rb.withQuery("acl", "");
    setAclGrant(input, rb);
    auto req = rb.Build(http::MethodPut, ss);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    PutObjectAclOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, PutObjectAclV2Output> TosClientImpl::putObjectAcl(const PutObjectAclV2Input& input) {
    Outcome<TosError, PutObjectAclV2Output> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    std::shared_ptr<std::stringstream> ss = nullptr;
    std::string jsonRules(input.toJsonString());
    if (jsonRules != "null") {
        ss = std::make_shared<std::stringstream>(jsonRules);
        //        std::cout << ss->str() << std::endl;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("acl", "");
    rb.withHeader(HEADER_ACL, ACLTypetoString[input.getAcl()]);
    rb.withHeader(HEADER_VERSIONID, input.getVersionId());
    auto req = rb.Build(http::MethodPut, ss);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    PutObjectAclV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, GetObjectAclOutput> TosClientImpl::getObjectAcl(const std::string& bucket,
                                                                  const std::string& objectKey) {
    Outcome<TosError, GetObjectAclOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    this->getObjectAcl(rb, res);
    return res;
}
Outcome<TosError, GetObjectAclOutput> TosClientImpl::getObjectAcl(const std::string& bucket,
                                                                  const std::string& objectKey,
                                                                  const RequestOptionBuilder& builder) {
    Outcome<TosError, GetObjectAclOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->getObjectAcl(rb, res);
    return res;
}
Outcome<TosError, GetObjectAclV2Output> TosClientImpl::getObjectAcl(const GetObjectAclV2Input& input) {
    Outcome<TosError, GetObjectAclV2Output> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("acl", "");
    rb.withQueryCheckEmpty("versionId", input.getVersionId());
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    GetObjectAclV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, CreateMultipartUploadOutput> TosClientImpl::createMultipartUpload(const std::string& bucket,
                                                                                    const std::string& objectKey) {
    Outcome<TosError, CreateMultipartUploadOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    setContentType(rb, objectKey);
    this->createMultipartUpload(rb, res);
    return res;
}
Outcome<TosError, CreateMultipartUploadOutput> TosClientImpl::createMultipartUpload(
        const std::string& bucket, const std::string& objectKey, const RequestOptionBuilder& builder) {
    Outcome<TosError, CreateMultipartUploadOutput> res;
    std::string check = isValidNames(bucket, {objectKey});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    setContentType(rb, objectKey);
    this->createMultipartUpload(rb, res);
    return res;
}
static void createMultipartUploadSetOptionHeader(RequestBuilder& rb, const CreateMultipartUploadInput& input) {
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());

    rb.withHeader(http::HEADER_CONTENT_TYPE, input.getContentType());
    rb.withHeader(http::HEADER_CACHE_CONTROL, input.getCacheControl());
    rb.withHeader(http::HEADER_EXPIRES, TimeUtils::transTimeToGmtTime(input.getExpires()));

    auto UrlEncode_disposition = CryptoUtils::UrlEncodeChinese(input.getContentDisposition());
    rb.withHeader(http::HEADER_CONTENT_DISPOSITION, UrlEncode_disposition);

    rb.withHeader(http::HEADER_CONTENT_ENCODING, input.getContentEncoding());
    rb.withHeader(http::HEADER_CONTENT_LANGUAGE, input.getContentLanguage());
    rb.withHeader(HEADER_ACL, ACLTypetoString[input.getAcl()]);
    rb.withHeader(HEADER_GRANT_FULL_CONTROL, input.getGrantFullControl());
    rb.withHeader(HEADER_GRANT_READ, input.getGrantRead());
    rb.withHeader(HEADER_GRANT_READ_ACP, input.getGrantReadAcp());
    rb.withHeader(HEADER_GRANT_WRITE_ACP, input.getGrantWriteAcp());
    setMetaHeader(input.getMeta(), rb);

    setSSECHeader(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5(), rb);
    rb.withHeader(HEADER_WEBSITE_REDIRECT_LOCATION, input.getWebsiteRedirectLocation());
    rb.withHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[input.getStorageClass()]);
    rb.withHeader(HEADER_SSE, input.getServerSideEncryption());
}

Outcome<TosError, CreateMultipartUploadOutput> TosClientImpl::createMultipartUpload(
        const CreateMultipartUploadInput& input) {
    Outcome<TosError, CreateMultipartUploadOutput> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidSSEC(input.getSsecAlgorithm(), input.getSsecKey(), input.getSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("uploads", "");

    if (config_.isAutoRecognizeContentType()) {
        setContentType(rb, input.getKey());
    }

    createMultipartUploadSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodPost, nullptr);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    inner::MultipartUpload upload;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    upload.fromJsonString(ss.str());
    CreateMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setBucket(upload.getBucket());
    output.setKey(upload.getKey());
    output.setUploadId(upload.getUploadId());
    output.setEncodingType(upload.getEncodingType());
    output.setSsecAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSsecMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, UploadPartOutput> TosClientImpl::uploadPart(const std::string& bucket, const UploadPartInput& input) {
    Outcome<TosError, UploadPartOutput> res;
    std::string check = isValidNames(bucket, {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getPartNumber() == 0) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getKey());
    this->uploadPart(rb, input, res);
    return res;
}
Outcome<TosError, UploadPartOutput> TosClientImpl::uploadPart(const std::string& bucket, const UploadPartInput& input,
                                                              const RequestOptionBuilder& builder) {
    Outcome<TosError, UploadPartOutput> res;
    std::string check = isValidNames(bucket, {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getPartNumber() == 0) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getKey(), builder);
    this->uploadPart(rb, input, res);
    return res;
}
Outcome<TosError, UploadPartV2Output> TosClientImpl::uploadPart(const UploadPartV2Input& input,
                                                                std::shared_ptr<uint64_t> hashCrc64ecma) {
    Outcome<TosError, UploadPartV2Output> res;
    const UploadPartBasicInput& uploadPartBasicInput_ = input.getUploadPartBasicInput();
    std::string check = isValidNames(uploadPartBasicInput_.getBucket(), {uploadPartBasicInput_.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    check = isValidSSEC(uploadPartBasicInput_.getSsecAlgorithm(), uploadPartBasicInput_.getSsecKey(),
                        uploadPartBasicInput_.getSsecKeyMd5());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (uploadPartBasicInput_.getPartNumber() == 0) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter PartNumber is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (uploadPartBasicInput_.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    auto rb = newBuilder(uploadPartBasicInput_.getBucket(), uploadPartBasicInput_.getKey());
    rb.setContentLength(input.getContentLength());
    rb.withQuery("uploadId", uploadPartBasicInput_.getUploadId());
    rb.withQuery("partNumber", std::to_string(uploadPartBasicInput_.getPartNumber()));

    const auto& basic_input = input.getUploadPartBasicInput();
    rb.withHeader(http::HEADER_CONTENT_MD5, basic_input.getContentMd5());
    setSSECHeader(basic_input.getSsecAlgorithm(), basic_input.getSsecKey(), basic_input.getSsecKeyMd5(), rb);

    auto req = rb.Build(http::MethodPut, input.getContent());
    // 设置funcName
    req->setFuncName(__func__);
    // 进度条回调设置
    auto handler = uploadPartBasicInput_.getDataTransferListener();
    SetProcessHandlerToReq(req, handler);
    // 客户端限速
    auto limiter = uploadPartBasicInput_.getRateLimiter();
    SetRateLimiterToReq(req, limiter);
    // crc64校验
    SetCrc64ParmToReq(req);
    if (hashCrc64ecma != nullptr) {
        req->setCheckCrc64(true);
    }
    // 针对 uploadFromFile 场景，content 存在 offset
    req->setContentOffset(req->getContent()->tellg());
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    // crc64校验
    if (hashCrc64ecma != nullptr) {
        *hashCrc64ecma = tosRes.result()->getHashCrc64Result();
    }
    if (config_.isEnableCrc()) {
        auto hashcrc64 = std::stoull(tosRes.result()->findHeader(HEADER_CRC64));
        if (tosRes.result()->getHashCrc64Result() != hashcrc64) {
            TosError error;
            error.setIsClientError(true);
            error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
            res.setE(error);
            res.setSuccess(false);
            return res;
        }
    }
    UploadPartV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setPartNumber(input.getUploadPartBasicInput().getPartNumber());
    output.setETag(tosRes.result()->findHeader(http::HEADER_ETAG));
    output.setSsecAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSsecMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    output.setHashCrc64ecma(stoull(tosRes.result()->findHeader(HEADER_CRC64)));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, UploadPartFromFileOutput> TosClientImpl::uploadPartFromFile(const UploadPartFromFileInput& input,
                                                                              std::shared_ptr<uint64_t> hashCrc64ecma) {
    Outcome<TosError, UploadPartFromFileOutput> res;
    auto check = isValidFilePath(input.getFilePath());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("invalid file path, the file does not exist");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto content = std::make_shared<std::fstream>(input.getFilePath(), std::ios::in | std::ios_base::binary);
    if (!content->good()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("open file failed");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    content->seekg(input.getOffset(), content->beg);
    UploadPartV2Input input_(input.getUploadPartBasicInput(), content, input.getPartSize());
    auto res_ = this->uploadPart(input_, hashCrc64ecma);
    if (!res_.isSuccess()) {
        res.setE(res_.error());
        res.setSuccess(false);
        return res;
    }
    UploadPartFromFileOutput output_file;
    output_file.setUploadPartV2Output(res_.result());
    res.setSuccess(true);
    res.setR(output_file);
    return res;
}
Outcome<TosError, CompleteMultipartUploadOutput> TosClientImpl::completeMultipartUpload(
        const std::string& bucket, CompleteMultipartUploadInput& input) {
    Outcome<TosError, CompleteMultipartUploadOutput> res;

    std::string check = isValidNames(bucket, {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadedParts().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadedParts is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    int partsNum = input.getUploadedPartsLength();
    inner::InnerCompleteMultipartUploadInput multipart(partsNum);
    for (int i = 0; i < partsNum; ++i) {
        inner::InnerUploadedPart part(input.getUploadedParts()[i].getPartNumber(),
                                      input.getUploadedParts()[i].getEtag());
        multipart.setPartsByIdx(part, i);
    }
    multipart.sort();
    auto content = std::make_shared<std::stringstream>(multipart.toJsonString());
    auto rb = newBuilder(bucket, input.getKey());
    rb.withQuery("uploadId", input.getUploadId());
    auto req = rb.Build(http::MethodPost, content);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CompleteMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setCrc64(tosRes.result()->findHeader(HEADER_CRC64));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, CompleteMultipartUploadOutput> TosClientImpl::completeMultipartUpload(
        const std::string& bucket, CompleteMultipartCopyUploadInput& input) {
    Outcome<TosError, CompleteMultipartUploadOutput> res;
    int partsNum = input.getUploadedPartsLength();
    inner::InnerCompleteMultipartUploadInput multipart(partsNum);
    for (int i = 0; i < partsNum; ++i) {
        inner::InnerUploadedPart part(input.getUploadedParts()[i].getPartNumber(),
                                      input.getUploadedParts()[i].getEtag());
        multipart.setPartsByIdx(part, i);
    }
    multipart.sort();
    auto content = std::make_shared<std::stringstream>(multipart.toJsonString());
    auto rb = newBuilder(bucket, input.getKey());
    rb.withQuery("uploadId", input.getUploadId());
    auto req = rb.Build(http::MethodPost, content);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CompleteMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, CompleteMultipartUploadV2Output> TosClientImpl::completeMultipartUpload(
        const CompleteMultipartUploadV2Input& input) {
    Outcome<TosError, CompleteMultipartUploadV2Output> res;

    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getParts().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter Parts is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }

    std::shared_ptr<std::stringstream> ss = nullptr;
    std::string jsonParts(input.toJsonString());
    if (jsonParts != "null") {
        ss = std::make_shared<std::stringstream>(jsonParts);
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("uploadId", input.getUploadId());
    auto req = rb.Build(http::MethodPost, ss);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    CompleteMultipartUploadV2Output output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setHashCrc64ecma(stoull(tosRes.result()->findHeader(HEADER_CRC64)));
    std::stringstream ss_;
    ss_ << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss_.str());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, AbortMultipartUploadOutput> TosClientImpl::abortMultipartUpload(
        const std::string& bucket, const AbortMultipartUploadInput& input) {
    Outcome<TosError, AbortMultipartUploadOutput> res;
    std::string check = isValidNames(bucket, {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getKey());
    rb.withQuery("uploadId", input.getUploadId());
    auto req = rb.Build(http::MethodDelete, nullptr);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    AbortMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, AbortMultipartUploadOutput> TosClientImpl::abortMultipartUpload(
        const AbortMultipartUploadInput& input) {
    Outcome<TosError, AbortMultipartUploadOutput> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("uploadId", input.getUploadId());
    auto req = rb.Build(http::MethodDelete, nullptr);
    // 设置funcName
    req->setFuncName(__func__);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    AbortMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
Outcome<TosError, ListUploadedPartsOutput> TosClientImpl::listUploadedParts(const std::string& bucket,
                                                                            const ListUploadedPartsInput& input) {
    Outcome<TosError, ListUploadedPartsOutput> res;
    std::string check = isValidKey(input.getKey());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getKey());
    this->listUploadedParts(rb, input.getUploadId(), res);
    return res;
}
Outcome<TosError, ListUploadedPartsOutput> TosClientImpl::listUploadedParts(const std::string& bucket,
                                                                            const ListUploadedPartsInput& input,
                                                                            const RequestOptionBuilder& builder) {
    Outcome<TosError, ListUploadedPartsOutput> res;
    std::string check = isValidKey(input.getKey());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, input.getKey(), builder);
    this->listUploadedParts(rb, input.getUploadId(), res);
    return res;
}

Outcome<TosError, ListPartsOutput> TosClientImpl::listParts(const ListPartsInput& input) {
    Outcome<TosError, ListPartsOutput> res;
    std::string check = isValidNames(input.getBucket(), {input.getKey()});
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getUploadId().empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("tos: Parameter UploadId is not set");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(input.getBucket(), input.getKey());
    rb.withQuery("uploadId", input.getUploadId());

    if (input.getMaxParts() != 0) {
        rb.withQueryCheckEmpty("max-parts", std::to_string(input.getMaxParts()));
    }
    if (input.getPartNumberMarker() != 0) {
        rb.withQueryCheckEmpty("part-number-marker", std::to_string(input.getPartNumberMarker()));
    }

    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListPartsOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, ListMultipartUploadsOutput> TosClientImpl::listMultipartUploads(
        const std::string& bucket, const ListMultipartUploadsInput& input) {
    Outcome<TosError, ListMultipartUploadsOutput> res;
    auto rb = newBuilder(bucket, "");
    rb.withQuery("uploads", "");
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("key-marker", input.getKeyMarker());
    rb.withQueryCheckEmpty("upload-id-marker", input.getUploadIdMarker());
    if (input.getMaxUploads() != 0) {
        rb.withQueryCheckEmpty("max-uploads", std::to_string(input.getMaxUploads()));
    }

    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListMultipartUploadsOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}
static void listMultipartUploadsSetOptionHeader(RequestBuilder& rb, const ListMultipartUploadsV2Input& input) {
    rb.withQueryCheckEmpty("delimiter", input.getDelimiter());
    rb.withQueryCheckEmpty("encoding-type", input.getEncodingType());
    if (input.getMaxUploads() != 0) {
        rb.withQueryCheckEmpty("max-uploads", std::to_string(input.getMaxUploads()));
    }
    rb.withQueryCheckEmpty("prefix", input.getPrefix());
    rb.withQueryCheckEmpty("key-marker", input.getKeyMarker());
    rb.withQueryCheckEmpty("upload-id-marker", input.getUploadIdMarker());
}

Outcome<TosError, ListMultipartUploadsV2Output> TosClientImpl::listMultipartUploads(
        const ListMultipartUploadsV2Input& input) {
    Outcome<TosError, ListMultipartUploadsV2Output> res;
    auto rb = newBuilder(input.getBucket(), "");
    rb.withQuery("uploads", "");
    listMultipartUploadsSetOptionHeader(rb, input);
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return res;
    }
    ListMultipartUploadsV2Output output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
    return res;
}

Outcome<TosError, std::string> TosClientImpl::preSignedURL(const std::string& httpMethod, const std::string& bucket,
                                                           const std::string& objectKey,
                                                           std::chrono::duration<int> ttl) {
    Outcome<TosError, std::string> res;
    std::string check = isValidKey(objectKey);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey);
    this->preSignedURL(rb, httpMethod, ttl, res);
    return res;
}
Outcome<TosError, std::string> TosClientImpl::preSignedURL(const std::string& httpMethod, const std::string& bucket,
                                                           const std::string& objectKey, std::chrono::duration<int> ttl,
                                                           const RequestOptionBuilder& builder) {
    Outcome<TosError, std::string> res;
    std::string check = isValidKey(objectKey);
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto rb = newBuilder(bucket, objectKey, builder);
    this->preSignedURL(rb, httpMethod, ttl, res);
    return res;
}

Outcome<TosError, PreSignedURLOutput> TosClientImpl::preSignedURL(const PreSignedURLInput& input) {
    Outcome<TosError, PreSignedURLOutput> res;
    std::string check = isValidKey(input.getKey());
    if (!check.empty()) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage(check);
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    if (input.getExpires() <= 0 || input.getExpires() > 604800) {
        TosError error;
        error.setIsClientError(true);
        error.setMessage("invalid expires, the expires must be [1, 604800]");
        res.setE(error);
        res.setSuccess(false);
        return res;
    }
    auto headers = input.getHeader();
    auto querys = input.getQuery();
    auto rb = newBuilder(input.getBucket(), input.getKey(), input.getAlternativeEndpoint(), headers, querys);
    auto req = rb.build(HttpMethodTypetoString[input.getHttpMethod()]);
    std::chrono::duration<int> ttl(input.getExpires());
    auto query = signer_->signQuery(req, ttl);
    for (auto& iter : query) {
        req->setSingleQuery(iter.first, iter.second);
    }
    auto url = req->toUrl().toString();
    PreSignedURLOutput output;
    output.setSignUrl(url);
    output.setSignHeader(input.getHeader());
    res.setR(output);
    res.setSuccess(true);
}

std::set<std::string> CanRetryMethods = {"createBucket",
                                         "deleteBucket",
                                         "createMultipartUpload",
                                         "completeMultipartUpload",
                                         "abortMultipartUpload",
                                         "setObjectMeta",
                                         "putObjectAcl",
                                         "deleteObject",
                                         "putObject",
                                         "uploadPart"};
bool findInCanRetryMethods(std::string method) {
    auto pos = CanRetryMethods.find(method);
    if (pos != CanRetryMethods.end()) {
        return true;
    }
    return false;
}

bool TosClientImpl::checkShouldRetry(const std::shared_ptr<TosRequest>& request,
                                     const std::shared_ptr<TosResponse>& response) {
    auto resCode = response->getStatusCode();
    bool timeout = (response->getStatusMsg() == "operation timeout");
    if (resCode == 429 || resCode >= 500 || timeout) {
        if (request->getMethod() == http::MethodGet || request->getMethod() == http::MethodHead) {
            if (request->getMethod() == http::MethodGet) {
                auto content_ = response->getContent();
                if (content_ != nullptr) {
                    int64_t contentLength_ = calContentLength(content_);
                    if (contentLength_ != 0) {
                        return false;
                    }
                }
            }
            return true;
        }
        if ((resCode == 429 || resCode >= 500) && findInCanRetryMethods(request->getFuncName())) {
            if (request->getFuncName() == "putObject" || request->getFuncName() == "uploadPart") {
                auto content = response->getContent();
                int64_t offset = request->getContentOffset();
                content->seekg(offset, content->beg);
                if (content->bad() || content->fail()) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}
Outcome<TosError, std::shared_ptr<TosResponse>> TosClientImpl::roundTrip(const std::shared_ptr<TosRequest>& request,
                                                                         int expectedCode) {
    Outcome<TosError, std::shared_ptr<TosResponse>> ret;
    auto logger = LogUtils::GetLogger();
    auto rateLimiter = request->getRataLimiter();
    auto maxRetry = config_.getMaxRetryCount() < 1 ? 1 : config_.getMaxRetryCount();
    for (int retry = 0; retry < maxRetry; retry++) {
        if (retry != 0) {
            TimeUtils::sleepMilliSecondTimes(config_.getRetrySleepScale() * (1 << retry));
        }
        auto startTime = std::chrono::high_resolution_clock::now();
        // 实际进行一次请求
        auto resp = transport_->roundTrip(request);
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = endTime - startTime;
        if (resp->getStatusCode() == expectedCode) {
            if (logger != nullptr) {
                logger->info("Response StatusCode:{}, RequestId:{}, Cost:{} ms", resp->getStatusCode(),
                             resp->getRequestID(), fp_ms.count());
            }
            ret.setR(resp);
            ret.setSuccess(true);
            return ret;
        } else if (checkShouldRetry(request, resp) && retry < maxRetry) {
            if (logger != nullptr) {
                logger->info("http status code:{}, http error:{}, func name:{}, will retry once", resp->getStatusCode(),
                             resp->getStatusMsg(), request->getFuncName());
            }
            continue;
        } else {
            // check error
            ret.setSuccess(false);
            TosError se;
            if (resp->getStatusMsg() == "operation timeout") {
                se.setIsClientError(true);
                se.setMessage("http request timeout");
                se.setCode("operation timeout");
                ret.setE(se);
                if (logger != nullptr) {
                    logger->info("http status code:{}, http error:{}", resp->getStatusCode(), se.getCode());
                }
                return ret;
            }
            if (resp->getStatusCode() >= 300) {
                if (resp->getContent()) {
                    std::stringstream ss;
                    ss << resp->getContent()->rdbuf();
                    std::string error = ss.str();
                    se.fromJsonString(error);
                    if (se.getRequestId().empty() && se.getMessage().empty() && se.getCode().empty() &&
                        se.getHostId().empty()) {
                        // json parse error
                        se.setMessage(error);
                        se.setCode("unable to do serialization");
                        se.setIsClientError(true);
                    }
                    se.setStatusCode(resp->getStatusCode());
                    ret.setE(se);
                    if (logger != nullptr) {
                        logger->info("http status code:{}, http error:{}", resp->getStatusCode(), se.getCode());
                    }
                    return ret;
                }
                // 特别处理 404
                if (resp->getStatusCode() == 404) {
                    se.setCode("not found");
                    se.setStatusCode(resp->getStatusCode());
                    se.setRequestId(resp->getRequestID());
                    ret.setE(se);
                    if (logger != nullptr) {
                        logger->info("http status code:{}, http error:{}", resp->getStatusCode(), se.getCode());
                    }
                    return ret;
                }
            }

            se.setStatusCode(resp->getStatusCode());
            se.setCode("UnexpectedStatusCode error");
            se.setMessage(resp->getStatusMsg());
            se.setRequestId(resp->getRequestID());
            ret.setE(se);
            if (logger != nullptr) {
                logger->info("http status code:{}, http error:{}", resp->getStatusCode(), se.getCode());
            }
            return ret;
        }
    }
}

RequestBuilder TosClientImpl::newBuilder(const std::string& bucket, const std::string& object) {
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queries;
    auto rb = RequestBuilder(signer_, scheme_, host_, bucket, object, urlMode_, headers, queries);
    rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
    return rb;
}
RequestBuilder TosClientImpl::newBuilder(const std::string& bucket, const std::string& object,
                                         const std::string& alternativeEndpoint,
                                         const std::map<std::string, std::string>& headers,
                                         std::map<std::string, std::string>& queries) {
    // todo: alternativeEndpoint 不带桶名，当成 client的endpoint，可能要处理一下 schema
    std::string host = alternativeEndpoint.empty() ? host_ : alternativeEndpoint;
    auto rb = RequestBuilder(signer_, scheme_, host, bucket, object, urlMode_, headers, queries);
    rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
    return rb;
}
RequestBuilder TosClientImpl::newBuilder(const std::string& bucket, const std::string& object,
                                         const RequestOptionBuilder& builder) {
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queries;
    auto rb = RequestBuilder(signer_, scheme_, host_, bucket, object, urlMode_, headers, queries);
    rb.withHeader(http::HEADER_USER_AGENT, userAgent_);
    auto headerIter = builder.getHeaders().begin();
    for (; headerIter != builder.getHeaders().end(); ++headerIter) {
        rb.withHeader(headerIter->first, headerIter->second);
    }
    auto queryIter = builder.getQueries().begin();
    for (; queryIter != builder.getQueries().end(); ++queryIter) {
        rb.withQuery(queryIter->first, queryIter->second);
    }
    rb.setContentLength(builder.getContentLength());
    rb.setAutoRecognizeContentType(builder.isAutoRecognizeContentType());
    rb.setRange(builder.getRange());
    return rb;
}

void TosClientImpl::getObject(RequestBuilder& rb, Outcome<TosError, GetObjectOutput>& res) {
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, expectedCode(rb));
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    GetObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setContentRange(rb.findHeader(http::HEADER_CONTENT_RANGE));
    output.setContent(tosRes.result()->getContent());
    output.setObjectMetaFromResponse(*tosRes.result());
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::headObject(RequestBuilder& rb, Outcome<TosError, HeadObjectOutput>& res) {
    auto req = rb.Build(http::MethodHead, nullptr);
    auto tosRes = roundTrip(req, expectedCode(rb));
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    HeadObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setContentRange(rb.findHeader(http::HEADER_CONTENT_RANGE));
    output.setObjectMeta(*(tosRes.result()));
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::deleteObject(RequestBuilder& rb, Outcome<TosError, DeleteObjectOutput>& res) {
    auto req = rb.Build(http::MethodDelete, nullptr);
    auto tosRes = roundTrip(req, 204);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    DeleteObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setDeleteMarker(tosRes.result()->findHeader(HEADER_DELETE_MARKER) == "true");
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::deleteMultiObjects(RequestBuilder& rb, const std::string& data, const std::string& dataMd5,
                                       Outcome<TosError, DeleteMultiObjectsOutput>& res) {
    rb.withHeader(http::HEADER_CONTENT_MD5, dataMd5);
    rb.withQuery("delete", "");
    auto req = rb.Build(http::MethodPost, std::make_shared<std::stringstream>(data));
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    std::stringstream out;
    out << tosRes.result()->getContent()->rdbuf();
    DeleteMultiObjectsOutput output;
    output.fromJsonString(out.str());
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::putObject(const std::shared_ptr<TosRequest>& req, Outcome<TosError, PutObjectOutput>& res) {
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    PutObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setCrc64(tosRes.result()->findHeader(HEADER_CRC64));
    output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSseCustomerKeyMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    output.setSseCustomerKey(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY));
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::appendObject(const std::shared_ptr<TosRequest>& req, Outcome<TosError, AppendObjectOutput>& res) {
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    auto nextOffset = tosRes.result()->findHeader(HEADER_NEXT_APPEND_OFFSET);
    AppendObjectOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
    output.setNextAppendOffset(stoi(nextOffset));
    output.setCrc64(tosRes.result()->findHeader(HEADER_CRC64));
    res.setSuccess(true);
    res.setR(output);
}

void TosClientImpl::setObjectMeta(RequestBuilder& rb, Outcome<TosError, SetObjectMetaOutput>& res) {
    rb.withQuery("metadata", "");
    auto req = rb.Build(http::MethodPost, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    SetObjectMetaOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::copyObject(const std::string& dstBucket, const std::string& dstObject, const std::string& srcBucket,
                               const std::string& srcObject, Outcome<TosError, CopyObjectOutput>& outcome) {
    auto rb = newBuilder(dstBucket, dstObject);
    auto req = rb.BuildWithCopySource(http::MethodPut, srcBucket, srcObject);
    this->copyObject(req, outcome);
}
void TosClientImpl::copyObject(const std::string& dstBucket, const std::string& dstObject, const std::string& srcBucket,
                               const std::string& srcObject, const RequestOptionBuilder& builder,
                               Outcome<TosError, CopyObjectOutput>& outcome) {
    auto rb = newBuilder(dstBucket, dstObject, builder);
    auto req = rb.BuildWithCopySource(http::MethodPut, srcBucket, srcObject);
    this->copyObject(req, outcome);
}
void TosClientImpl::copyObject(std::shared_ptr<TosRequest>& req, Outcome<TosError, CopyObjectOutput>& res) {
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    CopyObjectOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setCrc64(tosRes.result()->findHeader(HEADER_CRC64));
    res.setSuccess(true);
    res.setR(output);
}

void TosClientImpl::uploadPartCopy(RequestBuilder& rb, const UploadPartCopyInput& input,
                                   Outcome<TosError, UploadPartCopyOutput>& res) {
    rb.withQuery("partNumber", std::to_string(input.getPartNumber()));
    rb.withQuery("uploadId", input.getUploadId());
    rb.withQueryCheckEmpty("versionId", input.getSourceVersionId());
    rb.withHeader(HEADER_COPY_SOURCE_RANGE, copyRange(input.getStartOffset(), input.getPartSize()));
    auto req = rb.BuildWithCopySource(http::MethodPut, input.getSourceBucket(), input.getSourceKey());
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    inner::InnerUploadPartCopyOutput out;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    out.fromJsonString(ss.str());
    UploadPartCopyOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setVersionId(tosRes.result()->findHeader(HEADER_VERSIONID));
    output.setSourceVersionId(tosRes.result()->findHeader(HEADER_COPY_SOURCE_VERSION_ID));
    output.setPartNumber(input.getPartNumber());
    output.setEtag(out.getEtag());
    output.setLastModified(out.getLastModified());
    output.setCrc64(tosRes.result()->findHeader(HEADER_CRC64));
    res.setSuccess(true);
    res.setR(output);
}

void TosClientImpl::getObjectAcl(RequestBuilder& rb, Outcome<TosError, GetObjectAclOutput>& res) {
    rb.withQuery("acl", "");
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    GetObjectAclOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::createMultipartUpload(RequestBuilder& rb, Outcome<TosError, CreateMultipartUploadOutput>& res) {
    rb.withQuery("uploads", "");
    auto req = rb.Build(http::MethodPost, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    inner::MultipartUpload upload;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    upload.fromJsonString(ss.str());
    CreateMultipartUploadOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setBucket(upload.getBucket());
    output.setKey(upload.getKey());
    output.setUploadId(upload.getUploadId());
    output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSseCustomerMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    output.setSseCustomerKey(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY));
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::uploadPart(RequestBuilder& rb, const UploadPartInput& input,
                               Outcome<TosError, UploadPartOutput>& res) {
    if ((int)input.getPartSize() != input.getPartSize()) {
        TosError se;
        std::string errMsg = "UploadPart method has overhead size: ";
        errMsg += std::to_string(input.getPartSize());
        se.setMessage(errMsg);
        res.setE(se);
        res.setSuccess(false);
        return;
    }
    rb.setContentLength(input.getPartSize());
    rb.withQuery("uploadId", input.getUploadId());
    rb.withQuery("partNumber", std::to_string(input.getPartNumber()));
    auto req = rb.Build(http::MethodPut, input.getContent());
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    UploadPartOutput output;
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    output.setPartNumber(input.getPartNumber());
    output.setEtag(tosRes.result()->findHeader(http::HEADER_ETAG));
    output.setSseCustomerAlgorithm(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_ALGORITHM));
    output.setSseCustomerMd5(tosRes.result()->findHeader(HEADER_SSE_CUSTOMER_KEY_MD5));
    res.setSuccess(true);
    res.setR(output);
}
void TosClientImpl::listUploadedParts(RequestBuilder& rb, const std::string& uploadId,
                                      Outcome<TosError, ListUploadedPartsOutput>& res) {
    rb.withQuery("uploadId", uploadId);
    auto req = rb.Build(http::MethodGet, nullptr);
    auto tosRes = roundTrip(req, 200);
    if (!tosRes.isSuccess()) {
        res.setE(tosRes.error());
        res.setSuccess(false);
        return;
    }
    ListUploadedPartsOutput output;
    std::stringstream ss;
    ss << tosRes.result()->getContent()->rdbuf();
    output.fromJsonString(ss.str());
    output.setRequestInfo(tosRes.result()->GetRequestInfo());
    res.setSuccess(true);
    res.setR(output);
}

void TosClientImpl::preSignedURL(RequestBuilder& rb, const std::string& method, const std::chrono::duration<int>& ttl,
                                 Outcome<TosError, std::string>& res) {
    auto req = rb.build(method);
    auto query = signer_->signQuery(req, ttl);
    for (auto& iter : query) {
        req->setSingleQuery(iter.first, iter.second);
    }
    auto url = req->toUrl().toString();
    res.setR(url);
    res.setSuccess(true);
}
