#pragma once

#include <string>
#include "Type.h"
#include "MirrorHeader.h"
#include "PublicSource.h"
namespace VolcengineTos {
class Redirect {
public:
    const std::string& getRedirectType() const {
        return redirectType_;
    }
    void setRedirectType(const std::string& redirectType) {
        redirectType_ = redirectType;
    }
    bool isFetchSourceOnRedirect() const {
        return fetchSourceOnRedirect_;
    }
    void setFetchSourceOnRedirect(bool fetchSourceOnRedirect) {
        fetchSourceOnRedirect_ = fetchSourceOnRedirect;
    }
    bool isPassQuery() const {
        return passQuery_;
    }
    void setPassQuery(bool passQuery) {
        passQuery_ = passQuery;
    }
    bool isFollowRedirect() const {
        return followRedirect_;
    }
    void setFollowRedirect(bool followRedirect) {
        followRedirect_ = followRedirect;
    }
    const MirrorHeader& getMirrorHeader() const {
        return mirrorHeader_;
    }
    void setMirrorHeader(const MirrorHeader& mirrorHeader) {
        mirrorHeader_ = mirrorHeader;
    }
    const PublicSource& getPublicSource() const {
        return publicSource_;
    }
    void setPublicSource(const PublicSource& publicSource) {
        publicSource_ = publicSource;
    }

private:
    std::string redirectType_;
    bool fetchSourceOnRedirect_;
    bool passQuery_;
    bool followRedirect_;
    MirrorHeader mirrorHeader_;
    PublicSource publicSource_;
};
}  // namespace VolcengineTos
