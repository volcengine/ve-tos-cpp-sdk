#pragma once
#include "CreateMultipartUploadInput.h"
#include "Type.h"
#include <string>
#include <utility>
namespace VolcengineTos {
class UploadFileV2Input {
public:
    UploadFileV2Input(std::string bucket, std::string key)
            : createMultipartUploadInput_(std::move(bucket), std::move(key)) {
    }
    UploadFileV2Input() = default;
    virtual ~UploadFileV2Input() = default;
    const CreateMultipartUploadInput& getCreateMultipartUploadInput() const {
        return createMultipartUploadInput_;
    }
    void setCreateMultipartUploadInput(const CreateMultipartUploadInput& createmultipartuploadinput) {
        createMultipartUploadInput_ = createmultipartuploadinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
    }
    int getTaskNum() const {
        return taskNum_;
    }
    void setTaskNum(int tasknum) {
        taskNum_ = tasknum;
    }
    bool isEnableCheckpoint() const {
        return enableCheckpoint_;
    }
    void setEnableCheckpoint(bool enablecheckpoint) {
        enableCheckpoint_ = enablecheckpoint;
    }
    const std::string& getCheckpointFile() const {
        return checkpointFile_;
    }
    void setCheckpointFile(const std::string& checkpointfile) {
        checkpointFile_ = checkpointfile;
    }
    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        dataTransferListener_ = datatransferlistener;
    }
    const UploadEventListener& getUploadEventListener() const {
        return uploadEventListener_;
    }
    void setUploadEventListener(const UploadEventListener& uploadeventlistener) {
        uploadEventListener_ = uploadeventlistener;
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        rateLimiter_ = ratelimiter;
    }
    void setCancelHook1(const std::shared_ptr<CancelHook>& cancelhook) {
        cancelHook_ = cancelhook;
    }
    const std::shared_ptr<CancelHook>& getCancelHook() const {
        return cancelHook_;
    }
    void setCancelHook(const std::shared_ptr<CancelHook>& cancelhook) {
        cancelHook_ = cancelhook;
    }

    const std::string& getBucket() const {
        return createMultipartUploadInput_.getBucket();
    }
    void setBucket(const std::string& bucket) {
        createMultipartUploadInput_.setBucket(bucket);
    }
    const std::string& getKey() const {
        return createMultipartUploadInput_.getKey();
    }
    void setKey(const std::string& key) {
        createMultipartUploadInput_.setKey(key);
    }
    const std::string& getEncodingType() const {
        return createMultipartUploadInput_.getEncodingType();
    }
    void setEncodingType(const std::string& encodingtype) {
        createMultipartUploadInput_.setEncodingType(encodingtype);
    }
    const std::string& getCacheControl() const {
        return createMultipartUploadInput_.getCacheControl();
    }
    void setCacheControl(const std::string& cachecontrol) {
        createMultipartUploadInput_.setCacheControl(cachecontrol);
    }
    const std::string& getContentDisposition() const {
        return createMultipartUploadInput_.getContentDisposition();
    }
    void setContentDisposition(const std::string& contentdisposition) {
        createMultipartUploadInput_.setContentDisposition(contentdisposition);
    }
    const std::string& getContentEncoding() const {
        return createMultipartUploadInput_.getContentEncoding();
    }
    void setContentEncoding(const std::string& contentencoding) {
        createMultipartUploadInput_.setContentEncoding(contentencoding);
    }
    const std::string& getContentLanguage() const {
        return createMultipartUploadInput_.getContentLanguage();
    }
    void setContentLanguage(const std::string& contentlanguage) {
        createMultipartUploadInput_.setContentLanguage(contentlanguage);
    }
    const std::string& getContentType() const {
        return createMultipartUploadInput_.getContentType();
    }
    void setContentType(const std::string& contenttype) {
        createMultipartUploadInput_.setContentType(contenttype);
    }
    time_t getExpires() const {
        return createMultipartUploadInput_.getExpires();
    }
    void setExpires(time_t expires) {
        createMultipartUploadInput_.setExpires(expires);
    }
    ACLType getAcl() const {
        return createMultipartUploadInput_.getAcl();
    }
    void setAcl(ACLType acl) {
        createMultipartUploadInput_.setAcl(acl);
    }
    const std::string& getGrantFullControl() const {
        return createMultipartUploadInput_.getGrantFullControl();
    }
    void setGrantFullControl(const std::string& grantfullcontrol) {
        createMultipartUploadInput_.setGrantFullControl(grantfullcontrol);
    }
    const std::string& getGrantRead() const {
        return createMultipartUploadInput_.getGrantRead();
    }
    void setGrantRead(const std::string& grantread) {
        createMultipartUploadInput_.setGrantRead(grantread);
    }
    const std::string& getGrantReadAcp() const {
        return createMultipartUploadInput_.getGrantReadAcp();
    }
    void setGrantReadAcp(const std::string& grantreadacp) {
        createMultipartUploadInput_.setGrantReadAcp(grantreadacp);
    }
    const std::string& getGrantWriteAcp() const {
        return createMultipartUploadInput_.getGrantWriteAcp();
    }
    void setGrantWriteAcp(const std::string& grantwriteacp) {
        createMultipartUploadInput_.setGrantWriteAcp(grantwriteacp);
    }
    const std::string& getSsecAlgorithm() const {
        return createMultipartUploadInput_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        createMultipartUploadInput_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKeyMd5() const {
        return createMultipartUploadInput_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        createMultipartUploadInput_.setSsecKeyMd5(sseckeymd5);
    }
    const std::string& getSsecKey() const {
        return createMultipartUploadInput_.getSsecKey();
    }
    void setSsecKey(const std::string& sseckey) {
        createMultipartUploadInput_.setSsecKey(sseckey);
    }
    const std::string& getServerSideEncryption() const {
        return createMultipartUploadInput_.getServerSideEncryption();
    }
    void setServerSideEncryption(const std::string& serversideencryption) {
        createMultipartUploadInput_.setServerSideEncryption(serversideencryption);
    }
    const std::map<std::string, std::string>& getMeta() const {
        return createMultipartUploadInput_.getMeta();
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        createMultipartUploadInput_.setMeta(meta);
    }
    const std::string& getWebsiteRedirectLocation() const {
        return createMultipartUploadInput_.getWebsiteRedirectLocation();
    }
    void setWebsiteRedirectLocation(const std::string& websiteredirectlocation) {
        createMultipartUploadInput_.setWebsiteRedirectLocation(websiteredirectlocation);
    }
    StorageClassType getStorageClass() const {
        return createMultipartUploadInput_.getStorageClass();
    }
    void setStorageClass(StorageClassType storageclass) {
        createMultipartUploadInput_.setStorageClass(storageclass);
    }

private:
    CreateMultipartUploadInput createMultipartUploadInput_;
    std::string filePath_;
    int64_t partSize_ = 20 * 1024 * 1024;  // 默认20MB分片大小
    int taskNum_ = 1;
    bool enableCheckpoint_ = false;
    std::string checkpointFile_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    UploadEventListener uploadEventListener_ = {nullptr};
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;
    std::shared_ptr<CancelHook> cancelHook_ = nullptr;
};
}  // namespace VolcengineTos
