#pragma once

#include "Credential.h"
namespace VolcengineTos {
class Credentials {
public:
  virtual Credential credential() {
    Credential cred("", "", "");
    return cred;
  }
  virtual ~Credentials() = default;
};
} // namespace VolcengineTos