#include "TosRequest.h"
#include "auth/SignV4.h"
using namespace VolcengineTos;

Url TosRequest::toUrl() {
  Url url;
  auto iter = queries_.begin();
  for (; iter != queries_.end() ; iter++) {
    url.addQuery(iter->first, iter->second);
  }
  url.setScheme(scheme_);
  url.setHost(host_);
  url.setPath(SignV4::uriEncode(path_, false));
  return url;
}

void TosRequest::resolveContentLength() {
  int64_t currentPos = content_->tellg();
  if (currentPos == static_cast<std::streampos>(-1)) {
    currentPos = 0;
    content_->clear();
  }
  content_->seekg(0, content_->end);
  int64_t size = content_->tellg();
  content_->seekg(currentPos, content_->beg);
  this->setContentLength(size);
}