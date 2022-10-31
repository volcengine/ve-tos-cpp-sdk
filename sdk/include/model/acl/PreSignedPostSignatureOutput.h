#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class PreSignedPostSignatureOutput {
public:
    PreSignedPostSignatureOutput(std::string originPolicy, std::string policy, std::string algorithm,
                                 std::string credential, std::string date, std::string signature)
            : originPolicy_(std::move(originPolicy)),
              policy_(std::move(policy)),
              algorithm_(std::move(algorithm)),
              credential_(std::move(credential)),
              date_(std::move(date)),
              signature_(std::move(signature)) {
    }
    PreSignedPostSignatureOutput() = default;
    ~PreSignedPostSignatureOutput() = default;
    
    const std::string& getOriginPolicy() const {
        return originPolicy_;
    }
    void setOriginPolicy(const std::string& originPolicy) {
        originPolicy_ = originPolicy;
    }
    const std::string& getPolicy() const {
        return policy_;
    }
    void setPolicy(const std::string& policy) {
        policy_ = policy;
    }
    const std::string& getAlgorithm() const {
        return algorithm_;
    }
    void setAlgorithm(const std::string& algorithm) {
        algorithm_ = algorithm;
    }
    const std::string& getCredential() const {
        return credential_;
    }
    void setCredential(const std::string& credential) {
        credential_ = credential;
    }
    const std::string& getDate() const {
        return date_;
    }
    void setDate(const std::string& date) {
        date_ = date;
    }
    const std::string& getSignature() const {
        return signature_;
    }
    void setSignature(const std::string& signature) {
        signature_ = signature;
    }

private:
    std::string originPolicy_;
    std::string policy_;
    std::string algorithm_;
    std::string credential_;
    std::string date_;
    std::string signature_;
};
}  // namespace VolcengineTos
