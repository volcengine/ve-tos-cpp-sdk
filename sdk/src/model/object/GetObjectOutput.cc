#include "model/object/GetObjectOutput.h"

void VolcengineTos::GetObjectOutput::setObjectMetaFromResponse(TosResponse& response) {
    objectMeta_.fromResponse(response);
}
