#pragma once
#include <string>
#include "SourceEndpoint.h"
namespace VolcengineTos {

class PublicSource {
public:
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
