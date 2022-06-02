#pragma once
#include <atomic>
#include <mutex>
#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif
#include "Credentials.h"
#include "FederationToken.h"
#include "FederationTokenProvider.h"
namespace VolcengineTos {
typedef std::chrono::duration<int> secType;
class FederationCredentials : public Credentials {
public:
  FederationCredentials() = delete;
  ~FederationCredentials() override = default;
  FederationCredentials(const FederationCredentials &fc)
      : tokenProvider_(fc.tokenProvider_) {
    cachedToken_ = fc.cachedToken_;
    preFetch_ = fc.preFetch_;
  }
  FederationCredentials& operator=(const FederationCredentials& fc) {
    cachedToken_ = fc.cachedToken_;
    preFetch_ = fc.preFetch_;
    tokenProvider_ = fc.tokenProvider_;
    return *this;
  }
  explicit FederationCredentials(VolcengineTos::FederationTokenProvider &tokenProvider);
  FederationToken token();
  Credential credential() override;

private:
  void updateToken();

  FederationToken cachedToken_;
  std::atomic_flag refreshing_ = ATOMIC_FLAG_INIT;
  secType preFetch_{};
  FederationTokenProvider &tokenProvider_;
  std::mutex update_;
};
}// namespace VolcengineTos

