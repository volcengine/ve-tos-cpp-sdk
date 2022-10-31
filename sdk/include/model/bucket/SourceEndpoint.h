#pragma once
#include <string>
#include <utility>
#include <vector>
namespace VolcengineTos {

class SourceEndpoint {
public:
    SourceEndpoint(std::vector<std::string> primary, std::vector<std::string> follower)
            : primary_(std::move(primary)), follower_(std::move(follower)) {
    }
    SourceEndpoint() = default;
    virtual ~SourceEndpoint() = default;
    const std::vector<std::string>& getPrimary() const {
        return primary_;
    }
    void setPrimary(const std::vector<std::string>& primary) {
        primary_ = primary;
    }
    const std::vector<std::string>& getFollower() const {
        return follower_;
    }
    void setFollower(const std::vector<std::string>& follower) {
        follower_ = follower;
    }

private:
    std::vector<std::string> primary_;
    std::vector<std::string> follower_;
};

}  // namespace VolcengineTos
