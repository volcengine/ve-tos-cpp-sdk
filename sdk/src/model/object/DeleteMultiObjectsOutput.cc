#include "model/object/DeleteMultiObjectsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::DeleteMultiObjectsOutput::fromJsonString(const std::string& output) {
    auto j = json::parse(output);
    auto des = j.at("Deleted");
    for (auto& de : des) {
        Deleted deleted;
        std::string tmp;
        if (de.contains("Key"))
            de.at("Key").get_to(tmp);
        deleted.setKey(tmp);
        if (de.contains("VersionId"))
            de.at("VersionId").get_to(tmp);
        deleted.setVersionId(tmp);
        bool deleteMarker = false;
        if (de.contains("DeleteMarker"))
            de.at("DeleteMarker").get_to(deleteMarker);
        deleted.setDeleteMarker(deleteMarker);
        if (de.contains("DeleteMarkerVersionId"))
            de.at("DeleteMarkerVersionId").get_to(tmp);
        deleted.setDeleteMarkerVersionId(tmp);
        deleteds_.emplace_back(deleted);
    }
    auto errs = j.at("Error");
    for (auto& err : errs) {
        DeleteError de;
        std::string tmp;
        if (err.contains("Code"))
            err.at("Code").get_to(tmp);
        de.setCode(tmp);
        if (err.contains("Message"))
            err.at("Message").get_to(tmp);
        de.setMessage(tmp);
        if (err.contains("Key"))
            err.at("Key").get_to(tmp);
        de.setKey(tmp);
        if (err.contains("VersionId"))
            err.at("VersionId").get_to(tmp);
        de.setVersionId(tmp);
        errors_.emplace_back(de);
    }
}
