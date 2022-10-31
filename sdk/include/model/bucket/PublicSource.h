#pragma once
#include <string>
#include <utility>
#include "SourceEndpoint.h"
namespace VolcengineTos {

class PublicSource {
public:
    PublicSource() = default;
    virtual ~PublicSource() = default;
    explicit PublicSource(SourceEndpoint sourceEndpoint) : sourceEndpoint_(std::move(sourceEndpoint)) {
    }
    const SourceEndpoint& getSourceEndpoint() const {
        return sourceEndpoint_;
    }
    void setSourceEndpoint(const SourceEndpoint& sourceEndpoint) {
        sourceEndpoint_ = sourceEndpoint;
    }

private:
    SourceEndpoint sourceEndpoint_;
};

}  // namespace VolcengineTos
