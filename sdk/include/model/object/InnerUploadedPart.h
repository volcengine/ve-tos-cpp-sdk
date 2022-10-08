#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
namespace inner {
class InnerUploadedPart {
public:
    InnerUploadedPart() = default;
    ~InnerUploadedPart() = default;
    InnerUploadedPart(int partNumber, std::string etag) : partNumber_(partNumber), etag_(std::move(etag)) {
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
    int operator<(const InnerUploadedPart& part) const {
        return partNumber_ - part.partNumber_;
    }
    std::string toString() {
        std::string ret("{");
        ret += std::to_string(partNumber_);
        ret += " ";
        ret += etag_;
        ret += "}";
        return ret;
    }

private:
    int partNumber_ = 0;
    std::string etag_;
};
}  // namespace inner
}  // namespace VolcengineTos
