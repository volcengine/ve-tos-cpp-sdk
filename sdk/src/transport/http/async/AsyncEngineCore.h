#pragma once

#include "AsyncEngine.h"
#include "AsyncHttpClient.h"

#include <condition_variable>
#include <mutex>

namespace VolcengineTos {
struct AsyncSharedProfile;
struct AsyncClientSession {
    const TransportConfig config;
    const std::shared_ptr<AsyncSharedProfile> profile;
    const uint64_t id;
    std::atomic<bool> closing{false};
    std::atomic<size_t> outstanding{0};
    std::mutex mutex;
    std::condition_variable drained;
    AsyncClientSession(TransportConfig c, std::shared_ptr<AsyncSharedProfile> p, uint64_t i)
            : config(std::move(c)), profile(std::move(p)), id(i) {
    }
};

class AsyncEngineCore : public std::enable_shared_from_this<AsyncEngineCore> {
public:
    static std::shared_ptr<AsyncEngineCore> Create(const AsyncEngineOptions& options);
    ~AsyncEngineCore();
    std::shared_ptr<AsyncClientSession> Attach(const TransportConfig&, const AsyncClientSharingOptions&);
    void Detach(const std::shared_ptr<AsyncClientSession>&);
    void CloseSession(const std::shared_ptr<AsyncClientSession>&, bool wait);
    std::future<std::shared_ptr<HttpResponse>> Submit(const std::shared_ptr<AsyncClientSession>&,
                                                      const std::shared_ptr<HttpRequest>&,
                                                      const OnDataReceiveWithEvent&, const OnDataSendWithEvent&,
                                                      std::function<void(std::shared_ptr<HttpResponse>)>,
                                                      OnRequestStart, OnHttpStatusSet, OnContentLengthSet,
                                                      bool need_future);
    void BeginClose() noexcept;
    bool Close();
    bool IsWorker() const noexcept;
    AsyncEngineStats Stats() const;
    static void RetireOnControlThread(std::shared_ptr<AsyncEngineCore>) noexcept;

private:
    friend class EngineJoiner;
    struct State;
    explicit AsyncEngineCore(const AsyncEngineOptions&);
    std::unique_ptr<State> state_;
};
}  // namespace VolcengineTos
