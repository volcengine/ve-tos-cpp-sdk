#include "model/object/UploadPartOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::UploadPartOutput::dump() {
    json j;
    j["RequestInfo"]["RequestId"] = requestInfo_.getRequestId();
    j["PartNumber"] = partNumber_;
    j["Etag"] = etag_;
    j["SSECustomerAlgorithm"] = sseCustomerAlgorithm_;
    j["SSECustomerMD5"] = sseCustomerMD5_;
    return j.dump();
}

void VolcengineTos::UploadPartOutput::load(const std::string& output) {
    auto j = json::parse(output);
    RequestInfo ri;
    if (j.contains("RequestInfo") && j.at("RequestInfo").contains("RequestId")) {
        requestInfo_.setRequestId(j.at("RequestInfo").at("RequestId"));
    }
    if (j.contains("PartNumber"))
        j.at("PartNumber").get_to(partNumber_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(etag_);
    if (j.contains("SSECustomerAlgorithm"))
        j.at("SSECustomerAlgorithm").get_to(sseCustomerAlgorithm_);
    if (j.contains("SSECustomerMD5"))
        j.at("SSECustomerMD5").get_to(sseCustomerMD5_);
}
