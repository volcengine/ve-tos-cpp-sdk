#pragma once

#include <string>
namespace VolcengineTos {
class UploadPartCopyV2Input {
public:
    UploadPartCopyV2Input(std::string bucket, std::string key, std::string srcbucket, std::string srckey,
                          int partnumber, std::string uploadid)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              uploadID_(std::move(uploadid)),
              partNumber_(partnumber),
              srcBucket_(std::move(srcbucket)),
              srcKey_(std::move(srckey)) {
    }
    UploadPartCopyV2Input() = default;
    ~UploadPartCopyV2Input() = default;

    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadid) {
        uploadID_ = uploadid;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partnumber) {
        partNumber_ = partnumber;
    }
    const std::string& getSrcBucket() const {
        return srcBucket_;
    }
    void setSrcBucket(const std::string& srcbucket) {
        srcBucket_ = srcbucket;
    }
    const std::string& getSrcKey() const {
        return srcKey_;
    }
    void setSrcKey(const std::string& srckey) {
        srcKey_ = srckey;
    }
    const std::string& getSrcVersionId() const {
        return srcVersionID_;
    }
    void setSrcVersionId(const std::string& srcversionid) {
        srcVersionID_ = srcversionid;
    }
    int64_t getCopySourceRangeStart() const {
        return copySourceRangeStart_;
    }
    void setCopySourceRangeStart(int64_t copysourcerangestart) {
        copySourceRangeStart_ = copysourcerangestart;
    }
    int64_t getCopySourceRangeEnd() const {
        return copySourceRangeEnd_;
    }
    void setCopySourceRangeEnd(int64_t copysourcerangeend) {
        copySourceRangeEnd_ = copysourcerangeend;
    }
    const std::string& getCopySourceIfMatch() const {
        return copySourceIfMatch_;
    }
    void setCopySourceIfMatch(const std::string& copysourceifmatch) {
        copySourceIfMatch_ = copysourceifmatch;
    }
    time_t getCopySourceIfModifiedSince() const {
        return copySourceIfModifiedSince_;
    }
    void setCopySourceIfModifiedSince(time_t copysourceifmodifiedsince) {
        copySourceIfModifiedSince_ = copysourceifmodifiedsince;
    }
    const std::string& getCopySourceIfNoneMatch() const {
        return copySourceIfNoneMatch_;
    }
    void setCopySourceIfNoneMatch(const std::string& copysourceifnonematch) {
        copySourceIfNoneMatch_ = copysourceifnonematch;
    }
    time_t getCopySourceIfUnmodifiedSince() const {
        return copySourceIfUnmodifiedSince_;
    }
    void setCopySourceIfUnmodifiedSince(time_t copysourceifunmodifiedsince) {
        copySourceIfUnmodifiedSince_ = copysourceifunmodifiedsince;
    }
    const std::string& getCopySourceSsecAlgorithm() const {
        return copySourceSSECAlgorithm_;
    }
    void setCopySourceSsecAlgorithm(const std::string& copysourcessecalgorithm) {
        copySourceSSECAlgorithm_ = copysourcessecalgorithm;
    }
    const std::string& getCopySourceSsecKey() const {
        return copySourceSSECKey_;
    }
    void setCopySourceSsecKey(const std::string& copysourcesseckey) {
        copySourceSSECKey_ = copysourcesseckey;
    }
    const std::string& getCopySourceSsecKeyMd5() const {
        return copySourceSSECKeyMd5_;
    }
    void setCopySourceSsecKeyMd5(const std::string& copysourcesseckeymd5) {
        copySourceSSECKeyMd5_ = copysourcesseckeymd5;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMD5_ = ssecKeyMd5;
    }
    const std::string& getSsecKey() const {
        return ssecKey_;
    }
    void setSsecKey(const std::string& ssecKey) {
        ssecKey_ = ssecKey;
    }
    const std::string& getServerSideEncryption() const {
        return serverSideEncryption_;
    }
    void setServerSideEncryption(const std::string& serverSideEncryption) {
        serverSideEncryption_ = serverSideEncryption;
    }
    const std::string& getCopySourceRange() const {
        return copySourceRange_;
    }
    void setCopySourceRange(const std::string& copySourceRange) {
        copySourceRange_ = copySourceRange;
    }
    int64_t getTrafficLimit() const {
        return trafficLimit_;
    }
    void setTrafficLimit(int64_t trafficLimit) {
        trafficLimit_ = trafficLimit;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    int partNumber_ = 0;

    std::string srcBucket_;
    std::string srcKey_;
    std::string srcVersionID_;
    int64_t copySourceRangeStart_ = 0;
    int64_t copySourceRangeEnd_ = 0;
    std::string copySourceRange_;

    std::string copySourceIfMatch_;
    std::time_t copySourceIfModifiedSince_ = 0;
    std::string copySourceIfNoneMatch_;
    std::time_t copySourceIfUnmodifiedSince_ = 0;

    std::string copySourceSSECAlgorithm_;  // 可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string copySourceSSECKey_;
    std::string copySourceSSECKeyMd5_;

    std::string
            ssecAlgorithm_;  // 客户自定义密钥的加密方式，可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string ssecKeyMD5_;
    std::string ssecKey_;
    std::string serverSideEncryption_;  // TOS 管理密钥的加密方式，可扩展，当前只支持 AES256
    int64_t trafficLimit_ = 0;
};
}  // namespace VolcengineTos
