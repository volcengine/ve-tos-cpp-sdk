#include "model/object/CopyObjectOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::CopyObjectOutput::fromJsonString(const std::string& copy) {
    auto j = json::parse(copy);
    if (j.contains("VersionId"))
        j.at("VersionId").get_to(versionID_);
    if (j.contains("SourceVersionId"))
        j.at("SourceVersionId").get_to(sourceVersionID_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(etag_);
    if (j.contains("LastModified"))
        j.at("LastModified").get_to(lastModified_);
}
