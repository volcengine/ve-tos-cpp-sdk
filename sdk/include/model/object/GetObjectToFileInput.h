#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "GetObjectV2Input.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class GetObjectToFileInput : public GenericInput {
public:
    GetObjectToFileInput(std::string bucket, std::string key, std::string filepath)
            : getObjectInput_(std::move(bucket), std::move(key)), filePath_(std::move(filepath)) {
    }
    GetObjectToFileInput() = default;
    ~GetObjectToFileInput() = default;
    
    const GetObjectV2Input& getGetObjectInput() const {
        return getObjectInput_;
    }
    void setGetObjectInput(const GetObjectV2Input& getobjectinput) {
        getObjectInput_ = getobjectinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }

    const std::string& getBucket() const {
        return getObjectInput_.getBucket();
    }
    void setBucket(const std::string& bucket) {
        getObjectInput_.setBucket(bucket);
    }
    const std::string& getKey() const {
        return getObjectInput_.getKey();
    }
    void setKey(const std::string& key) {
        getObjectInput_.setKey(key);
    }
    const std::string& getVersionId() const {
        return getObjectInput_.getVersionId();
    }
    void setVersionId(const std::string& versionid) {
        getObjectInput_.setVersionId(versionid);
    }
    const std::string& getIfMatch() const {
        return getObjectInput_.getIfMatch();
    }
    void setIfMatch(const std::string& ifmatch) {
        getObjectInput_.setIfMatch(ifmatch);
    }
    time_t getIfModifiedSince() const {
        return getObjectInput_.getIfModifiedSince();
    }
    void setIfModifiedSince(time_t ifmodifiedsince) {
        getObjectInput_.setIfModifiedSince(ifmodifiedsince);
    }
    const std::string& getIfNoneMatch() const {
        return getObjectInput_.getIfNoneMatch();
    }
    void setIfNoneMatch(const std::string& ifnonematch) {
        getObjectInput_.setIfNoneMatch(ifnonematch);
    }
    time_t getIfUnmodifiedSince() const {
        return getObjectInput_.getIfUnmodifiedSince();
    }
    void setIfUnmodifiedSince(time_t ifunmodifiedsince) {
        getObjectInput_.setIfUnmodifiedSince(ifunmodifiedsince);
    }
    const std::string& getSsecAlgorithm() const {
        return getObjectInput_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        getObjectInput_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKey() const {
        return getObjectInput_.getSsecKey();
    }
    void setSsecKey(const std::string& sseckey) {
        getObjectInput_.setSsecKey(sseckey);
    }
    const std::string& getSsecKeyMd5() const {
        return getObjectInput_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        getObjectInput_.setSsecKeyMd5(sseckeymd5);
    }
    const std::string& getResponseCacheControl() const {
        return getObjectInput_.getResponseCacheControl();
    }
    void setResponseCacheControl(const std::string& responsecachecontrol) {
        getObjectInput_.setResponseCacheControl(responsecachecontrol);
    }
    const std::string& getResponseContentDisposition() const {
        return getObjectInput_.getResponseContentDisposition();
    }
    void setResponseContentDisposition(const std::string& responsecontentdisposition) {
        getObjectInput_.setResponseContentDisposition(responsecontentdisposition);
    }
    const std::string& getResponseContentEncoding() const {
        return getObjectInput_.getResponseContentEncoding();
    }
    void setResponseContentEncoding(const std::string& responsecontentencoding) {
        getObjectInput_.setResponseContentEncoding(responsecontentencoding);
    }
    const std::string& getResponseContentLanguage() const {
        return getObjectInput_.getResponseContentLanguage();
    }
    void setResponseContentLanguage(const std::string& responsecontentlanguage) {
        getObjectInput_.setResponseContentLanguage(responsecontentlanguage);
    }
    const std::string& getResponseContentType() const {
        return getObjectInput_.getResponseContentType();
    }
    void setResponseContentType(const std::string& responsecontenttype) {
        getObjectInput_.setResponseContentType(responsecontenttype);
    }
    time_t getResponseExpires() const {
        return getObjectInput_.getResponseExpires();
    }
    void setResponseExpires(time_t responseexpires) {
        getObjectInput_.setResponseExpires(responseexpires);
    }
    int64_t getRangeStart() const {
        return getObjectInput_.getRangeStart();
    }
    void setRangeStart(int64_t rangestart) {
        getObjectInput_.setRangeStart(rangestart);
    }
    int64_t getRangeEnd() const {
        return getObjectInput_.getRangeEnd();
    }
    void setRangeEnd(int64_t rangeend) {
        getObjectInput_.setRangeEnd(rangeend);
    }
    const DataTransferListener& getDataTransferListener() const {
        return getObjectInput_.getDataTransferListener();
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        getObjectInput_.setDataTransferListener(datatransferlistener);
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return getObjectInput_.getRateLimiter();
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        getObjectInput_.setRateLimiter(ratelimiter);
    }

    const std::string& getRange() const {
        return getObjectInput_.getRange();
    }
    void setRange(const std::string& range) {
        getObjectInput_.setRange(range);
    }
    int64_t getTrafficLimit() const {
        return getObjectInput_.getTrafficLimit();
    }
    void setTrafficLimit(int64_t trafficLimit) {
        getObjectInput_.setTrafficLimit(trafficLimit);
    }
    const std::string& getProcess() const {
        return getObjectInput_.getProcess();
    }
    void setProcess(const std::string& process) {
        getObjectInput_.setProcess(process);
    }

private:
    GetObjectV2Input getObjectInput_;
    std::string filePath_;
};
}  // namespace VolcengineTos
