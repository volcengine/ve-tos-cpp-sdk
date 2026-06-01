#pragma once

#include "Type.h"
#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "transport/http/HttpResponse.h"
#include "utils/BaseUtils.h"

#include <string>

namespace VolcengineTos {
class StorageBase : virtual public Headers {
public:
    StorageBase() = default;
    ~StorageBase() override = default;

    const StorageClassType& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const StorageClassType& storageClass) {
        storageClass_ = storageClass;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_STORAGE_CLASS, StorageClassTypetoString[storageClass_]);
    }

    void headers2Output(HttpResponse& response) override {
        storageClass_ =
                StringtoStorageClassType[MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_STORAGE_CLASS)];
    }

private:
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
