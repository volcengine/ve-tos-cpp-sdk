#pragma once

#include "model/async/base/PiplineInputInterface.h"

#include <map>
#include <string>

namespace VolcengineTos {
class Queries : virtual public PiplineInputInterface {
public:
    Queries() = default;
    ~Queries() override = default;

    const std::map<std::string, std::string>& addQuery(const std::string& key, const std::string& value) {
        if (!key.empty() && !value.empty()) {
            queries_[key] = value;
        }
        return queries_;
    }

    const std::map<std::string, std::string>& getQueries() const {
        return queries_;
    }
    void setQueries(const std::map<std::string, std::string>& queries) {
        queries_ = queries;
    }

protected:
    const std::map<std::string, std::string>& addQueryWithEmptyValue(const std::string& key) {
        if (!key.empty()) {
            queries_[key] = "";
        }
        return queries_;
    }

private:
    std::map<std::string, std::string> queries_ = {};
};
}  // namespace VolcengineTos
