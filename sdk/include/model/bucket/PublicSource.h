#pragma once
#include <string>
#include <utility>
#include "SourceEndpoint.h"
namespace VolcengineTos {

class PublicSource {
public:
    PublicSource() = default;
    virtual ~PublicSource() = default;
    explicit PublicSource(SourceEndpoint sourceEndpoint) : sourceEndpoint_(sourceEndpoint) {
    }
    PublicSource(SourceEndpoint sourceEndpoint, bool fixedEndpoint)
            : sourceEndpoint_(sourceEndpoint), fixedEndpoint_(fixedEndpoint) {
    }
    const SourceEndpoint& getSourceEndpoint() const {
        return sourceEndpoint_;
    }
    void setSourceEndpoint(const SourceEndpoint& sourceEndpoint) {
        sourceEndpoint_ = sourceEndpoint;
    }
    bool isFixedEndpoint() const {
        return fixedEndpoint_;
    }
    void setFixedEndpoint(bool fixedEndpoint) {
        fixedEndpoint_ = fixedEndpoint;
    }

private:
    SourceEndpoint sourceEndpoint_;
    bool fixedEndpoint_{};
};

}  // namespace VolcengineTos
