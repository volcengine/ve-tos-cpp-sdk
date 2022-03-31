#include "TosResponse.h"
#include "model/RequestInfo.h"
#include "utils/BaseUtils.h"
using namespace VolcengineTos;

RequestInfo TosResponse::GetRequestInfo() {
  RequestInfo info;
  info.setRequestId(getRequestID());
  info.setHeaders(headers_);
  return info;
}

std::string TosResponse::getRequestID() {
  return MapUtils::findValueByKeyIgnoreCase(headers_, HEADER_REQUEST_ID);
}
