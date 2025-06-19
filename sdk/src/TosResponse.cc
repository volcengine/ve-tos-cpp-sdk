#include "TosResponse.h"
#include "model/RequestInfo.h"
#include "utils/BaseUtils.h"
using namespace VolcengineTos;

RequestInfo TosResponse::GetRequestInfo() {
    RequestInfo info;
    info.setRequestId(getRequestID());
    info.setHeaders(headers_);
    info.setId2(headers_["x-tos-id-2"]);
    info.setStatusCode(statusCode_);
    return info;
}

std::string TosResponse::getRequestID() {
    return MapUtils::findValueByKeyIgnoreCase(headers_, HEADER_REQUEST_ID);
}

std::string TosResponse::getEcCode(){
    return MapUtils::findValueByKeyIgnoreCase(headers_, HEADER_EC_CODE);
}