
#include "model/bucket/ListedBucket.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListedBucket::fromJsonString(const nlohmann::json &bucket) {
  if (bucket.contains("CreationDate")) bucket.at("CreationDate").get_to(creationDate_);
  if (bucket.contains("Name")) bucket.at("Name").get_to(name_);
  if (bucket.contains("Location")) bucket.at("Location").get_to(location_);
  if (bucket.contains("ExtranetEndpoint")) bucket.at("ExtranetEndpoint").get_to(extranetEndpoint_);
  if (bucket.contains("IntranetEndpoint")) bucket.at("IntranetEndpoint").get_to(intranetEndpoint_);
}
