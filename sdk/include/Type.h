#pragma once
#include <map>
#include <mutex>
#include <iostream>
#include <atomic>
#include <memory>
#include <ctime>
#include <functional>
namespace VolcengineTos {
enum class ACLType {
    NotSet = 0,
    Private,
    PublicRead,
    PublicReadWrite,
    AuthenticatedRead,
    BucketOwnerRead,
    BucketOwnerFullControl
};
static std::map<ACLType, std::string> ACLTypetoString{{ACLType::NotSet, ""},
                                                      {ACLType::Private, "private"},
                                                      {ACLType::PublicRead, "public-read"},
                                                      {ACLType::PublicReadWrite, "public-read-write"},
                                                      {ACLType::AuthenticatedRead, "authenticated-read"},
                                                      {ACLType::BucketOwnerRead, "bucket-owner-read"},
                                                      {ACLType::BucketOwnerFullControl, "bucket-owner-full-control"}};
static std::map<std::string, ACLType> StringtoACLType{{"private", ACLType::Private},
                                                      {"public-read", ACLType::PublicRead},
                                                      {"public-read-write", ACLType::PublicReadWrite},
                                                      {"authenticated-read", ACLType::AuthenticatedRead},
                                                      {"bucket-owner-read", ACLType::BucketOwnerRead},
                                                      {"bucket-owner-full-control", ACLType::BucketOwnerFullControl}};

enum class StorageClassType { NotSet = 0, STANDARD, IA };
static std::map<StorageClassType, std::string> StorageClassTypetoString{
        {StorageClassType::NotSet, ""}, {StorageClassType::STANDARD, "STANDARD"}, {StorageClassType::IA, "IA"}};
static std::map<std::string, StorageClassType> StringtoStorageClassType{{"STANDARD", StorageClassType::STANDARD},
                                                                        {"IA", StorageClassType::IA}};

enum MetadataDirectiveType { COPY = 0, REPLACE };
static std::map<MetadataDirectiveType, std::string> MetadataDirectiveTypetoString{{COPY, "COPY"}, {REPLACE, "REPLACE"}};
static std::map<std::string, MetadataDirectiveType> StringtoMetadataDirectiveType{{"COPY", COPY}, {"REPLACE", REPLACE}};

enum class AzRedundancyType { NotSet = 0, SingleAz, MultiAz };
static std::map<AzRedundancyType, std::string> AzRedundancyTypetoString{{AzRedundancyType::NotSet, ""},
                                                                        {AzRedundancyType::SingleAz, "single-az"},
                                                                        {AzRedundancyType::MultiAz, "multi-az"}};
static std::map<std::string, AzRedundancyType> StringtoAzRedundancyType{{"single-az", AzRedundancyType::SingleAz},
                                                                        {"multi-az", AzRedundancyType::MultiAz}};

enum class PermissionType { NotSet = 0, Read, Write, ReadAcp, WriteAcp, FullControl };
static std::map<PermissionType, std::string> PermissionTypetoString{{PermissionType::NotSet, ""},
                                                                    {PermissionType::Read, "Read"},
                                                                    {PermissionType::Write, "Write"},
                                                                    {PermissionType::ReadAcp, "READ_ACP"},
                                                                    {PermissionType::WriteAcp, "WRITE_ACP"},
                                                                    {PermissionType::FullControl, "FULL_CONTROL"}};
static std::map<std::string, PermissionType> StringtoPermissionType{{"", PermissionType::NotSet},
                                                                    {"Read", PermissionType::Read},
                                                                    {"Write", PermissionType::Write},
                                                                    {"READ_ACP", PermissionType::ReadAcp},
                                                                    {"WRITE_ACP", PermissionType::WriteAcp},
                                                                    {"FULL_CONTROL", PermissionType::FullControl}};

enum class GranteeType { NotSet = 0, Group, CanonicalUser };
static std::map<GranteeType, std::string> GranteeTypetoString{
        {GranteeType::NotSet, ""}, {GranteeType::Group, "Group"}, {GranteeType::CanonicalUser, "CanonicalUser"}};
static std::map<std::string, GranteeType> StringtoGranteeType{
        {"", GranteeType::NotSet}, {"Group", GranteeType::Group}, {"CanonicalUser", GranteeType::CanonicalUser}};

enum class CannedType { NotSet = 0, AllUsers, AuthenticatedUsers };
static std::map<CannedType, std::string> CannedTypetoString{{CannedType::NotSet, ""},
                                                            {CannedType::AllUsers, "AllUsers"},
                                                            {CannedType::AuthenticatedUsers, "AuthenticatedUsers"}};
static std::map<std::string, CannedType> StringtoCannedType{{"", CannedType::NotSet},
                                                            {"AllUsers", CannedType::AllUsers},
                                                            {"AuthenticatedUsers", CannedType::AuthenticatedUsers}};

enum class HttpMethodType { NotSet = 0, Get, Put, Post, Delete, Head };
static std::map<HttpMethodType, std::string> HttpMethodTypetoString{
        {HttpMethodType::NotSet, ""},   {HttpMethodType::Get, "GET"},       {HttpMethodType::Put, "PUT"},
        {HttpMethodType::Post, "POST"}, {HttpMethodType::Delete, "DELETE"}, {HttpMethodType::Head, "HEAD"}};
static std::map<std::string, HttpMethodType> StringtoHttpMethodType{
        {"", HttpMethodType::NotSet},   {"GET", HttpMethodType::Get},       {"PUT", HttpMethodType::Put},
        {"POST", HttpMethodType::Post}, {"DELETE", HttpMethodType::Delete}, {"HEAD", HttpMethodType::Head}};

enum class StatusType { NotSet = 0, StatusEnabled, StatusDisabled };
static std::map<StatusType, std::string> StatusTypetoString{
        {StatusType::NotSet, ""}, {StatusType::StatusEnabled, "Enabled"}, {StatusType::StatusDisabled, "Disabled"}};
static std::map<std::string, StatusType> StringtoStatusType{
        {"", StatusType::NotSet}, {"Enabled", StatusType::StatusEnabled}, {"Disabled", StatusType::StatusDisabled}};

enum class RedirectType { NotSet = 0, RedirectMirror, RedirectAsync };
static std::map<RedirectType, std::string> RedirectTypetoString{
        {RedirectType::NotSet, ""}, {RedirectType::RedirectMirror, "Mirror"}, {RedirectType::RedirectAsync, "Async"}};
static std::map<std::string, RedirectType> StringtoRedirectType{
        {"", RedirectType::NotSet}, {"Mirror", RedirectType::RedirectMirror}, {"Async", RedirectType::RedirectAsync}};

enum class StorageClassInheritDirectiveType { NotSet = 0, StorageClassIDDestinationBucket, StorageClassIDSourceObject };
static std::map<StorageClassInheritDirectiveType, std::string> StorageClassInheritDirectiveTypetoString{
        {StorageClassInheritDirectiveType::NotSet, ""},
        {StorageClassInheritDirectiveType::StorageClassIDDestinationBucket, "DESTINATION_BUCKET"},
        {StorageClassInheritDirectiveType::StorageClassIDSourceObject, "SOURCE_OBJECT"}};
static std::map<std::string, StorageClassInheritDirectiveType> StringtoStorageClassInheritDirectiveType{
        {"", StorageClassInheritDirectiveType::NotSet},
        {"DESTINATION_BUCKET", StorageClassInheritDirectiveType::StorageClassIDDestinationBucket},
        {"SOURCE_OBJECT", StorageClassInheritDirectiveType::StorageClassIDSourceObject}};

enum class VersioningStatusType { NotSet = 0, VersioningStatusEnabled, VersioningStatusSuspended };
static std::map<VersioningStatusType, std::string> VersioningStatusTypetoString{
        {VersioningStatusType::NotSet, ""},
        {VersioningStatusType::VersioningStatusEnabled, "Enabled"},
        {VersioningStatusType::VersioningStatusSuspended, "Suspended"}};
static std::map<std::string, VersioningStatusType> StringtoVersioningStatusType{
        {"", VersioningStatusType::NotSet},
        {"Enabled", VersioningStatusType::VersioningStatusEnabled},
        {"Suspended", VersioningStatusType::VersioningStatusSuspended}};

enum class ProtocolType { NotSet = 0, ProtocolHttp, ProtocolHttps };
static std::map<ProtocolType, std::string> ProtocolTypetoString{
        {ProtocolType::NotSet, ""}, {ProtocolType::ProtocolHttp, "http"}, {ProtocolType::ProtocolHttps, "https"}};
static std::map<std::string, ProtocolType> StringtoProtocolType{
        {"", ProtocolType::NotSet}, {"http", ProtocolType::ProtocolHttp}, {"https", ProtocolType::ProtocolHttps}};

enum class CertStatusType { NotSet = 0, CertStatusBound, CertStatusUnbound, CertStatusExpired };
static std::map<CertStatusType, std::string> CertStatusTypetoString{{CertStatusType::NotSet, ""},
                                                                    {CertStatusType::CertStatusBound, "CertBound"},
                                                                    {CertStatusType::CertStatusUnbound, "CertUnbound"},
                                                                    {CertStatusType::CertStatusExpired, "CertExpired"}};
static std::map<std::string, CertStatusType> StringtoCertStatusType{{"", CertStatusType::NotSet},
                                                                    {"CertBound", CertStatusType::CertStatusBound},
                                                                    {"CertUnbound", CertStatusType::CertStatusUnbound},
                                                                    {"CertExpired", CertStatusType::CertStatusExpired}};

enum LogLevel {
    LogOff = 0,
    LogInfo,
    LogDebug,
};

// 支持进度条
using DataTransferType = int;
static const DataTransferType DataTransferStarted = 1;
static const DataTransferType DataTransferRW = 2;
static const DataTransferType DataTransferSucceed = 3;
static const DataTransferType DataTransferFailed = 4;
struct DataTransferStatus {
    int64_t consumedBytes_;
    int64_t totalBytes_;
    int64_t rwOnceBytes_;
    DataTransferType type_;
    void* userData_;
};
using DataTransferStatusChange = std::function<void(std::shared_ptr<DataTransferStatus>)>;
struct DataTransferListener {
    DataTransferStatusChange dataTransferStatusChange_;
    void* userData_;
};

class UploadDownloadFileProcessStat {
public:
    std::mutex lock_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    int64_t totalBytes_ = 0;
    int64_t rwOnceBytes_ = 0;
    int64_t consumedBytes_ = 0;
    DataTransferType type_ = 1;
    bool startTrans_ = false;
    void* userData;
};
static void UploadDownloadFileProcessCallback(const std::shared_ptr<DataTransferStatus>& dataTransferStatus) {
    auto processStat = (UploadDownloadFileProcessStat*)dataTransferStatus->userData_;
    int64_t consumedBytes = dataTransferStatus->consumedBytes_;
    int64_t totalBytes = dataTransferStatus->totalBytes_;
    int64_t rwOnceBytes = dataTransferStatus->rwOnceBytes_;

    std::lock_guard<std::mutex> lck(processStat->lock_);
    processStat->consumedBytes_ += rwOnceBytes;

    if (dataTransferStatus->type_ == 4) {
        processStat->type_ = 4;
    } else {
        if (processStat->type_ == 1 && !processStat->startTrans_) {
            processStat->startTrans_ = true;
        } else if (processStat->type_ == 1 && processStat->startTrans_) {
            processStat->type_ = 2;
        }
        if (processStat->consumedBytes_ == processStat->totalBytes_) {
            processStat->type_ = 3;
        }
    }

    auto process = processStat->dataTransferListener_.dataTransferStatusChange_;
    if (process) {
        DataTransferStatus status{processStat->consumedBytes_, processStat->totalBytes_, rwOnceBytes,
                                  processStat->type_, processStat->dataTransferListener_.userData_};
        auto data = std::make_shared<DataTransferStatus>(status);
        process(data);
    }
}

// 限流limiter
class RateLimiter {
public:
    virtual std::pair<bool, time_t> Acquire(int64_t want) = 0;
};
//  容量最小是80kb
//  速度最小是1kb
class MyRateLimiter : public RateLimiter {
public:
    MyRateLimiter(int64_t capacity, int64_t rate) : capacity_(capacity), rate_(rate), currentAmount_(capacity) {
        lastTokenGiven_ = time(nullptr);
        if (capacity < 80000) {
            capacity_ = 80000;
        }
        if (rate < 1000) {
            rate_ = 1000;
        }
    }
    std::pair<bool, time_t> Acquire(int64_t want) override {
        // 内部维护一个时间，每次调用的时候先加上对应的令牌
        // 然后再判断桶里剩余的令牌是否足够
        std::lock_guard<std::mutex> lockGuard(mutex_);
        time_t differTime = time(nullptr) - lastTokenGiven_;
        int64_t tokenIncreaseAmount = differTime * rate_;
        int64_t tokenAmountMax = tokenIncreaseAmount + currentAmount_;
        currentAmount_ = tokenAmountMax > capacity_ ? capacity_ : tokenAmountMax;
        // 极端情况下，want > 总容量，直接放过
        if (want > capacity_) {
            std::pair<bool, std::time_t> res(true, 0);
            return res;
        }
        if (want > currentAmount_) {
            std::time_t timeToWait = (want - currentAmount_) / rate_;
            std::pair<bool, std::time_t> res(false, timeToWait);
            return res;
        }
        lastTokenGiven_ = time(nullptr);
        currentAmount_ = currentAmount_ - want;
        std::pair<bool, std::time_t> res(true, 0);
        return res;
    }

private:
    int64_t capacity_ = 0;
    int64_t rate_ = 0;
    time_t lastTokenGiven_;
    int64_t currentAmount_ = 0;
    std::mutex mutex_;
};
static MyRateLimiter* NewRateLimiter(int64_t capacity, int64_t rate) {
    auto* myRateLimiter = new MyRateLimiter(capacity, rate);
    return myRateLimiter;
};

// uploadfile 相关回调
struct UploadPartInfo {
    int partNumber_;
    int64_t partSize_;
    int64_t offset_;
    std::shared_ptr<std::string> eTag_;  // upload part succeed 事件发生时有值
    std::shared_ptr<uint64_t> hashCrc64ecma_;
};
using UploadEventType = int;
static const UploadEventType UploadEventCreateMultipartUploadSucceed = 1;
static const UploadEventType UploadEventCreateMultipartUploadFailed = 2;
static const UploadEventType UploadEventUploadPartSucceed = 3;
static const UploadEventType UploadEventUploadPartFailed = 4;
static const UploadEventType UploadEventUploadPartAborted = 5;
static const UploadEventType UploadEventCompleteMultipartUploadSucceed = 6;
static const UploadEventType UploadEventCompleteMultipartUploadFailed = 7;
struct UploadEvent {
    UploadEventType type_;
    bool error_;
    std::string bucket_;
    std::string key_;
    std::shared_ptr<std::string> uploadId_;
    std::shared_ptr<std::string> checkpointFile_;
    std::shared_ptr<UploadPartInfo> uploadPartInfo_;
};
using UploadEventChange = std::function<void(std::shared_ptr<UploadEvent>)>;
struct UploadEventListener {
    UploadEventChange eventChange_;
};
// downloadfile 相关回调
struct DownloadPartInfo {
    int partNumber_;
    int64_t rangeStart_;
    int64_t rangeEnd_;
};
using DownloadEventType = int;
static const DownloadEventType DownloadEventCreateMultipartDownloadSucceed = 1;
static const DownloadEventType DownloadEventCreateMultipartDownloadFailed = 2;
static const DownloadEventType DownloadEventDownloadPartSucceed = 3;
static const DownloadEventType DownloadEventDownloadPartFailed = 4;
static const DownloadEventType DownloadEventDownloadPartAborted = 5;
static const DownloadEventType DownloadEventCompleteMultipartDownloadSucceed = 6;
static const DownloadEventType DownloadEventCompleteMultipartDownloadFailed = 7;
struct DownloadEvent {
    DownloadEventType type_;
    bool error_;
    std::string bucket_;
    std::string key_;
    std::string versionId_;
    std::string filePath_;
    std::shared_ptr<std::string> checkpointFile_;
    std::shared_ptr<std::string> tempFilePath_;
    std::shared_ptr<DownloadPartInfo> downloadPartInfo_;
};
using DownloadEventChange = std::function<void(std::shared_ptr<DownloadEvent>)>;
struct DownloadEventListener {
    DownloadEventChange eventChange_;
};

class CancelHook {
public:
    virtual void Cancel(bool isAbort) = 0;
    virtual bool isAbortFunc() = 0;
    virtual bool isNotAbortFunc() = 0;
    virtual bool isCancel() = 0;
};
class MyCancelHook : public CancelHook {
public:
    MyCancelHook() {
        isCall_ = false;
        isAbort_ = false;
    }
    void Cancel(bool isAbort) override {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (!isCall_) {
            isCall_ = true;
            isAbort_ = isAbort;
        }
    };
    bool isCancel() override {
        bool res = isAbortFunc() || isNotAbortFunc();
        return res;
    }
    virtual ~MyCancelHook() = default;

    bool isAbortFunc() override {
        if (isCall_ && isAbort_) {
            return true;
        }
        return false;
    };
    bool isNotAbortFunc() override {
        if (isCall_ && !isAbort_) {
            return true;
        }
        return false;
    };

private:
    std::atomic<bool> isCall_;
    std::atomic<bool> isAbort_;
    std::mutex mutex_;
};
static MyCancelHook* NewCancelHook() {
    auto* myCancelHook = new MyCancelHook();
    return myCancelHook;
};

}  // namespace VolcengineTos
