#include "AsyncEngineCore.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace VolcengineTos {
namespace {
using Clock = std::chrono::steady_clock;
int64_t Now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();
}
int64_t WallNow() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
}
thread_local const void* current_engine = nullptr;
void WakeFd(int fd) noexcept {
    const uint64_t one = 1;
    ssize_t n;
    do {
        n = ::write(fd, &one, sizeof(one));
    } while (n < 0 && errno == EINTR);
}
void DrainFd(int fd) noexcept {
    uint64_t n;
    while (::read(fd, &n, sizeof(n)) == sizeof(n)) {
    }
}
std::string ProxyEnv(const char* lower, const char* upper = nullptr) {
    const char* value = std::getenv(lower);
    if ((!value || !*value) && upper)
        value = std::getenv(upper);
    return value ? value : "";
}
struct ProxyEnvironment {
    const std::string http{ProxyEnv("http_proxy")};  // uppercase HTTP_PROXY is intentionally ignored by curl
    const std::string https{ProxyEnv("https_proxy", "HTTPS_PROXY")};
    const std::string all{ProxyEnv("all_proxy", "ALL_PROXY")};
    const std::string bypass{ProxyEnv("no_proxy", "NO_PROXY")};
    const std::string& For(const std::string& scheme) const {
        const auto& specific = scheme == "https" ? https : http;
        return specific.empty() ? all : specific;
    }
};
std::string ProfileKey(const TransportConfig& c, const AsyncClientSharingOptions& o) {
    if (c.getSslCtxCallback() || c.getSslCtxCallbackUserData())
        throw std::invalid_argument("shared engine requires an immutable TLS profile; SSL callbacks are not supported");
    std::ostringstream s;
    auto part = [&](const std::string& v) { s << v.size() << ':' << v; };
    part(o.isolation_domain);
    part(o.tls_generation);
    part(c.getProxyHost());
    part(std::to_string(c.getProxyPort()));
    part(c.getProxyUsername());
    part(c.getProxyPassword());
    part(c.getCaFile());
    part(c.getCaPath());
    part(c.getClientCrt());
    part(c.getClientKey());
    part(c.getNetInterface());
    s << ':' << c.isEnableVerifySsl() << ':' << c.getDnsCacheTime() << ':' << c.getKeepAlive() << ':'
      << c.isConnectionReuse();
    return s.str();  // private registry key; never logged or exported
}
}  // namespace

struct AsyncSharedProfile {
    uint64_t id;
    std::string key;
    TransportConfig config;
    CURLSH* share{nullptr};
    std::mutex mutex;
    AsyncSharedProfile(uint64_t i, std::string k, const TransportConfig& c)
            : id(i), key(std::move(k)), config(c), share(curl_share_init()) {
        if (!share)
            throw std::bad_alloc();
        if (curl_share_setopt(share, CURLSHOPT_LOCKFUNC, &Lock) != CURLSHE_OK ||
            curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, &Unlock) != CURLSHE_OK ||
            curl_share_setopt(share, CURLSHOPT_USERDATA, this) != CURLSHE_OK ||
            curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS) != CURLSHE_OK ||
            curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION) != CURLSHE_OK) {
            curl_share_cleanup(share);
            share = nullptr;
            throw std::runtime_error("cannot configure shared DNS/TLS session cache");
        }
    }
    ~AsyncSharedProfile() {
        if (share)
            curl_share_cleanup(share);
    }
    static void Lock(CURL*, curl_lock_data, curl_lock_access, void* p) {
        static_cast<AsyncSharedProfile*>(p)->mutex.lock();
    }
    static void Unlock(CURL*, curl_lock_data, void* p) {
        static_cast<AsyncSharedProfile*>(p)->mutex.unlock();
    }
};

struct AsyncEngineCore::State {
    using Context = AsyncHttpClient::RequestContext;
    using Multi = AsyncHttpClient::CurlMultiHandle;
    struct Worker;
    struct Group {
        Worker* worker;
        std::shared_ptr<AsyncSharedProfile> profile;
        Multi multi;
        size_t credits{1};
        int64_t curl_due{-1}, idle_since{Now()};
        bool failed{false}, draining{false};
        explicit Group(Worker* w, std::shared_ptr<AsyncSharedProfile> p, size_t handles)
                : worker(w), profile(std::move(p)), multi(profile->config, static_cast<int>(handles)) {
            multi.running_finish = true;  // no private thread; lifetime is worker-owned
            if (!multi.multi_handle)
                throw std::bad_alloc();
        }
    };
    struct Info {
        std::shared_ptr<AsyncClientSession> session;
        Worker* worker;
        uint64_t id;
        size_t reservation;
        Clock::time_point submitted{Clock::now()};
        int64_t deadline{0}, ready_at{0};
        bool accepted{false}, queued_traced{false}, started{false};
    };
    struct Watch {
        int fd;
        Group* group;
    };
    struct Worker {
        State* state;
        size_t index;
        int wake{-1}, epoll{-1};
        std::thread thread;
        std::mutex inbox_mutex;
        std::deque<std::unique_ptr<Context>> inbox;
        std::deque<std::unique_ptr<Context>> pending;
        std::map<uint64_t, std::unique_ptr<Group>> groups;
        std::map<uint64_t, Watch> watches;
        std::map<int, uint64_t> fd_tokens;
        uint64_t next_watch{1};  // zero is eventfd; never reuse an epoll token
        std::atomic<size_t> load{0};
        std::atomic<uint64_t> cpu_ns{0};
        bool failed{false};
        bool waiting_credit{false}, pass_needs_credit{false};
        std::atomic<bool> waiting_capacity{false};
        bool pass_needs_capacity{false};
        explicit Worker(State* s, size_t i) : state(s), index(i) {
            wake = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            epoll = epoll_create1(EPOLL_CLOEXEC);
            if (wake < 0 || epoll < 0) {
                if (wake >= 0)
                    ::close(wake);
                if (epoll >= 0)
                    ::close(epoll);
                throw std::runtime_error("shared engine event loop initialization failed");
            }
            epoll_event ev{};
            ev.events = EPOLLIN;
            ev.data.u64 = 0;
            if (epoll_ctl(epoll, EPOLL_CTL_ADD, wake, &ev) != 0) {
                ::close(wake);
                ::close(epoll);
                throw std::runtime_error("shared engine eventfd registration failed");
            }
        }
        ~Worker() {
            if (wake >= 0)
                ::close(wake);
            if (epoll >= 0)
                ::close(epoll);
        }
        void Notify() noexcept {
            WakeFd(wake);
        }
        static void NotifyFromEvent(void* p) {
            static_cast<Worker*>(p)->Notify();
        }
        static int Socket(CURL*, curl_socket_t fd, int what, void* p, void*) noexcept {
            auto* g = static_cast<Group*>(p);
            auto& w = *g->worker;
            try {
                auto old = w.fd_tokens.find(fd);
                if (what == CURL_POLL_REMOVE) {
                    if (old != w.fd_tokens.end()) {
                        epoll_ctl(w.epoll, EPOLL_CTL_DEL, fd, nullptr);
                        w.watches.erase(old->second);
                        w.fd_tokens.erase(old);
                    }
                    return 0;
                }
                uint64_t token = old == w.fd_tokens.end() ? w.next_watch++ : old->second;
                epoll_event ev{};
                ev.events = (what == CURL_POLL_IN || what == CURL_POLL_INOUT ? EPOLLIN : 0) |
                            (what == CURL_POLL_OUT || what == CURL_POLL_INOUT ? EPOLLOUT : 0);
                ev.data.u64 = token;
                if (epoll_ctl(w.epoll, old == w.fd_tokens.end() ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, fd, &ev) != 0)
                    throw std::runtime_error("epoll socket update failed");
                w.watches[token] = {fd, g};
                w.fd_tokens[fd] = token;
                return 0;
            } catch (...) {
                g->failed = true;
                w.Notify();
                return -1;
            }
        }
        static int Timer(CURLM*, long timeout, void* p) noexcept {
            auto* g = static_cast<Group*>(p);
            g->curl_due = timeout < 0 ? -1 : Now() + timeout;
            return 0;
        }
        void Drive(Group& g, int fd, int action) {
            AsyncHttpClient::updateCurrentTime(g.multi);
            int running = 0;
            if (curl_multi_socket_action(g.multi.multi_handle, fd, action, &running) != CURLM_OK)
                g.failed = true;
            auto tid = std::this_thread::get_id();
            AsyncHttpClient::processCompletedRequests(g.multi, tid, g.multi.curl_cache, g.multi.extra_fds);
            if (g.multi.active_set_.empty())
                g.idle_since = Now();
        }
        void RemoveGroup(std::map<uint64_t, std::unique_ptr<Group>>::iterator it) {
            Group* g = it->second.get();
            const size_t credits = g->credits;
            // curl cleanup invokes socket callbacks. Keep both Group and its
            // owner alive until cleanup finishes; remove any residual watches.
            for (auto w = watches.begin(); w != watches.end();) {
                if (w->second.group != g) {
                    ++w;
                    continue;
                }
                epoll_ctl(epoll, EPOLL_CTL_DEL, w->second.fd, nullptr);
                fd_tokens.erase(w->second.fd);
                w = watches.erase(w);
            }
            curl_multi_setopt(g->multi.multi_handle, CURLMOPT_SOCKETFUNCTION, nullptr);
            curl_multi_setopt(g->multi.multi_handle, CURLMOPT_TIMERFUNCTION, nullptr);
            groups.erase(it);
            state->credits.fetch_sub(credits);
            state->shards.fetch_sub(1);
            state->evictions.fetch_add(1);
            if (state->outstanding.load())
                state->WakeAll();
        }
        bool Credit() {
            // Publish before checking capacity: a different worker can finish
            // its last IO between our failed CAS and the next epoll wait.
            waiting_capacity = true;
            size_t n = state->credits.load();
            while (n < state->options.max_connections) {
                if (state->credits.compare_exchange_weak(n, n + 1))
                    return true;
            }
            pass_needs_capacity = true;
            return false;
        }
        void RequestReclaim() {
            pass_needs_credit = true;
            if (!waiting_credit) {
                waiting_credit = true;
                ++state->waiting_workers;
                state->WakeAll();
            }
        }
        Group* GetGroup(const std::shared_ptr<AsyncSharedProfile>& p) {
            auto it = groups.find(p->id);
            if (it != groups.end())
                return it->second.get();
            if (!Credit()) {
                RequestReclaim();
                return nullptr;
            }
            try {
                auto group = std::make_unique<Group>(this, p, state->options.max_connections);
                auto* g = group.get();
                g->multi.engine_notify = &NotifyFromEvent;
                g->multi.engine_notify_data = this;
                auto check = [](CURLMcode c) {
                    if (c != CURLM_OK)
                        throw std::runtime_error("curl multi option failed");
                };
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1L));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_MAXCONNECTS, 1L));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS,
                                        static_cast<long>(state->options.max_host_connections)));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_SOCKETFUNCTION, &Socket));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_SOCKETDATA, g));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_TIMERFUNCTION, &Timer));
                check(curl_multi_setopt(g->multi.multi_handle, CURLMOPT_TIMERDATA, g));
                groups.emplace(p->id, std::move(group));
                state->shards.fetch_add(1);
                return g;
            } catch (...) {
                state->credits.fetch_sub(1);
                throw;
            }
        }
        void TraceQueued(Context& ctx) {
            auto info = std::static_pointer_cast<Info>(ctx.engine_request);
            if (!info->queued_traced) {
                info->queued_traced = true;
                state->Trace(*info, AsyncEngineTrace::Phase::Queued);
            }
        }
        void Reject(Context& ctx, CURLcode code, const char* message) {
            TraceQueued(ctx);
            ctx.failSetup(code, message);
        }
        void ServicePending() {
            pass_needs_credit = false;
            pass_needs_capacity = false;
            // Each pass visits the current queue once; blocked profiles never
            // spin. Wakeup/timer/credit release is required before retrying.
            for (auto it = pending.begin(); it != pending.end();) {
                const int64_t now = Now();
                auto& ctx = **it;
                auto info = std::static_pointer_cast<Info>(ctx.engine_request);
                TraceQueued(ctx);
                if (state->closing || info->session->closing || (info->deadline && now >= info->deadline)) {
                    const bool timeout = !state->closing && !info->session->closing;
                    Reject(ctx, timeout ? CURLE_OPERATION_TIMEDOUT : CURLE_ABORTED_BY_CALLBACK,
                           timeout ? "request deadline expired in shared queue" : "async client closed");
                    it = pending.erase(it);
                    continue;
                }
                if (now < info->ready_at) {
                    ++it;
                    continue;
                }
                Group* g = nullptr;
                try {
                    g = GetGroup(info->session->profile);
                    if (!g) {
                        ++it;
                        continue;
                    }
                    // Reclamation is sticky per existing shard. A newly
                    // created shard must be allowed to start its first work:
                    // another worker can set global pressure after GetGroup
                    // reserved the last credit. Gating it on that global bit
                    // would park every request with no active IO to wake it.
                    if (g->draining) {
                        // Another owner may already have returned the needed
                        // capacity. Do not wait for this shard's slow active IO
                        // merely because it was previously chosen to drain.
                        if (state->credits < state->options.max_connections) {
                            g->draining = false;
                        } else {
                            RequestReclaim();
                            ++it;
                            continue;
                        }
                    }
                    if (g->multi.active_count >= g->credits) {
                        if (!Credit()) {
                            // An idle incompatible shard may hold all remaining
                            // credits. Ask owners to retire idle shards, but do
                            // not churn connections when every credit is busy.
                            if (state->active < state->credits)
                                RequestReclaim();
                            ++it;
                            continue;
                        }
                        ++g->credits;
                        curl_multi_setopt(g->multi.multi_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS,
                                          static_cast<long>(g->credits));
                        curl_multi_setopt(g->multi.multi_handle, CURLMOPT_MAXCONNECTS, static_cast<long>(g->credits));
                    }
                    ctx.parent = &g->multi;
                    ctx.curl_handle = g->multi.curl_cache.Acquire();
                    if (!ctx.curl_handle)
                        throw std::bad_alloc();
                    g->multi.curl_cache.Configure(ctx.curl_handle, info->session->config, g->profile->share,
                                                  info->deadline ? std::max<int64_t>(1, info->deadline - Now()) : 0,
                                                  state->proxy_environment.For(ctx.request->url().scheme()),
                                                  state->proxy_environment.bypass);
                    AsyncHttpClient::updateCurrentTime(g->multi);
                    info->started = true;
                    state->active.fetch_add(1);
                    state->Trace(*info, AsyncEngineTrace::Phase::Started);
                    if (state->closing || info->session->closing) {
                        Reject(ctx, CURLE_ABORTED_BY_CALLBACK, "async client closed during request start");
                        it = pending.erase(it);
                        continue;
                    }
                    auto code = AsyncHttpClient::addRequestToEventor(g->multi.multi_handle, &ctx);
                    if (code != CURLE_OK) {
                        Reject(ctx, code, "shared request setup failed");
                        it = pending.erase(it);
                        continue;
                    }
                    g->multi.active_set_.insert(&ctx);
                    ++g->multi.active_count;
                } catch (...) {
                    Reject(ctx, CURLE_OUT_OF_MEMORY, "shared request setup allocation failed");
                    it = pending.erase(it);
                    continue;
                }
                // Ownership has transferred to the multi. Drive may complete
                // and delete ctx, so it must not be inside the setup catch.
                it->release();
                it = pending.erase(it);
                Drive(*g, CURL_SOCKET_TIMEOUT, 0);
            }
            if (waiting_credit && !pass_needs_credit) {
                waiting_credit = false;
                --state->waiting_workers;
            }
            waiting_capacity = pass_needs_capacity;
        }
        void ServiceGroups(bool wake) {
            auto tid = std::this_thread::get_id();
            // Prefer an already-idle victim before asking busy groups to
            // drain. Otherwise a busy group can unnecessarily stall its own
            // queue while an idle group on this same worker holds a credit.
            if (state->Pressure()) {
                for (auto it = groups.begin();
                     it != groups.end() && state->credits >= state->options.max_connections;) {
                    if (it->second->multi.active_set_.empty()) {
                        auto remove = it++;
                        RemoveGroup(remove);
                    } else
                        ++it;
                }
            }
            for (auto it = groups.begin(); it != groups.end();) {
                auto& g = *it->second;
                if (state->Pressure() && state->credits >= state->options.max_connections)
                    g.draining = true;
                AsyncHttpClient::updateCurrentTime(g.multi);
                // Only active groups need application deadline/backpressure
                // inspection. Idle groups have no request containers to scan.
                for (auto a = g.multi.active_set_.begin(); a != g.multi.active_set_.end();) {
                    auto* ctx = *a++;
                    auto info = std::static_pointer_cast<Info>(ctx->engine_request);
                    if (g.failed || state->closing || info->session->closing ||
                        (info->deadline && Now() >= info->deadline)) {
                        const auto code =
                                g.failed ? CURLE_RECV_ERROR
                                         : (state->closing || info->session->closing ? CURLE_ABORTED_BY_CALLBACK
                                                                                     : CURLE_OPERATION_TIMEDOUT);
                        AsyncHttpClient::finishRequest(g.multi, ctx, code);
                    }
                }
                const auto pause_due = AsyncHttpClient::getNextIdleWakeupMs(g.multi);
                if (wake || (pause_due >= 0 && pause_due <= WallNow()))
                    AsyncHttpClient::processPausedRequest(g.multi, tid);
                if (g.curl_due >= 0 && Now() >= g.curl_due) {
                    g.curl_due = -1;
                    Drive(g, CURL_SOCKET_TIMEOUT, 0);
                }
                if (g.multi.active_set_.empty() && (state->closing || g.draining || g.failed ||
                                                    Now() - g.idle_since >= state->options.idle_profile_timeout_ms)) {
                    auto remove = it++;
                    RemoveGroup(remove);
                } else
                    ++it;
            }
        }
        int WaitTimeout() {
            int64_t due = LLONG_MAX;
            const int64_t now = Now(), wall = WallNow();
            for (const auto& ctx : pending) {
                auto info = std::static_pointer_cast<Info>(ctx->engine_request);
                if (info->deadline)
                    due = std::min(due, info->deadline);
                if (info->ready_at > now)
                    due = std::min(due, info->ready_at);
            }
            for (const auto& entry : groups) {
                auto& g = *entry.second;
                if (g.curl_due >= 0)
                    due = std::min(due, g.curl_due);
                if (g.multi.active_set_.empty())
                    due = std::min(due, g.idle_since + state->options.idle_profile_timeout_ms);
                for (const auto* ctx : g.multi.active_set_) {
                    auto info = std::static_pointer_cast<Info>(ctx->engine_request);
                    if (info->deadline)
                        due = std::min(due, info->deadline);
                    if (ctx->in_paused_queue.load()) {
                        auto d = AsyncHttpClient::pausedDeadlineMs(*ctx);
                        if (d >= 0)
                            due = std::min(due, now + std::max<int64_t>(0, d - wall));
                    }
                }
            }
            return due == LLONG_MAX ? -1
                                    : static_cast<int>(std::min<int64_t>(INT_MAX, std::max<int64_t>(0, due - now)));
        }
        void Run() noexcept {
            current_engine = state;
            pthread_setname_np(pthread_self(), "tos-shared-io");
            bool notified = true;
            try {
                for (;;) {
                    state->iterations.fetch_add(1);
                    {
                        std::lock_guard<std::mutex> lock(inbox_mutex);
                        while (!inbox.empty()) {
                            pending.push_back(std::move(inbox.front()));
                            inbox.pop_front();
                        }
                    }
                    ServiceGroups(notified);
                    ServicePending();
                    if (state->closing && pending.empty())
                        break;
                    epoll_event events[64];
                    timespec cpu{};
                    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu);
                    cpu_ns = uint64_t(cpu.tv_sec) * 1000000000 + cpu.tv_nsec;
                    int n;
                    do {
                        n = epoll_wait(epoll, events, 64, WaitTimeout());
                    } while (n < 0 && errno == EINTR);
                    if (n < 0)
                        throw std::runtime_error("shared epoll wait failed");
                    notified = false;
                    for (int i = 0; i != n; ++i) {
                        if (events[i].data.u64 == 0) {
                            DrainFd(wake);
                            notified = true;
                            state->wakeups.fetch_add(1);
                            continue;
                        }
                        auto watch = watches.find(events[i].data.u64);
                        if (watch == watches.end())
                            continue;
                        int action = (events[i].events & EPOLLIN ? CURL_CSELECT_IN : 0) |
                                     (events[i].events & EPOLLOUT ? CURL_CSELECT_OUT : 0) |
                                     (events[i].events & (EPOLLERR | EPOLLHUP) ? CURL_CSELECT_ERR : 0);
                        Drive(*watch->second.group, watch->second.fd, action);
                    }
                }
            } catch (...) {
                state->closing = true;
                state->WakeAll();
            }
            // Fail closed on reactor failure. Do not leave accepted requests
            // behind even if allocation or a user callback threw.
            {
                std::lock_guard<std::mutex> lock(state->admission);
                state->closing = true;
            }
            for (;;) {
                std::unique_ptr<Context> ctx;
                {
                    std::lock_guard<std::mutex> lock(inbox_mutex);
                    if (inbox.empty())
                        break;
                    ctx = std::move(inbox.front());
                    inbox.pop_front();
                }
                Reject(*ctx, CURLE_ABORTED_BY_CALLBACK, "shared engine stopped");
            }
            for (auto& ctx : pending)
                Reject(*ctx, CURLE_ABORTED_BY_CALLBACK, "shared engine stopped");
            pending.clear();
            for (auto& entry : groups) {
                auto& g = *entry.second;
                while (!g.multi.active_set_.empty())
                    AsyncHttpClient::finishRequest(g.multi, *g.multi.active_set_.begin(), CURLE_ABORTED_BY_CALLBACK);
            }
            while (!groups.empty())
                RemoveGroup(groups.begin());
            if (waiting_credit) {
                waiting_credit = false;
                --state->waiting_workers;
            }
            waiting_capacity = false;
            timespec cpu{};
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu);
            cpu_ns = uint64_t(cpu.tv_sec) * 1000000000 + cpu.tv_nsec;
            current_engine = nullptr;
        }
    };

    const AsyncEngineOptions options;
    const ProxyEnvironment proxy_environment;  // immutable engine-local routing snapshot
    std::list<std::shared_ptr<AsyncEngineCore>> retirement_node{nullptr};
    bool curl_initialized{false};
    std::mutex admission, join_mutex;
    std::map<std::string, std::weak_ptr<AsyncSharedProfile>> profiles;
    std::vector<std::unique_ptr<Worker>> workers;
    std::atomic<bool> closing{false};
    // Each blocked worker owns its registration. Another worker acquiring a
    // credit must never erase outstanding demand (a lossy global boolean did).
    std::atomic<size_t> waiting_workers{0};
    uint64_t next_client{1}, next_profile{1}, next_request{1};
    std::atomic<size_t> clients{0}, shards{0}, outstanding{0}, active{0}, bytes{0}, credits{0};
    std::atomic<uint64_t> accepted{0}, rejected{0}, results{0}, released{0}, timeouts{0}, cancelled{0};
    std::atomic<uint64_t> new_connections{0}, reused_connections{0}, wakeups{0}, iterations{0}, evictions{0};
    explicit State(AsyncEngineOptions o) : options(std::move(o)) {
    }
    bool Pressure() const noexcept {
        return waiting_workers.load() != 0;
    }
    void WakeAll() noexcept {
        for (auto& w : workers)
            w->Notify();
    }
    void WakeCapacityWaiters() noexcept {
        for (auto& w : workers)
            if (w->waiting_capacity)
                w->Notify();
    }
    void Trace(const Info& i, AsyncEngineTrace::Phase phase, Context* ctx = nullptr) noexcept {
        if (!options.trace && phase != AsyncEngineTrace::Phase::Result)
            return;
        AsyncEngineTrace t;
        t.phase = phase;
        t.request_id = i.id;
        t.client_id = i.session->id;
        t.profile_id = i.session->profile->id;
        t.worker = i.worker->index;
        t.elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - i.submitted).count();
        if (ctx) {
            t.curl_code = ctx->response->getCurlErrCode();
            if (ctx->curl_handle) {
                long created = 0;
                char* ip = nullptr;
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_NUM_CONNECTS, &created);
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_PRIMARY_IP, &ip);
                t.new_connection = created > 0;
                if (created > 0)
                    new_connections.fetch_add(created);
                else if (ip && *ip && t.curl_code == CURLE_OK)
                    reused_connections.fetch_add(1);
                double dns = 0, connect = 0, tls = 0, first = 0;
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_NAMELOOKUP_TIME, &dns);
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_CONNECT_TIME, &connect);
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_APPCONNECT_TIME, &tls);
                curl_easy_getinfo(ctx->curl_handle, CURLINFO_STARTTRANSFER_TIME, &first);
                t.dns_us = dns * 1000000;
                t.connect_us = std::max(0., connect - dns) * 1000000;
                t.tls_us = tls > 0 ? std::max(0., tls - connect) * 1000000 : 0;
                t.first_byte_us = first * 1000000;
            }
        }
        if (options.trace) {
            try {
                options.trace(t);
            } catch (...) {
            }
        }
    }
};

// One sleeping lifecycle thread per SDK instance, not per client or profile.
// Intrusive retirement avoids allocating when the last engine reference is
// released inside a network callback. The joiner is constructed on Create().
class EngineJoiner {
    std::mutex mutex_;
    std::condition_variable cv_;
    std::list<std::shared_ptr<AsyncEngineCore>> queue_;
    bool stop_{false};
    std::thread thread_;

public:
    EngineJoiner()
            : thread_([this] {
                  for (;;) {
                      std::shared_ptr<AsyncEngineCore> core;
                      {
                          std::unique_lock<std::mutex> lock(mutex_);
                          cv_.wait(lock, [&] { return stop_ || !queue_.empty(); });
                          if (queue_.empty())
                              return;
                          core = std::move(queue_.front());
                          queue_.pop_front();
                      }
                      core->Close();
                  }
              }) {
    }
    ~EngineJoiner() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_one();
        thread_.join();
    }
    static EngineJoiner& Get() {
        static EngineJoiner j;
        return j;
    }
    void Enqueue(std::shared_ptr<AsyncEngineCore> c) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto& node = c->state_->retirement_node;
            node.front() = c;
            queue_.splice(queue_.end(), node);
        }
        cv_.notify_one();
    }
};

AsyncEngineCore::AsyncEngineCore(const AsyncEngineOptions& options) : state_(new State(options)) {
}
std::shared_ptr<AsyncEngineCore> AsyncEngineCore::Create(const AsyncEngineOptions& o) {
    if (!o.worker_count || o.worker_count > 256 || !o.max_connections || o.max_connections > INT_MAX ||
        !o.max_host_connections || o.max_host_connections > INT_MAX || !o.max_requests || o.max_requests > INT_MAX ||
        !o.max_requests_per_client || !o.max_profiles || !o.affinity_spill_threshold ||
        o.idle_profile_timeout_ms <= 0 || o.max_request_header_bytes > 16 * 1024 * 1024 ||
        !o.max_response_header_bytes || o.max_response_header_bytes > 16 * 1024 * 1024 ||
        o.max_transport_buffer_bytes < o.max_response_header_bytes + 2 * CURL_MAX_WRITE_SIZE)
        throw std::invalid_argument("invalid async engine limits");
    auto core = std::shared_ptr<AsyncEngineCore>(new AsyncEngineCore(o));
    AsyncHttpClient::initCurl();
    core->state_->curl_initialized = true;
    try {
        (void)EngineJoiner::Get();
        for (size_t i = 0; i != o.worker_count; ++i)
            core->state_->workers.emplace_back(new State::Worker(core->state_.get(), i));
        for (auto& w : core->state_->workers)
            w->thread = std::thread([p = w.get()] { p->Run(); });
    } catch (...) {
        core->BeginClose();
        core->Close();
        throw;
    }
    return core;
}
AsyncEngineCore::~AsyncEngineCore() {
    Close();
    const bool cleanup = state_->curl_initialized;
    state_.reset();
    if (cleanup)
        AsyncHttpClient::cleanupCurl();
}
std::shared_ptr<AsyncClientSession> AsyncEngineCore::Attach(const TransportConfig& c,
                                                            const AsyncClientSharingOptions& o) {
    const auto key = ProfileKey(c, o);
    std::lock_guard<std::mutex> lock(state_->admission);
    if (state_->closing)
        throw std::runtime_error("async engine closed");
    for (auto i = state_->profiles.begin(); i != state_->profiles.end();)
        if (i->second.expired())
            i = state_->profiles.erase(i);
        else
            ++i;
    auto it = state_->profiles.find(key);
    auto profile = it == state_->profiles.end() ? nullptr : it->second.lock();
    if (!profile) {
        if (state_->profiles.size() >= state_->options.max_profiles)
            throw std::runtime_error("async transport profile limit exceeded");
        profile = std::make_shared<AsyncSharedProfile>(state_->next_profile++, key, c);
        state_->profiles[key] = profile;
    }
    auto session = std::make_shared<AsyncClientSession>(c, std::move(profile), state_->next_client++);
    ++state_->clients;
    return session;
}
void AsyncEngineCore::Detach(const std::shared_ptr<AsyncClientSession>&) {
    --state_->clients;
}
void AsyncEngineCore::CloseSession(const std::shared_ptr<AsyncClientSession>& s, bool wait) {
    {
        std::lock_guard<std::mutex> lock(state_->admission);
        s->closing = true;
    }
    state_->WakeAll();
    if (wait && !IsWorker()) {
        std::unique_lock<std::mutex> lock(s->mutex);
        s->drained.wait(lock, [&] { return s->outstanding.load() == 0; });
    }
}

std::future<std::shared_ptr<HttpResponse>> AsyncEngineCore::Submit(
        const std::shared_ptr<AsyncClientSession>& session, const std::shared_ptr<HttpRequest>& request,
        const OnDataReceiveWithEvent& receive, const OnDataSendWithEvent& produce,
        std::function<void(std::shared_ptr<HttpResponse>)> done, OnRequestStart start, OnHttpStatusSet status,
        OnContentLengthSet length, bool need_future) {
    using Context = State::Context;
    if (!request)
        throw std::invalid_argument("null HTTP request");
    auto ctx = std::make_unique<Context>(request);
    auto info = std::make_shared<State::Info>();
    info->session = session;
    info->deadline = session->config.getRequestTimeout() > 0 ? Now() + session->config.getRequestTimeout() : 0;
    info->ready_at = Now() + std::max<int64_t>(0, request->notSendUtilMs() - WallNow());
    size_t header_bytes = request->url().toString().size();
    for (const auto& h : request->Headers()) {
        if (header_bytes > state_->options.max_request_header_bytes)
            break;
        header_bytes += h.first.size() + h.second.size() + 4;
    }
    ctx->url = request->url().toString();
    ctx->on_data_receive_with_event_ = receive;
    ctx->on_data_send_with_event_ = produce;
    ctx->on_request_finished_ = std::move(done);
    ctx->on_request_start_ = std::move(start);
    ctx->on_http_status_set_ = std::move(status);
    ctx->on_content_length_set_ = std::move(length);
    ctx->enable_crc_ = request->isCheckCrc64();
    ctx->is_chunked = request->isChunked();
    ctx->rate_limiter = request->getRateLimiter();
    ctx->event_ = std::make_shared<AsyncEvent>();
    ctx->response_header_limit = state_->options.max_response_header_bytes;
    std::future<std::shared_ptr<HttpResponse>> future;
    if (need_future) {
        ctx->promise.emplace();
        future = ctx->promise->get_future();
    }
    info->reservation = header_bytes + state_->options.max_response_header_bytes + 2 * CURL_MAX_WRITE_SIZE;
    std::unique_lock<std::mutex> admission(state_->admission);
    if (state_->closing || session->closing || header_bytes > state_->options.max_request_header_bytes ||
        state_->outstanding >= state_->options.max_requests ||
        session->outstanding >= state_->options.max_requests_per_client ||
        info->reservation > state_->options.max_transport_buffer_bytes ||
        state_->bytes > state_->options.max_transport_buffer_bytes - info->reservation) {
        ++state_->rejected;
        const bool closed = state_->closing || session->closing;
        admission.unlock();
        ctx->failSetup(closed ? CURLE_ABORTED_BY_CALLBACK : CURLE_AGAIN,
                       closed ? "async client or engine closed" : "async engine admission limit exceeded");
        return future;
    }
    std::string origin = request->url().scheme() + "://" + request->url().host() + ":" + request->url().port();
    size_t index = (std::hash<std::string>{}(origin) ^ session->profile->id) % state_->workers.size();
    auto* worker = state_->workers[index].get();
    if (worker->load >= state_->options.affinity_spill_threshold)
        for (auto& w : state_->workers)
            if (w->load < worker->load)
                worker = w.get();
    info->worker = worker;
    info->id = state_->next_request++;
    ctx->engine_request = info;
    ctx->event_->setNotifier(&State::Worker::NotifyFromEvent, worker);
    ctx->on_result_ = [s = state_.get(), info](Context& c) {
        ++s->results;
        if (info->started)
            --s->active;
        const auto code = c.response->getCurlErrCode();
        if (code == CURLE_OPERATION_TIMEDOUT)
            ++s->timeouts;
        if (code == CURLE_ABORTED_BY_CALLBACK && (s->closing || info->session->closing))
            ++s->cancelled;
        s->Trace(*info, AsyncEngineTrace::Phase::Result, &c);
    };
    ctx->on_retired_ = [s = state_.get(), info] {
        if (info->accepted) {
            s->Trace(*info, AsyncEngineTrace::Phase::Released);
            ++s->released;
        }
        s->bytes.fetch_sub(info->reservation);
        --s->outstanding;
        --info->worker->load;
        {
            std::lock_guard<std::mutex> lock(info->session->mutex);
            --info->session->outstanding;
        }
        info->session->drained.notify_all();
        // An active shard just became a possible idle eviction victim. A
        // waiting registration already owned by another worker does not generate a new
        // event on its own; publish this resource transition explicitly.
        if (s->Pressure())
            s->WakeAll();
        else
            s->WakeCapacityWaiters();
    };
    ++state_->outstanding;
    ++session->outstanding;
    ++worker->load;
    state_->bytes.fetch_add(info->reservation);
    try {
        std::lock_guard<std::mutex> lock(worker->inbox_mutex);
        worker->inbox.push_back(std::move(ctx));
        info->accepted = true;
    } catch (...) {
        // No acceptance: roll back the reservation, without publishing a result.
        ctx->on_result_ = {};
        admission.unlock();
        ctx.reset();
        throw;
    }
    ++state_->accepted;
    admission.unlock();
    worker->Notify();
    return future;
}
void AsyncEngineCore::BeginClose() noexcept {
    {
        std::lock_guard<std::mutex> lock(state_->admission);
        state_->closing = true;
    }
    state_->WakeAll();
}
// A callback on engine A must not synchronously join/drain engine B either:
// callbacks on B could be closing A at the same time.
bool AsyncEngineCore::IsWorker() const noexcept {
    return current_engine != nullptr;
}
bool AsyncEngineCore::Close() {
    BeginClose();
    if (IsWorker())
        return false;
    std::lock_guard<std::mutex> lock(state_->join_mutex);
    for (auto& w : state_->workers)
        if (w->thread.joinable())
            w->thread.join();
    return true;
}
AsyncEngineStats AsyncEngineCore::Stats() const {
    AsyncEngineStats s;
    s.worker_count = state_->workers.size();
    s.clients = state_->clients;
    {
        std::lock_guard<std::mutex> lock(state_->admission);
        for (const auto& p : state_->profiles)
            if (!p.second.expired())
                ++s.profiles;
    }
    s.multi_shards = state_->shards;
    s.outstanding = state_->outstanding;
    s.active = state_->active;
    s.reserved_buffer_bytes = state_->bytes;
    s.connection_credits = state_->credits;
    s.accepted = state_->accepted;
    s.rejected = state_->rejected;
    s.results = state_->results;
    s.released = state_->released;
    s.timeouts = state_->timeouts;
    s.cancelled = state_->cancelled;
    s.new_connections = state_->new_connections;
    s.reused_connections = state_->reused_connections;
    s.wakeups = state_->wakeups;
    s.loop_iterations = state_->iterations;
    s.profile_evictions = state_->evictions;
    for (const auto& w : state_->workers)
        s.worker_cpu_ns += w->cpu_ns;
    return s;
}
void AsyncEngineCore::RetireOnControlThread(std::shared_ptr<AsyncEngineCore> core) noexcept {
    try {
        EngineJoiner::Get().Enqueue(std::move(core));
    } catch (...) {
        std::terminate();
    }
}
AsyncEngine::AsyncEngine(std::shared_ptr<AsyncEngineCore> core) : core_(std::move(core)) {
}
std::shared_ptr<AsyncEngine> AsyncEngine::Create(const AsyncEngineOptions& o) {
    return std::shared_ptr<AsyncEngine>(new AsyncEngine(AsyncEngineCore::Create(o)));
}
AsyncEngine::~AsyncEngine() {
    core_->BeginClose();
    if (core_->IsWorker())
        AsyncEngineCore::RetireOnControlThread(std::move(core_));
    else
        core_->Close();
}
void AsyncEngine::beginClose() noexcept {
    core_->BeginClose();
}
bool AsyncEngine::close() {
    return core_->Close();
}
AsyncEngineStats AsyncEngine::stats() const {
    return core_->Stats();
}
}  // namespace VolcengineTos
