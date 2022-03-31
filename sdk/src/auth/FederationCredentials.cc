#include <utility>

#include "auth/FederationCredentials.h"

VolcengineTos::FederationCredentials::FederationCredentials(
    VolcengineTos::FederationTokenProvider tokenProvider) {
  cachedToken_ = tokenProvider.federationToken();
  tokenProvider_ = std::move(tokenProvider);
  preFetch_ = secType(300);
}
VolcengineTos::FederationCredentials::~FederationCredentials() = default;

VolcengineTos::FederationToken VolcengineTos::FederationCredentials::token() {
  return cachedToken_;
}

void VolcengineTos::FederationCredentials::updateToken() {
  std::lock_guard<std::mutex> lock(update_);
  cachedToken_ = tokenProvider_.federationToken();
}

VolcengineTos::Credential VolcengineTos::FederationCredentials::credential() {
  std::time_t now = time(nullptr);
  if (now > token().getExpiration()) {
    updateToken();
  } else if (difftime(now, token().getExpiration()) < preFetch_.count()) {
    if (!refreshing_.test_and_set()) {
      cachedToken_ = tokenProvider_.federationToken();
      refreshing_.clear();
    }
  }
  return token().getCredential();
}
