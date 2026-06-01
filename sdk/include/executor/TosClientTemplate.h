#pragma once
#include "RequestBuilder.h"
#include "TosError.h"
#include "model/RequestInfo.h"
#include "model/async/object/output/GetObjectAsyncOutput.h"
#include "model/object/GetObjectV2Output.h"
#include "transport/http/HttpResponse.h"

namespace VolcengineTos {
int extendMmap(int file_fd, void*& mmap_ptr, size_t& mmap_size, size_t file_offset, size_t need_add_size);
size_t align_to_page(size_t size);

bool checkShouldRetry(const std::string& funcName, int resCode, int curlErrCode, int flowBytes);

std::string getRequestID(const std::map<std::string, std::string>& headers);
std::string getEcCode(const std::map<std::string, std::string>& headers);
std::string getRequestId2(const std::map<std::string, std::string>& headers);

}  // namespace VolcengineTos
