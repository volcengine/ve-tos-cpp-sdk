#pragma once

#include "TosRequest.h"
#include "auth/SignKeyInfo.h"
#include "auth/Credentials.h"
#include <memory>
#include <chrono>

namespace VolcengineTos {
  class Signer {
    public:
      Signer();
      Signer(const std::shared_ptr<Credentials> &credentials, const std::string &region);
      virtual ~Signer();
      virtual std::map<std::string, std::string> signHeader(const std::shared_ptr<TosRequest> &req);
      virtual std::map<std::string, std::string> signQuery(const std::shared_ptr<TosRequest> &req, std::chrono::duration<int> ttl);
  };
} // namespace VolcengineTos