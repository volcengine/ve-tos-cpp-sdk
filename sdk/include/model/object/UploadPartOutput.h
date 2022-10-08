#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class [[gnu::deprecated]] UploadPartOutput {
public:
    std::string dump();
    void load(const std::string& output);
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partNumber) {
        partNumber_ = partNumber;
    }
    const std::string& getEtag() const {
        return etag_;
    }
    void setEtag(const std::string& etag) {
        etag_ = etag;
    }
    const std::string& getSseCustomerAlgorithm() const {
        return sseCustomerAlgorithm_;
    }
    void setSseCustomerAlgorithm(const std::string& sseCustomerAlgorithm) {
        sseCustomerAlgorithm_ = sseCustomerAlgorithm;
    }
    const std::string& getSseCustomerMd5() const {
        return sseCustomerMD5_;
    }
    void setSseCustomerMd5(const std::string& sseCustomerMd5) {
        sseCustomerMD5_ = sseCustomerMd5;
    }

private:
    RequestInfo requestInfo_;
    int partNumber_ = 0;
    std::string etag_;
    std::string sseCustomerAlgorithm_;
    std::string sseCustomerMD5_;
};
}  // namespace VolcengineTos
