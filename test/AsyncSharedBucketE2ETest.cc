// Opt-in real-service contracts. Never registered with CTest. Credentials are
// runtime-only; all writes/deletes are confined to this run's unique keys.
#include "AsyncEngine.h"
#include "TosAsyncClient.h"
#include "logger/logger.h"
#include <curl/curl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

using namespace VolcengineTos;
using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;
constexpr size_t MiB = 1024 * 1024;
struct Failure : std::runtime_error {
    using std::runtime_error::runtime_error;
};
void Require(bool ok, const std::string& what) {
    if (!ok)
        throw Failure(what);
}
std::string Env(const char* key, const char* fallback = "") {
    const char* p = std::getenv(key);
    return p && *p ? p : fallback;
}
template <class F>
void Until(F condition, int milliseconds = 15000) {
    const auto end = Clock::now() + Ms(milliseconds);
    while (!condition()) {
        Require(Clock::now() < end, "bounded wait expired");
        std::this_thread::sleep_for(Ms(1));
    }
}
unsigned char Pattern(size_t offset, unsigned seed) {
    return static_cast<unsigned char>(offset * 131 + (offset >> 8) + (offset >> 16) + seed * 17);
}
double ProcessCpu() {
    rusage r{};
    Require(getrusage(RUSAGE_SELF, &r) == 0, "getrusage failed");
    return r.ru_utime.tv_sec + r.ru_utime.tv_usec / 1e6 + r.ru_stime.tv_sec + r.ru_stime.tv_usec / 1e6;
}
long Memory(const char* field) {
    std::ifstream in("/proc/self/status");
    std::string line;
    while (std::getline(in, line))
        if (line.compare(0, std::strlen(field), field) == 0)
            return std::stol(line.substr(std::strlen(field)));
    throw Failure("process memory counter unavailable");
}
size_t Count(const char* path) {
    DIR* d = opendir(path);
    Require(d != nullptr, "process resource directory unavailable");
    size_t count = 0;
    while (auto* p = readdir(d))
        if (p->d_name[0] != '.')
            ++count;
    closedir(d);
    return count;
}
struct Result {
    std::mutex mutex;
    std::condition_variable cv;
    const std::string operation;
    unsigned calls{0};
    bool success{false}, valid{true}, paused_once{false};
    bool closed_before_send{false};
    int status{0}, curl{0};
    size_t bytes{0};
    int64_t content_length{0};
    std::string version, etag, range, request_id;
    std::vector<std::string> keys;
    bool truncated{false};
    std::string next;
    AsyncEvent* event{nullptr};  // guarded by mutex, cleared before callback returns
    Clock::time_point start{Clock::now()}, end{};
    explicit Result(std::string op) : operation(std::move(op)) {
    }
    template <class T>
    void Done(T& outcome) {
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        event = nullptr;
        end = Clock::now();
        if (calls == 1) {
            success = outcome.isSuccess();
            if (success) {
                auto& out = outcome.result();
                if constexpr (std::is_base_of_v<BaseOutput, std::decay_t<decltype(out)>>) {
                    status = out.getStatusCode();
                    content_length = out.getContentLength();
                    request_id = out.getRequestId();
                    version = MapUtils::findValueByKeyIgnoreCase(out.getHeaders(), HEADER_VERSIONID);
                    etag = MapUtils::findValueByKeyIgnoreCase(out.getHeaders(), "ETag");
                    range = MapUtils::findValueByKeyIgnoreCase(out.getHeaders(), "Content-Range");
                } else {
                    // A few public async APIs still use the sync-shaped output.
                    status = out.getRequestInfo().getStatusCode();
                    request_id = out.getRequestInfo().getRequestId();
                }
            } else {
                status = outcome.error().getStatusCode();
                curl = outcome.error().getCurlErrCode();
                closed_before_send = outcome.error().isClientError() &&
                                     outcome.error().getCode() == "UnhandledException" &&
                                     outcome.error().getMessage() == "asynchronous client is closed";
            }
        }
        cv.notify_all();
    }
    void Wait(bool ok = true, int http = 0, int curl_code = -1) {
        std::unique_lock<std::mutex> lock(mutex);
        Require(cv.wait_for(lock, std::chrono::seconds(75), [&] { return calls != 0; }),
                operation + " callback timeout");
        Require(calls == 1 && success == ok && (!http || status == http) && (curl_code < 0 || curl == curl_code),
                operation + " status=" + std::to_string(status) + " curl=" + std::to_string(curl) +
                        " calls=" + std::to_string(calls));
        if (ok)
            Require(!request_id.empty(), operation + " missing service request ID");
    }
    void Data(size_t expected) {
        Wait();
        Require(bytes == expected && valid, operation + " offset-sensitive payload mismatch");
    }
    void Paused() {
        std::unique_lock<std::mutex> lock(mutex);
        Require(cv.wait_for(lock, std::chrono::seconds(10), [&] { return event || calls; }) && event && !calls,
                operation + " did not reach live body pause");
    }
    void Resume() {
        std::lock_guard<std::mutex> lock(mutex);
        if (event)
            event->Resume();
    }
};
struct Trace {
    std::array<std::atomic<unsigned>, 32768> phase{};
    std::atomic<uint64_t> worker_mask{0};
    std::atomic<bool> bad{false};
    std::atomic<size_t> queued{0}, started{0}, results{0}, released{0};
    void On(const AsyncEngineTrace& e) noexcept {
        if (!e.request_id || e.request_id >= phase.size()) {
            bad = true;
            return;
        }
        auto& p = phase[e.request_id];
        unsigned previous = p.load();
        switch (e.phase) {
            case AsyncEngineTrace::Phase::Queued:
                if (previous)
                    bad = true;
                p = 1;
                ++queued;
                break;
            case AsyncEngineTrace::Phase::Started:
                worker_mask.fetch_or(uint64_t(1) << e.worker);
                if (previous != 1)
                    bad = true;
                p = 2;
                ++started;
                break;
            case AsyncEngineTrace::Phase::Result:
                if (previous != 1 && previous != 2)
                    bad = true;
                p = 3;
                ++results;
                break;
            case AsyncEngineTrace::Phase::Released:
                if (previous != 3)
                    bad = true;
                p = 4;
                ++released;
                break;
        }
    }
};
struct Group {
    std::shared_ptr<Trace> trace{std::make_shared<Trace>()};
    std::shared_ptr<AsyncEngine> engine;
    std::vector<std::shared_ptr<TosAsyncClient>> clients;
    void Drain() {
        Until([&] { return engine->stats().outstanding == 0; }, 75000);
        auto s = engine->stats();
        Require(!s.active && !s.reserved_buffer_bytes && s.accepted == s.results && s.results == s.released,
                "engine terminal accounting mismatch");
        Require(!trace->bad && trace->queued == s.accepted && trace->released == s.released &&
                        trace->results == s.results,
                "trace phase order/once mismatch");
    }
    void Close() {
        for (auto& c : clients)
            c->close();
        Require(engine->close(), "control thread did not join engine");
        Drain();
        clients.clear();
        auto s = engine->stats();
        Require(!s.connection_credits && !s.multi_shards && !s.clients, "engine resources not released");
    }
};
struct Object {
    std::string bucket, key;
    size_t size;
    unsigned seed;
    bool may_exist{false};
    std::set<std::string> versions;  // only confirmed nonempty service version IDs
};
struct Multipart {
    std::shared_ptr<Object> object;
    std::string id;
    bool active{true};
};
class Run {
    // Observations outlive SDK callbacks, including every engine's drain.
    std::mutex observed_mutex_;
    std::vector<std::shared_ptr<Result>> observed_;
    size_t verified_callbacks_{0};
    std::vector<std::shared_ptr<Group>> groups_;
    std::vector<std::shared_ptr<Object>> objects_;
    std::vector<std::shared_ptr<Multipart>> multipart_;
    std::shared_ptr<StaticCredentials> credentials_;
    ClientConfig config_;
    std::string region_;
    bool probed_{false};

public:
    std::vector<std::string> buckets;
    std::string prefix;
    explicit Run() {
        Require(Env("FSX_E2E_CONFIRM") == "1", "set FSX_E2E_CONFIRM=1 to authorize bounded test writes");
        const auto profile = Env("FSX_E2E_PROFILE", "external");
        Require(profile == "external" || profile == "ut", "unknown runtime credential profile");
        const bool ut = profile == "ut";
        const auto ak = Env(ut ? "UT_VAR1" : "FSX_EXTERNAL_AK");
        const auto sk = Env(ut ? "UT_VAR2" : "FSX_EXTERNAL_SK");
        Require(!ak.empty() && !sk.empty(), "runtime credential variables are missing");
        buckets.push_back(Env(ut ? "FSX_E2E_UT_BUCKET" : "FSX_EXTERNAL_BUCKET"));
        Require(ut || buckets[0] == "xrh-bj-tosfuse-test", "refusing non-authorized primary bucket");
        const auto second = Env("FSX_E2E_SECOND_BUCKET");
        if (!second.empty() && second != buckets[0])
            buckets.push_back(second);
        for (const auto& bucket : buckets)
            Require(bucket.size() >= 3 && bucket.size() <= 63 &&
                            bucket.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-") == std::string::npos,
                    "invalid explicitly selected test bucket");
        config_.endPoint =
                ut ? Env("FSX_E2E_UT_ENDPOINT") : Env("FSX_EXTERNAL_ENDPOINT", "http://tos-cn-beijing.volces.com");
        // Internal UT endpoints/bucket names are runtime-only, not public test constants.
        // The invoker must select the endpoint documented by the existing UT config.
        const auto host = config_.endPoint.substr(std::min<size_t>(7, config_.endPoint.size()));
        const std::string suffix = ".volces.com";
        const bool ut_endpoint = config_.endPoint.compare(0, 11, "http://tos-") == 0 && host.size() > suffix.size() &&
                                 host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0 &&
                                 host.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-.") == std::string::npos;
        Require(ut ? ut_endpoint
                   : (config_.endPoint == "http://tos-cn-beijing.volces.com" ||
                      config_.endPoint == "http://tos-cn-beijing.ivolces.com"),
                "refusing non-approved HTTP endpoint");
        region_ = ut ? Env("FSX_E2E_UT_REGION") : Env("FSX_EXTERNAL_REGION", "cn-beijing");
        Require(!region_.empty(), "explicit UT region is required");
        config_.connectionTimeout = 5000;
        config_.requestTimeout = 60000;
        config_.maxRetryCount = 0;
        config_.enableCRC = false;  // full offset-sensitive verification is done by this harness
        config_.enableDebug = false;
        config_.detail_log_ = false;
        config_.connection_reuse_ = true;
        credentials_ = std::make_shared<StaticCredentials>(ak, sk);
        std::random_device random;
        prefix = "sdk-shared-e2e/" +
                 std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count()) +
                 "-" + std::to_string(getpid()) + "-" + std::to_string(random()) + "/";
        Logger::getInstance().setAsyncLogLevel(ERROR);
        Logger::getInstance().setMaxQueueSize(0);  // no raw SDK headers/errors in real-credential test logs
        std::cout << "RUN prefix=" << prefix << " buckets=" << buckets.size() << " protocol=HTTP retries=0\n";
    }
    std::shared_ptr<Result> State(const std::string& op) {
        auto r = std::make_shared<Result>(op);
        std::lock_guard<std::mutex> lock(observed_mutex_);
        observed_.push_back(r);
        return r;
    }
    std::shared_ptr<Group> Engine(size_t workers = 1, size_t connections = 32, size_t limit = 4096,
                                  size_t client_limit = 256) {
        auto g = std::make_shared<Group>();
        AsyncEngineOptions o;
        o.worker_count = workers;
        o.max_connections = connections;
        o.max_host_connections = connections;
        o.max_requests = limit;
        o.max_requests_per_client = client_limit;
        o.trace = [t = g->trace](const AsyncEngineTrace& e) { t->On(e); };
        g->engine = AsyncEngine::Create(o);
        groups_.push_back(g);
        return g;
    }
    TosAsyncClient& Client(const std::shared_ptr<Group>& g, int timeout = 60000, bool invalid = false,
                           const std::string& isolation = "") {
        auto config = config_;
        config.requestTimeout = timeout;
        auto credentials = credentials_;
        if (invalid) {
            // Corrupt only a runtime copy: exercise service-side signature
            // rejection without embedding credentials or changing the valid client.
            auto rejected_signing_key = credentials_->getSecretKey();
            Require(!rejected_signing_key.empty(), "runtime signing key is missing");
            rejected_signing_key.front() ^= 1;
            credentials = std::make_shared<StaticCredentials>(
                    credentials_->getAccessKey(), std::move(rejected_signing_key), credentials_->getSecurityToken());
        }
        AsyncClientSharingOptions sharing;
        sharing.isolation_domain = isolation;
        g->clients.push_back(std::make_shared<TosAsyncClient>(region_, credentials, config, g->engine, sharing));
        return *g->clients.back();
    }
    void Probe() {
        auto g = Engine();
        auto& c = Client(g, 5000);
        for (size_t i = 0; i < buckets.size(); ++i) {
            Object missing{buckets[i], prefix + "readonly-probe", 0, 0, false, {}};
            Head(c, missing)->Wait(false, 404);
            Require(List(c, buckets[i], 1).empty(), "probe prefix already exists");
            std::cout << "PASS readonly_probe bucket_index=" << i << " missing_head=404 prefix_list=200\n";
        }
        g->Close();
        probed_ = true;
    }
    std::shared_ptr<Result> Head(TosAsyncClient& c, const Object& o) {
        auto r = State("HEAD");
        c.headObjectAsync(HeadObjectAsyncInput(o.bucket, o.key), [r](auto& out) { r->Done(out); });
        return r;
    }
    std::shared_ptr<Object> New(TosAsyncClient& c, const std::string& suffix, size_t size, unsigned seed,
                                size_t bucket = 0) {
        auto o = std::make_shared<Object>();
        o->bucket = buckets.at(bucket);
        o->key = prefix + suffix;
        o->size = size;
        o->seed = seed;
        for (const auto& old : objects_)
            Require(old->bucket != o->bucket || old->key != o->key, "duplicate owned key");
        Head(c, *o)->Wait(false, 404);  // never overwrite an existing key
        objects_.push_back(o);
        return o;
    }
    std::shared_ptr<Result> Put(TosAsyncClient& c, const std::shared_ptr<Object>& o, bool overwrite = false) {
        auto r = State("PUT");
        PutObjectAsyncInput in(o->bucket, o->key, TransferEncoding::ContentLength);
        in.setContentLength(static_cast<int64_t>(o->size));
        if (!overwrite)
            in.addHeader(HEADER_FORBID_OVERWRITE, "true");
        in.addHeader("x-tos-meta-e2e-seed", std::to_string(o->seed));
        in.addHeader("Expect", "");
        o->may_exist = true;  // includes ambiguous remote completion, even if local callback fails
        c.putObjectAsync(
                in,
                [r, size = o->size, seed = o->seed](char* p, size_t n, AsyncEvent*) {
                    n = std::min(n, size - r->bytes);
                    for (size_t i = 0; i < n; ++i)
                        p[i] = static_cast<char>(Pattern(r->bytes + i, seed));
                    r->bytes += n;
                    return n;
                },
                [r](auto& out) { r->Done(out); });
        return r;
    }
    void Written(TosAsyncClient& c, const std::shared_ptr<Object>& o, const std::shared_ptr<Result>& put) {
        put->Wait();
        Require(put->bytes == o->size, "PUT source length mismatch");
        if (!put->version.empty())
            o->versions.insert(put->version);
        auto h = Head(c, *o);
        h->Wait();
        Require(h->content_length == static_cast<int64_t>(o->size) && !h->etag.empty(), "HEAD size/etag mismatch");
        if (!h->version.empty())
            o->versions.insert(h->version);
    }
    std::shared_ptr<Result> Get(TosAsyncClient& c, const Object& o, const std::string& range = "", size_t offset = 0,
                                int pause = 0, const std::string& version = "") {
        auto r = State("GET");
        GetObjectAsyncInput in(o.bucket, o.key);
        if (!range.empty())
            in.setRange(range);
        if (!version.empty())
            in.setVersionId(version);
        c.getObjectAsync(
                in,
                [r, seed = o.seed, offset, pause](char* p, size_t n, AsyncEvent* event) {
                    const bool first = pause && !r->paused_once;
                    const size_t used = first ? std::min<size_t>(1, n) : n;
                    for (size_t i = 0; i < used; ++i)
                        if (static_cast<unsigned char>(p[i]) != Pattern(offset + r->bytes + i, seed))
                            r->valid = false;
                    r->bytes += used;
                    if (first) {
                        std::lock_guard<std::mutex> lock(r->mutex);
                        r->paused_once = true;
                        if (pause < 0)
                            event->Pause();
                        else
                            event->PauseFor(pause);
                        r->event = event;
                        r->cv.notify_all();
                    }
                    return used;
                },
                [r](auto& out) { r->Done(out); });
        return r;
    }
    std::set<std::string> List(TosAsyncClient& c, const std::string& bucket, int page_size = 2) {
        std::set<std::string> keys, markers;
        std::string marker;
        for (int page = 0; page < 256; ++page) {
            auto r = State("LIST");
            ListObjectsAsyncInput in(bucket);
            in.setPrefix(prefix);
            in.setMaxKeys(page_size);
            in.setMarker(marker);
            c.listObjectsAsync(in, [r](auto& out) {
                if (out.isSuccess()) {
                    for (const auto& item : out.result().getContents())
                        r->keys.push_back(item.getKey());
                    r->truncated = out.result().isTruncated();
                    r->next = out.result().getNextMarker();
                }
                r->Done(out);
            });
            r->Wait();
            for (const auto& key : r->keys)
                Require(key.compare(0, prefix.size(), prefix) == 0 && keys.insert(key).second,
                        "LIST escaped prefix or repeated a key");
            if (!r->truncated)
                return keys;
            Require(!r->next.empty() && markers.insert(r->next).second, "LIST continuation did not advance");
            marker = r->next;
        }
        throw Failure("LIST exceeded bounded page count");
    }
    std::shared_ptr<Multipart> BeginMultipart(TosAsyncClient& c, const std::shared_ptr<Object>& o) {
        auto m = std::make_shared<Multipart>();
        m->object = o;
        multipart_.push_back(m);
        auto r = State("CREATE MULTIPART");
        c.createMultipartUploadAsync(CreateMultipartUploadAsyncInput(o->bucket, o->key), [r, m](auto& out) {
            if (out.isSuccess())
                m->id = out.result().getUploadId();
            r->Done(out);
        });
        r->Wait();
        Require(!m->id.empty(), "multipart creation did not return an upload ID");
        return m;
    }
    std::shared_ptr<Result> Part(TosAsyncClient& c, const std::shared_ptr<Multipart>& m, int number, size_t offset,
                                 size_t bytes) {
        auto r = State("UPLOAD PART");
        UploadPartAsyncInput in(m->object->bucket, m->object->key, number, m->id, TransferEncoding::ContentLength);
        in.setContentLength(bytes);
        c.uploadPartAsync(
                in,
                [r, offset, bytes, seed = m->object->seed](char* p, size_t n, AsyncEvent*) {
                    n = std::min(n, bytes - r->bytes);
                    for (size_t i = 0; i < n; ++i)
                        p[i] = static_cast<char>(Pattern(offset + r->bytes + i, seed));
                    r->bytes += n;
                    return n;
                },
                [r](auto& out) { r->Done(out); });
        return r;
    }
    void Abort(TosAsyncClient& c, const std::shared_ptr<Multipart>& m, bool cleanup = false) {
        Require(!m->id.empty(), "ambiguous multipart initiation needs service-side reconciliation");
        auto r = State("ABORT MULTIPART");
        c.abortMultipartUploadAsync(AbortMultipartUploadInput(m->object->bucket, m->object->key, m->id),
                                    [r](auto& out) { r->Done(out); });
        try {
            r->Wait();
        } catch (const Failure&) {
            if (!cleanup)
                throw;
            r->Wait(false, 404);
        }
        m->active = false;
    }
    void MultipartContracts() {
        auto g = Engine(4, 16);
        auto& a = Client(g);
        auto& b = Client(g);
        auto o = New(a, "multipart-complete", 11 * MiB, 91);
        auto m = BeginMultipart(a, o);
        std::vector<std::shared_ptr<Result>> parts;
        for (int i = 0; i < 3; ++i)
            parts.push_back(Part(i % 2 ? a : b, m, i + 1, i * 5 * MiB, i == 2 ? MiB : 5 * MiB));
        std::vector<UploadedPartV2> manifest;
        for (int i = 0; i < 3; ++i) {
            parts[i]->Wait();
            Require(parts[i]->bytes == (i == 2 ? MiB : 5 * MiB) && !parts[i]->etag.empty(),
                    "part length/ETag mismatch");
            manifest.emplace_back(i + 1, parts[i]->etag);
        }
        CompleteMultipartUploadAsyncInput input(o->bucket, o->key, m->id);
        input.setParts(manifest);
        auto done = State("COMPLETE MULTIPART");
        o->may_exist = true;
        a.completeMultipartUploadAsync(input, [done](auto& out) { done->Done(out); });
        done->Wait();
        m->active = false;
        if (!done->version.empty())
            o->versions.insert(done->version);
        auto h = Head(b, *o);
        h->Wait();
        if (!h->version.empty())
            o->versions.insert(h->version);
        Require(h->content_length == static_cast<int64_t>(o->size), "multipart HEAD size mismatch");
        Get(b, *o)->Data(o->size);
        const auto offset = 5 * MiB - 512;
        Get(a, *o, "bytes=" + std::to_string(offset) + "-" + std::to_string(offset + 1023), offset)->Data(1024);
        auto abandoned = New(a, "multipart-abort", 5 * MiB, 92);
        auto upload = BeginMultipart(b, abandoned);
        Part(a, upload, 1, 0, abandoned->size)->Wait();
        Abort(b, upload);
        Head(a, *abandoned)->Wait(false, 404);
        Part(a, upload, 2, 0, 4096)->Wait(false, 404);  // upload ID no longer accepts parts
        g->Drain();
        std::cout << "PASS multipart parallel_parts=3 cross_part_range=1 aborted_upload_rejected=1\n";
    }
    void Contracts() {
        auto g = Engine(4, 32);
        auto& a = Client(g);
        auto& b = Client(g, 45000);
        std::vector<std::shared_ptr<Object>> data;
        for (size_t bucket = 0; bucket < buckets.size(); ++bucket) {
            for (size_t bytes : {size_t(0), size_t(4096), MiB, 100 * MiB}) {
                auto o = New(a, "dir one/文件-%-" + std::to_string(bytes), bytes, 31 + data.size(), bucket);
                Written(a, o, Put(a, o));
                Get(b, *o)->Data(bytes);
                if (bytes >= 4096) {
                    auto range = Get(b, *o, "bytes=17-1040", 17);
                    range->Data(1024);
                    Require(range->status == 206 && range->range == "bytes 17-1040/" + std::to_string(bytes),
                            "Range response metadata mismatch");
                    Get(a, *o, "bytes=" + std::to_string(bytes - 1024) + "-", bytes - 1024)->Data(1024);
                    Get(a, *o, "bytes=" + std::to_string(bytes) + "-")->Wait(false, 416);
                }
                data.push_back(o);
            }
        }
        auto changed = data.at(2);
        const Object old = *changed;
        const auto old_version = Head(a, old);
        old_version->Wait();
        changed->seed += 19;
        Written(b, changed, Put(b, changed, true));
        Get(a, *changed)->Data(changed->size);
        if (!old_version->version.empty() && old_version->version != "null")
            Get(a, old, "", 0, 0, old_version->version)->Data(old.size);
        auto copied = New(a, "copied", data[1]->size, data[1]->seed);
        CopyObjectAsyncInput in(copied->bucket, copied->key);
        in.setSourceBucket(data[1]->bucket);
        in.setSourceKey(data[1]->key);
        auto result = State("COPY");
        copied->may_exist = true;
        b.copyObjectAsync(in, [result](auto& out) { result->Done(out); });
        result->Wait();
        if (!result->version.empty())
            copied->versions.insert(result->version);
        Get(a, *copied)->Data(copied->size);
        for (const auto& bucket : buckets) {
            std::set<std::string> expected;
            for (const auto& o : objects_)
                if (o->bucket == bucket)
                    expected.insert(o->key);
            Require(List(b, bucket) == expected, "paginated LIST differs from owned keys");
        }
        auto& invalid = Client(g, 5000, true);
        Head(invalid, *data[1])->Wait(false, 403, CURLE_OK);  // Service rejection, not local setup/network failure.
        Get(b, *data[1])->Data(data[1]->size);  // invalid client must not poison another client's signing/connection
        std::cout << "PASS service_auth_rejection http=403 curl=0 valid_client_recovered=1\n";
        g->Drain();
        const auto before = g->engine->stats();
        for (int i = 0; i < 32; ++i)
            Get(i % 2 ? a : b, *data[1])->Data(data[1]->size);
        g->Drain();
        const auto after = g->engine->stats();
        Require(after.reused_connections > before.reused_connections, "no observed cross-client keep-alive reuse");
        std::cout << "PASS contracts objects=" << objects_.size()
                  << " reused_delta=" << after.reused_connections - before.reused_connections
                  << " new_connections_delta=" << after.new_connections - before.new_connections << "\n";
    }
    void Failures() {
        auto setup = Engine();
        auto& seed = Client(setup);
        auto o = New(seed, "fault-source", MiB, 77);
        Written(seed, o, Put(seed, o));
        auto g = Engine(2, 1);
        auto& long_client = Client(g, 15000);
        auto& short_client = Client(g, 200);
        auto hold = Get(long_client, *o, "", 0, -1);
        hold->Paused();
        const auto starts = g->trace->started.load();
        auto queued = Get(short_client, *o);
        queued->Wait(false, 0, CURLE_OPERATION_TIMEDOUT);
        Require(queued->end - queued->start < Ms(1500) && g->trace->started == starts,
                "queued deadline was delayed or sent to service");
        long_client.beginClose();
        long_client.close();
        hold->Wait(false, 0, CURLE_ABORTED_BY_CALLBACK);
        g->Drain();
        auto& survivor = Client(g);
        Get(survivor, *o)->Data(o->size);
        Get(long_client, *o)->Wait(false);
        std::cout << "PASS queue_deadline elapsed_ms="
                  << std::chrono::duration_cast<Ms>(queued->end - queued->start).count() << " no_wire_start=1\n";

        auto shared = Engine(4, 8);
        auto& a = Client(shared, 15000);
        auto& b = Client(shared);
        auto paused = Get(a, *o, "", 0, -1);
        paused->Paused();
        Get(b, *o)->Data(o->size);
        const auto closing = Clock::now();
        a.beginClose();
        a.close();
        paused->Wait(false, 0, CURLE_ABORTED_BY_CALLBACK);
        Require(Clock::now() - closing < Ms(1500), "single client close was not prompt");
        Get(b, *o)->Data(o->size);
        // Callback reentry closes one session and starts another real GET.
        auto& reenter = Client(shared);
        auto outer = State("reentrant HEAD"), inner = State("reentrant GET");
        reenter.headObjectAsync(HeadObjectAsyncInput(o->bucket, o->key), [&, outer, inner, o](auto& out) {
            reenter.close();
            b.getObjectAsync(
                    GetObjectAsyncInput(o->bucket, o->key),
                    [inner](char*, size_t n, AsyncEvent*) {
                        inner->bytes += n;
                        return n;
                    },
                    [inner](auto& value) { inner->Done(value); });
            outer->Done(out);
        });
        outer->Wait();
        inner->Data(o->size);
        for (int i = 0; i < 16; ++i) {
            auto& c = Client(shared, 15000);
            auto r = Get(c, *o, "", 0, -1);
            r->Paused();
            std::thread close([&] { c.close(); });
            r->Resume();
            close.join();
            // Either terminal result is valid when completion races close.
            bool ok;
            {
                std::unique_lock<std::mutex> lock(r->mutex);
                r->cv.wait(lock, [&] { return r->calls != 0; });
                ok = r->success;
            }
            r->Wait(ok, 0, ok ? -1 : CURLE_ABORTED_BY_CALLBACK);
            if (ok)
                r->Data(o->size);
        }
        shared->Drain();
        Get(b, *o)->Data(o->size);
        std::cout << "PASS independent_close reentry resume_close_races=16\n";

        auto limits = Engine(2, 2, 2, 1);
        auto& x = Client(limits, 15000);
        auto& y = Client(limits, 15000);
        auto& z = Client(limits);
        auto rx = Get(x, *o, "", 0, -1);
        rx->Paused();
        Get(x, *o)->Wait(false, 0, CURLE_AGAIN);
        auto ry = Get(y, *o, "", 0, -1);
        ry->Paused();
        Get(z, *o)->Wait(false, 0, CURLE_AGAIN);
        x.beginClose();
        y.beginClose();
        x.close();
        y.close();
        rx->Wait(false);
        ry->Wait(false);
        limits->Drain();
        Get(z, *o)->Data(o->size);
        std::cout << "PASS global_and_client_admission recovered=1\n";
    }
    void CrossBucket(bool cold_serial = false) {
        Require(buckets.size() == 2, "cross-bucket suite requires two explicitly selected buckets");
        auto setup = Engine();
        auto& seed = Client(setup);
        std::array<std::shared_ptr<Object>, 2> data;
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = New(seed, "cross-bucket-" + std::to_string(i), MiB, 101 + i, i);
            Written(seed, data[i], Put(seed, data[i]));
        }
        setup->Close();

        // Both virtual-host origins occupy one worker/profile CURLM. This is
        // shared pooling, NOT a claim that different hosts share one TCP socket.
        auto pooled = Engine(1, 2);
        auto& a = Client(pooled);
        auto& b = Client(pooled, 45000);
        if (cold_serial) {
            Get(a, *data[0])->Data(MiB);
            Get(b, *data[1])->Data(MiB);
        } else {
            // Exercise two live transfers to reserve two connection credits.
            // The separate cold-serial regression must NOT use this warmup:
            // otherwise it would hide cross-origin cache churn at concurrency 1.
            auto warming = Get(a, *data[0], "", 0, -1);
            warming->Paused();
            Get(b, *data[1])->Data(MiB);
            warming->Resume();
            warming->Data(MiB);
        }
        pooled->Drain();
        const auto before = pooled->engine->stats();
        for (int i = 0; i < 32; ++i)
            Get(i % 2 ? a : b, *data[i % 2])->Data(MiB);
        pooled->Drain();
        const auto after = pooled->engine->stats();
        std::cout << "POOL cold_serial=" << cold_serial << " workers=" << after.worker_count
                  << " clients=" << after.clients << " profiles=" << after.profiles
                  << " multi_shards=" << after.multi_shards << " credits=" << after.connection_credits
                  << " reused_delta=" << after.reused_connections - before.reused_connections
                  << " new_connections_delta=" << after.new_connections - before.new_connections << '\n';
        Require(after.worker_count == 1 && after.clients == 2 && after.profiles == 1 && after.multi_shards == 1 &&
                        after.connection_credits <= 2,
                "cross-bucket clients did not share one worker/profile pool");
        Require(after.reused_connections > before.reused_connections,
                cold_serial ? "cross-bucket cold serial keep-alive churn despite spare global capacity"
                            : "cross-bucket warm pool did not reuse connections");
        auto paused = Get(a, *data[0], "", 0, -1);
        paused->Paused();
        Get(b, *data[1])->Data(MiB);
        a.close();
        paused->Wait(false, 0, CURLE_ABORTED_BY_CALLBACK);
        Get(b, *data[1])->Data(MiB);
        pooled->Drain();
        std::cout << "PASS cross_bucket_pool workers=1 profiles=1 multi_shards=1 independent_close=1 reused_delta="
                  << after.reused_connections - before.reused_connections
                  << " new_connections_delta=" << after.new_connections - before.new_connections << '\n';
        pooled->Close();

        auto limited = Engine(2, 1);
        auto& owner = Client(limited, 15000);
        auto& deadline = Client(limited, 200);
        auto held = Get(owner, *data[0], "", 0, -1);
        held->Paused();
        const auto started = limited->trace->started.load();
        auto queued = Get(deadline, *data[1]);
        queued->Wait(false, 0, CURLE_OPERATION_TIMEDOUT);
        Require(queued->end - queued->start < Ms(1500) && limited->trace->started == started &&
                        limited->engine->stats().connection_credits == 1,
                "cross-bucket queued deadline bypassed global limit or returned late");
        owner.close();
        held->Wait(false, 0, CURLE_ABORTED_BY_CALLBACK);
        limited->Drain();
        auto& survivor = Client(limited);
        Get(survivor, *data[1])->Data(MiB);
        limited->Drain();
        std::cout << "PASS cross_bucket_global_limit=1 queue_deadline_ms="
                  << std::chrono::duration_cast<Ms>(queued->end - queued->start).count()
                  << " no_wire_start=1 capacity_recovered=1\n";
    }
    std::vector<std::shared_ptr<Object>> ReliabilityData(TosAsyncClient& c, size_t count, size_t bytes) {
        std::vector<std::shared_ptr<Object>> data;
        for (size_t i = 0; i < count; ++i) {
            auto o = New(c, "reliability-" + std::to_string(i), bytes, 131 + i, i % buckets.size());
            Written(c, o, Put(c, o));
            data.push_back(o);
        }
        return data;
    }
    void ReadOrCancelled(const std::shared_ptr<Result>& r, size_t bytes) {
        bool ok, closed_before_send;
        {
            std::unique_lock<std::mutex> lock(r->mutex);
            Require(r->cv.wait_for(lock, std::chrono::seconds(75), [&] { return r->calls != 0; }),
                    "close-race callback timeout");
            ok = r->success;
            closed_before_send = r->closed_before_send;
        }
        if (ok)
            r->Data(bytes);
        else if (closed_before_send) {
            // The public pipeline can reject before transport admission.
            // Match this precise local close error, not arbitrary exceptions.
            r->Wait(false, -1, 0);
            Require(r->bytes == 0, "pre-transport close error delivered body data");
        } else
            r->Wait(false, 0, CURLE_ABORTED_BY_CALLBACK);
    }
    void ReliabilityMixed(size_t rounds) {
        Require(rounds >= 1 && rounds <= 256, "mixed reliability round budget exceeded");
        auto g = Engine(4, 16);
        for (size_t i = 0; i < 32; ++i)
            Client(g);
        auto data = ReliabilityData(*g->clients[0], 4, MiB);
        const auto start = Clock::now();
        const double cpu = ProcessCpu();
        for (size_t round = 0; round < rounds; ++round) {
            std::vector<std::shared_ptr<Result>> puts, snapshots, ranges;
            for (size_t i = 0; i < data.size(); ++i) {
                auto& o = data[i];
                const unsigned old_seed = o->seed, new_seed = old_seed + 1;
                o->seed = new_seed;
                puts.push_back(Put(*g->clients[(round + i) % 32], o, true));
                auto r = State("GET atomic snapshot");
                // Select old/new by the first byte, then require that same
                // generation for the whole response (never accept torn bodies).
                auto selected = std::make_shared<int>(-1);
                g->clients[(round + i + 7) % 32]->getObjectAsync(
                        GetObjectAsyncInput(o->bucket, o->key),
                        [r, selected, old_seed, new_seed](char* p, size_t n, AsyncEvent*) {
                            if (n && *selected < 0) {
                                const auto first = static_cast<unsigned char>(p[0]);
                                *selected = first == Pattern(0, old_seed) ? 0 : 1;
                            }
                            const auto seed = *selected == 0 ? old_seed : new_seed;
                            for (size_t j = 0; j < n; ++j)
                                if (static_cast<unsigned char>(p[j]) != Pattern(r->bytes + j, seed))
                                    r->valid = false;
                            r->bytes += n;
                            return n;
                        },
                        [r](auto& out) { r->Done(out); });
                snapshots.push_back(r);
            }
            for (size_t i = 0; i < data.size(); ++i) {
                Written(*g->clients[(round + i + 11) % 32], data[i], puts[i]);
                snapshots[i]->Data(MiB);
                const size_t offset = (round * 7919 + i * 101) % (MiB - 4096);
                ranges.push_back(Get(*g->clients[(round + i + 19) % 32], *data[i],
                                     "bytes=" + std::to_string(offset) + "-" + std::to_string(offset + 4095), offset));
            }
            for (auto& r : ranges)
                r->Data(4096);
            g->Drain();
            Require(g->engine->stats().connection_credits <= 16, "mixed workload exceeded global credits");
            if ((round + 1) % 32 == 0)
                std::cout << "PROGRESS mixed rounds=" << round + 1 << " rss_kib=" << Memory("VmRSS:") << '\n';
        }
        for (size_t i = 0; i < data.size(); ++i)
            Get(*g->clients[(i + 1) % 32], *data[i])->Data(MiB);
        g->Drain();
        const double wall = std::chrono::duration<double>(Clock::now() - start).count();
        const double used_cpu = ProcessCpu() - cpu;
        std::cout << "PASS reliability_mixed rounds=" << rounds << " atomic_snapshot_reads=" << rounds * 4
                  << " range_reads=" << rounds * 4 << " wall_s=" << wall << " process_cpu_s=" << used_cpu
                  << " avg_process_cores=" << used_cpu / wall << " peak_rss_kib=" << Memory("VmHWM:") << '\n';
    }
    void VerifyCallbacks() {
        // Only called after all engines have joined and all producer threads
        // have joined. Forget observations between lifecycle rounds so test
        // bookkeeping is not mistaken for an SDK leak.
        std::lock_guard<std::mutex> observations(observed_mutex_);
        for (auto& r : observed_) {
            std::lock_guard<std::mutex> lock(r->mutex);
            Require(r->calls == 1, "duplicate/missing callback after full engine drain: " + r->operation);
        }
        verified_callbacks_ += observed_.size();
        observed_.clear();
    }
    void ReliabilityLifecycle(size_t rounds) {
        Require(rounds >= 1 && rounds <= 128, "lifecycle reliability round budget exceeded");
        auto setup = Engine();
        auto data = ReliabilityData(Client(setup), 2, MiB);
        CloseAll();
        VerifyCallbacks();
        groups_.clear();
        setup.reset();
        const size_t base_threads = Count("/proc/self/task"), base_fds = Count("/proc/self/fd");
        const long initial_rss = Memory("VmRSS:");
        for (size_t round = 0; round < rounds; ++round) {
            auto g = Engine(round % 2 ? 4 : 1, round % 3 ? 4 : 1);
            for (int i = 0; i < 8; ++i)
                Client(g, 15000, false, i % 2 ? "lifecycle-isolated" : "");
            // One live read and queued reads exist before simultaneous closes.
            auto held = Get(*g->clients[0], *data[0], "", 0, -1);
            held->Paused();
            std::atomic<bool> start{false};
            std::atomic<size_t> submitted{0};
            std::mutex errors_mutex;
            std::exception_ptr error;
            std::vector<std::thread> threads;
            threads.reserve(6);  // Allocate before any joinable thread exists.
            auto guarded = [&](auto work) {
                return std::thread([&, work] {
                    try {
                        Until([&] { return start.load(); });
                        work();
                    } catch (...) {
                        std::lock_guard<std::mutex> lock(errors_mutex);
                        if (!error)
                            error = std::current_exception();
                    }
                });
            };
            try {
                for (size_t t = 0; t < 4; ++t)
                    threads.push_back(guarded([&, t] {
                        std::vector<std::shared_ptr<Result>> reads;
                        for (size_t i = 0; i < 16; ++i) {
                            reads.push_back(Get(*g->clients[t + 1], *data[(i + t) % 2]));
                            ++submitted;
                        }
                        for (auto& r : reads)
                            ReadOrCancelled(r, MiB);
                    }));
                threads.push_back(guarded([&] {
                    Until([&] { return submitted >= 4; });
                    g->clients[1]->close();
                    g->engine->beginClose();
                    Require(g->engine->close(), "external closer did not join engine");
                }));
                threads.push_back(guarded([&] {
                    Until([&] { return submitted >= 4; });
                    held->Resume();
                    g->engine->beginClose();
                    Require(g->engine->close(), "second external closer did not join engine");
                }));
            } catch (...) {
                // Thread creation can fail part way through the batch. Release
                // both the start barrier and live IO before joining producers.
                g->engine->beginClose();
                start = true;
                for (auto& thread : threads)
                    if (thread.joinable())
                        thread.join();
                throw;
            }
            start = true;
            for (auto& thread : threads)
                thread.join();
            if (error)
                std::rethrow_exception(error);
            ReadOrCancelled(held, MiB);
            auto late = Get(*g->clients.back(), *data[1]);
            ReadOrCancelled(late, MiB);
            Require(!late->success, "submission after engine close succeeded");
            CloseAll();
            VerifyCallbacks();
            groups_.clear();
            g.reset();
            Until([&] { return Count("/proc/self/task") == base_threads && Count("/proc/self/fd") == base_fds; });
            if ((round + 1) % 16 == 0)
                std::cout << "PROGRESS lifecycle rounds=" << round + 1 << " rss_kib=" << Memory("VmRSS:")
                          << " threads=" << base_threads << " fds=" << base_fds << '\n';
        }
        std::cout << "PASS reliability_lifecycle rounds=" << rounds << " competing_submitters=4 competing_closers=2"
                  << " fd_thread_baseline_restored=1 rss_delta_kib=" << Memory("VmRSS:") - initial_rss << '\n';
    }
    void ReliabilityFaults(size_t rounds) {
        Require(rounds >= 1 && rounds <= 64, "fault reliability round budget exceeded");
        auto setup = Engine();
        auto data = ReliabilityData(Client(setup), 2, MiB);
        setup->Close();
        auto g = Engine(4, 1);
        auto& healthy = Client(g);
        auto& holder = Client(g, 15000);
        auto& deadline = Client(g, 250);
        for (size_t round = 0; round < rounds; ++round) {
            auto active_timeout = Get(deadline, *data[round % 2], "", 0, -1);
            active_timeout->Paused();
            active_timeout->Wait(false, 0, CURLE_OPERATION_TIMEDOUT);
            Require(active_timeout->end - active_timeout->start < Ms(1500), "paused read timeout returned late");
            active_timeout->Resume();  // terminal result cleared the event, must be a harmless no-op
            g->Drain();
            Get(healthy, *data[(round + 1) % 2])->Data(MiB);
            auto held = Get(holder, *data[round % 2], "", 0, -1);
            held->Paused();
            const auto starts = g->trace->started.load();
            std::vector<std::shared_ptr<Result>> queue;
            for (int i = 0; i < 32; ++i)
                queue.push_back(Get(deadline, *data[(round + 1) % 2]));
            for (auto& r : queue) {
                r->Wait(false, 0, CURLE_OPERATION_TIMEDOUT);
                Require(r->end - r->start < Ms(1500), "timeout storm returned late");
            }
            Require(g->trace->started == starts, "queued timeout storm escaped onto the wire");
            held->Resume();
            held->Data(MiB);
            g->Drain();
            auto injected = State("GET throwing receive");
            healthy.getObjectAsync(
                    GetObjectAsyncInput(data[0]->bucket, data[0]->key),
                    [round](char*, size_t, AsyncEvent*) -> size_t {
                        if (round % 2)
                            throw 17;
                        throw std::runtime_error("intentional receive callback exception");
                    },
                    [injected](auto& out) { injected->Done(out); });
            injected->Wait(false, 0, CURLE_WRITE_ERROR);
            g->Drain();
            Get(healthy, *data[1])->Data(MiB);
            if ((round + 1) % 8 == 0)
                std::cout << "PROGRESS faults rounds=" << round + 1 << " timeouts=" << g->engine->stats().timeouts
                          << '\n';
        }
        g->Drain();
        const auto before = g->engine->stats();
        const double cpu = ProcessCpu();
        std::this_thread::sleep_for(Ms(500));
        const auto after = g->engine->stats();
        Require(after.loop_iterations - before.loop_iterations < 20, "post-timeout storm idle spin");
        std::cout << "PASS reliability_faults rounds=" << rounds << " queued_timeouts=" << rounds * 32
                  << " active_timeouts=" << rounds << " receive_exceptions=" << rounds
                  << " idle_loops=" << after.loop_iterations - before.loop_iterations
                  << " idle_process_cpu_ms=" << (ProcessCpu() - cpu) * 1000 << '\n';
    }
    void Load(size_t workers, size_t clients, size_t window, size_t operations, size_t bytes) {
        Require(workers >= 1 && workers <= 8 && clients >= 2 && clients <= 1000 && window >= 1 && window <= 128 &&
                        operations >= window && operations <= 4096 && bytes >= 4096 && bytes <= 100 * MiB &&
                        operations <= (size_t(4) * 1024 * MiB) / bytes,
                "load exceeds bounded test budget");
        auto g = Engine(workers, window);
        const auto threads = Count("/proc/self/task"), fds = Count("/proc/self/fd");
        const auto rss = Memory("VmRSS:");
        for (size_t i = 0; i < clients; ++i)
            Client(g);
        Require(Count("/proc/self/task") == threads && Count("/proc/self/fd") == fds,
                "client creation allocated private threads/fds");
        std::cout << "DENSITY clients=" << clients << " client_rss_delta_kib=" << Memory("VmRSS:") - rss
                  << " new_threads=0 new_fds=0\n";
        const size_t count = std::min({clients, size_t(16), size_t(256) * MiB / bytes});
        std::vector<std::shared_ptr<Object>> data;
        std::vector<std::shared_ptr<Result>> puts;
        for (size_t i = 0; i < count; ++i)
            data.push_back(New(*g->clients[i], "load-" + std::to_string(i), bytes, i + 1, i % buckets.size()));
        const auto put_start = Clock::now();
        const double put_cpu = ProcessCpu();
        for (size_t i = 0; i < count; ++i)
            puts.push_back(Put(*g->clients[i], data[i]));
        for (const auto& p : puts)
            p->Wait();
        Metrics("PUT", count, bytes, put_start, put_cpu, g);
        for (size_t i = 0; i < count; ++i) {
            Written(*g->clients[i], data[i], puts[i]);
            Get(*g->clients[(i + 1) % clients], *data[i])->Data(bytes);
        }
        Get(*g->clients.back(), *data.front())->Data(bytes);
        g->Drain();
        const auto start = Clock::now();
        const double cpu = ProcessCpu();
        const auto baseline = g->engine->stats();
        g->trace->worker_mask = 0;
        for (size_t i = 0; i < operations; i += window) {
            std::vector<std::shared_ptr<Result>> wave;
            for (size_t j = i; j < std::min(i + window, operations); ++j)
                wave.push_back(Get(*g->clients[j % clients], *data[j % count]));
            for (auto& r : wave)
                r->Data(bytes);
            Require(g->engine->stats().connection_credits <= window, "global connection budget exceeded");
        }
        Metrics("GET", operations, bytes, start, cpu, g);
        g->Drain();
        const auto end = g->engine->stats();
        std::this_thread::sleep_for(Ms(300));
        const auto idle = g->engine->stats();
        const double idle_cpu = ProcessCpu();
        std::this_thread::sleep_for(Ms(500));
        const auto rested = g->engine->stats();
        Require(rested.loop_iterations - idle.loop_iterations < 20, "idle engine appears to spin");
        std::cout << "IDLE wall_ms=500 process_cpu_ms=" << (ProcessCpu() - idle_cpu) * 1000
                  << " loops=" << rested.loop_iterations - idle.loop_iterations
                  << " worker_cpu_ms=" << (rested.worker_cpu_ns - idle.worker_cpu_ns) / 1e6
                  << " new_connections=" << end.new_connections - baseline.new_connections
                  << " reused_connections=" << end.reused_connections - baseline.reused_connections << '\n';
    }
    void Metrics(const char* stage, size_t operations, size_t bytes, Clock::time_point start, double cpu,
                 const std::shared_ptr<Group>& g) {
        const double wall = std::chrono::duration<double>(Clock::now() - start).count();
        cpu = ProcessCpu() - cpu;
        const auto stats = g->engine->stats();
        std::vector<double> latencies;
        for (const auto& r : observed_)
            if (r->operation == stage && r->start >= start)
                latencies.push_back(std::chrono::duration<double, std::milli>(r->end - r->start).count());
        Require(latencies.size() == operations, "measurement sample count mismatch");
        std::sort(latencies.begin(), latencies.end());
        auto percentile = [&](size_t p) { return latencies[(latencies.size() - 1) * p / 100]; };
        std::cout << std::fixed << std::setprecision(3) << "METRIC stage=" << stage << " operations=" << operations
                  << " bytes=" << bytes << " workers=" << stats.worker_count << " clients=" << stats.clients
                  << " wall_s=" << wall << " process_cpu_s=" << cpu << " avg_process_cores=" << cpu / wall
                  << " cpu_ms_per_op=" << cpu * 1000 / operations
                  << " MiB_per_s=" << bytes * double(operations) / MiB / wall << " rss_kib=" << Memory("VmRSS:")
                  << " peak_rss_kib=" << Memory("VmHWM:") << " connection_credits=" << stats.connection_credits
                  << " participating_workers=" << __builtin_popcountll(g->trace->worker_mask.load())
                  << " p50_ms=" << percentile(50) << " p95_ms=" << percentile(95) << " p99_ms=" << percentile(99)
                  << '\n';
    }
    void CloseAll() {
        for (auto& g : groups_)
            g->Close();
    }
    void Cleanup() {
        // First join every writer; fault injection intentionally only cancels
        // reads. A local cancel is NOT a remote rollback guarantee.
        CloseAll();
        auto g = Engine();
        auto& c = Client(g);
        size_t deleted_versions = 0;
        for (auto& m : multipart_)
            if (m->active)
                Abort(c, m, true);
        for (auto& o : objects_) {
            if (!o->may_exist)
                continue;
            Require(o->key.compare(0, prefix.size(), prefix) == 0, "cleanup key escaped owned prefix");
            auto head = Head(c, *o);
            bool alive = false;
            try {
                head->Wait();
                alive = true;
            } catch (const Failure&) {
                head->Wait(false, 404);
            }
            if (alive && !head->version.empty())
                o->versions.insert(head->version);
            std::vector<std::string> versions(o->versions.begin(), o->versions.end());
            if (versions.empty() && alive)
                versions.push_back("");
            for (const auto& version : versions) {
                auto r = State("DELETE exact owned version");
                DeleteObjectAsyncInput in(o->bucket, o->key);
                if (!version.empty())
                    in.setVersionId(version);
                c.deleteObjectAsync(in, [r](auto& out) { r->Done(out); });
                r->Wait();
                if (!version.empty())
                    ++deleted_versions;
            }
            Head(c, *o)->Wait(false, 404);
            o->may_exist = false;
        }
        if (probed_)
            for (const auto& bucket : buckets)
                Require(List(c, bucket, 100).empty(), "cleanup prefix not empty");
        CloseAll();
        VerifyCallbacks();
        std::cout << "CLEANUP verified_head_404=1 prefix_empty=" << probed_ << " objects=" << objects_.size()
                  << " deleted_explicit_versions=" << deleted_versions
                  << " callbacks_exactly_once=" << verified_callbacks_ << " trace_balanced=1 resources_drained=1\n";
    }
    void Pending() const {
        for (const auto& m : multipart_)
            if (m->active)
                std::cerr << "MULTIPART_PENDING key=" << m->object->key << " upload_id=" << m->id << '\n';
        for (const auto& o : objects_)
            if (o->may_exist)
                std::cerr << "CLEANUP_PENDING bucket=" << o->bucket << " key=" << o->key << '\n';
    }
};
int main(int argc, char** argv) {
    std::cout << std::unitbuf;
    std::unique_ptr<Run> run;
    bool suite_succeeded = false, cleaned = false;
    try {
        Require(argc >= 2,
                "usage: probe | contracts | multipart | failures | cross_bucket | cross_bucket_cold | "
                "reliability_mixed rounds | reliability_lifecycle rounds | reliability_faults rounds | "
                "load workers clients concurrency operations bytes");
        const std::string suite = argv[1];
        const bool reliability =
                suite == "reliability_mixed" || suite == "reliability_lifecycle" || suite == "reliability_faults";
        Require(suite == "probe" || suite == "contracts" || suite == "multipart" || suite == "failures" ||
                        suite == "cross_bucket" || suite == "cross_bucket_cold" || (reliability && argc == 3) ||
                        (suite == "load" && argc == 7),
                "invalid suite arguments");
        run.reset(new Run());
        run->Probe();
        if (suite == "probe") {
            // All requests above are read-only, including cleanup of an empty run.
        } else if (suite == "contracts")
            run->Contracts();
        else if (suite == "multipart")
            run->MultipartContracts();
        else if (suite == "failures")
            run->Failures();
        else if (suite == "cross_bucket" || suite == "cross_bucket_cold")
            run->CrossBucket(suite == "cross_bucket_cold");
        else if (suite == "reliability_mixed")
            run->ReliabilityMixed(std::stoull(argv[2]));
        else if (suite == "reliability_lifecycle")
            run->ReliabilityLifecycle(std::stoull(argv[2]));
        else if (suite == "reliability_faults")
            run->ReliabilityFaults(std::stoull(argv[2]));
        else
            run->Load(std::stoull(argv[2]), std::stoull(argv[3]), std::stoull(argv[4]), std::stoull(argv[5]),
                      std::stoull(argv[6]));
        suite_succeeded = true;
    } catch (const Failure& e) {
        std::cerr << "FAIL " << e.what() << '\n';
    } catch (...) {
        std::cerr << "FAIL unexpected exception (details redacted)\n";
    }
    if (run) {
        try {
            run->Cleanup();
            cleaned = true;
        } catch (const Failure& e) {
            std::cerr << "FAIL cleanup: " << e.what() << '\n';
        } catch (...) {
            std::cerr << "FAIL cleanup exception (details redacted)\n";
        }
        if (!cleaned) {
            try {
                run->CloseAll();
            } catch (...) {
            }
            run->Pending();
        }
    }
    if (suite_succeeded && cleaned) {
        std::cout << "PASS real-bucket shared-engine E2E\n";
        return 0;
    }
    std::cout << "FAIL real-bucket shared-engine E2E\n";
    return 1;
}
