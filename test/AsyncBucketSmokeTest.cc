// Explicit opt-in only: creates exactly one uniquely named object
// per process, verifies native async PUT/HEAD/GET/Range/DELETE, then removes it.
// Credentials are read at runtime and are never logged or compiled in.
#include "TosAsyncClient.h"
#include "logger/logger.h"
#include <sys/resource.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <vector>

using namespace VolcengineTos;
using Clock = std::chrono::steady_clock;
struct Result {
    std::mutex mutex;
    std::condition_variable cv;
    unsigned calls = 0;
    bool success = false;
    int status = 0;
    int curl_error = 0;
    size_t bytes = 0;
    bool valid_bytes = true;
    template <typename T>
    void Done(T& outcome) {
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        success = outcome.isSuccess();
        status = outcome.error().getStatusCode();
        curl_error = outcome.error().getCurlErrCode();
        cv.notify_all();
    }
    void Wait(bool expected_success = true, int expected_status = 0) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!cv.wait_for(lock, std::chrono::seconds(60), [&] { return calls != 0; }))
            throw std::runtime_error("callback deadline exceeded");
        if (calls != 1 || success != expected_success || (expected_status && status != expected_status))
            throw std::runtime_error("operation failed: status=" + std::to_string(status) +
                                     " curl=" + std::to_string(curl_error) + " calls=" + std::to_string(calls));
    }
};
std::string Env(const char* name, const char* fallback = nullptr) {
    const char* value = std::getenv(name);
    if (value && *value) return value;
    if (fallback) return fallback;
    throw std::runtime_error(std::string("missing environment variable: ") + name);
}
double CpuSeconds() {
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage)) throw std::runtime_error("getrusage failed");
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 + usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
}
long PeakRssKiB() {
    // VmHWM is scoped to this executable's address space. ru_maxrss can include
    // a launcher's pre-exec high-water mark on Linux and distort small clients.
    std::ifstream status("/proc/self/status");
    std::string name;
    while (status >> name) {
        if (name == "VmHWM:") {
            long value = 0;
            status >> value;
            return value;
        }
        std::string rest;
        std::getline(status, rest);
    }
    throw std::runtime_error("VmHWM unavailable");
}

int main(int argc, char** argv) {
    std::unique_ptr<TosAsyncClient> client;
    std::string bucket, key;
    bool may_exist = false;
    bool versioned_cleanup = false;
    std::function<std::unique_ptr<TosAsyncClient>()> make_client;
    std::vector<std::shared_ptr<Result>> observations;
    auto state = [&] {
        auto value = std::make_shared<Result>();
        observations.push_back(value);
        return value;
    };
    auto remove = [&] {
        if (!client || !may_exist) return;
        auto metadata = state();
        auto version = std::make_shared<std::string>();
        client->headObjectAsync(HeadObjectAsyncInput(bucket, key), [metadata, version](auto& o) {
            if (o.isSuccess()) *version = o.result().getVersionId();
            metadata->Done(o);
        });
        try {
            metadata->Wait();
        } catch (...) {
            std::lock_guard<std::mutex> lock(metadata->mutex);
            if (metadata->calls == 1 && metadata->status == 404) {
                may_exist = false;
                return;
            }
            throw;
        }
        // Remove the exact version if versioning is enabled, not just create a
        // delete marker hiding a test object's retained storage.
        DeleteObjectAsyncInput deletion(bucket, key);
        if (!version->empty()) {
            deletion.setVersionId(*version);
            versioned_cleanup = true;
        }
        auto deleted = state();
        client->deleteObjectAsync(deletion, [deleted](auto& o) { deleted->Done(o); });
        deleted->Wait();
        auto absent = state();
        client->headObjectAsync(HeadObjectAsyncInput(bucket, key), [absent](auto& o) { absent->Done(o); });
        absent->Wait(false, 404);
        may_exist = false;
    };
    try {
        const size_t bytes = argc > 1 ? std::stoull(argv[1]) : 4096;
        const size_t operations = argc > 2 ? std::stoull(argv[2]) : 256;
        const size_t concurrency = argc > 3 ? std::stoull(argv[3]) : 16;
        const int workers = argc > 4 ? std::stoi(argv[4]) : 1;
        if (bytes < 4096 || bytes > 100 * 1024 * 1024 || operations == 0 || operations > 10000 || concurrency == 0 ||
            concurrency > 128 || workers < 1 || workers > 8)
            throw std::runtime_error("arguments outside finite test limits");
        bucket = Env("FSX_EXTERNAL_BUCKET");
        if (bucket != "xrh-bj-tosfuse-test") throw std::runtime_error("refusing non-authorized test bucket");
        key = "sdk-review-169/" +
              std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::system_clock::now().time_since_epoch())
                                 .count()) +
              "-" + std::to_string(getpid()) + "/data";
        const auto endpoint = Env("FSX_EXTERNAL_ENDPOINT", "http://tos-cn-beijing.volces.com");
        if (endpoint.compare(0, 7, "http://") != 0) throw std::runtime_error("this benchmark requires HTTP");
        ClientConfig config;
        config.endPoint = endpoint;
        config.event_thread_count_ = workers;
        config.async_transport_mode_ = AsyncTransportMode::Isolated;
        config.maxConnections = static_cast<int>(concurrency);
        config.max_request_queue_ = static_cast<int>(concurrency * 2);
        config.maxRetryCount = 0;
        config.requestTimeout = 45000;
        config.connectionTimeout = 5000;
        config.enableCRC = false;
        config.enableDebug = false;
        config.detail_log_ = false;
        config.connection_reuse_ = true;
        Logger::getInstance().setAsyncLogLevel(ERROR);
        make_client = [config] {
            return std::unique_ptr<TosAsyncClient>(new TosAsyncClient(
                Env("FSX_EXTERNAL_REGION", "cn-beijing"), Env("FSX_EXTERNAL_AK"), Env("FSX_EXTERNAL_SK"), config));
        };
        client = make_client();
        auto absent = state();
        client->headObjectAsync(HeadObjectAsyncInput(bucket, key), [absent](auto& o) { absent->Done(o); });
        absent->Wait(false, 404);  // Never overwrite a preexisting key.
        auto upload = state();
        PutObjectAsyncInput put(bucket, key, TransferEncoding::ContentLength);
        put.setContentLength(static_cast<int64_t>(bytes));
        auto sent = std::make_shared<size_t>(0);
        may_exist = true;  // Includes ambiguous completion/timeout cleanup.
        client->putObjectAsync(put,
                               [sent, bytes](char* data, size_t length, AsyncEvent*) {
                                   length = std::min(length, bytes - *sent);
                                   std::memset(data, 'x', length);
                                   *sent += length;
                                   return length;
                               },
                               [upload](auto& o) { upload->Done(o); });
        upload->Wait();
        auto head = state();
        client->headObjectAsync(HeadObjectAsyncInput(bucket, key), [head](auto& o) { head->Done(o); });
        head->Wait();

        auto get = [&](bool verify, const std::string& range = std::string()) {
            auto result = state();
            GetObjectAsyncInput input(bucket, key);
            if (!range.empty()) input.setRange(range);
            client->getObjectAsync(input,
                                   [result, verify](char* data, size_t length, AsyncEvent*) {
                                       result->bytes += length;
                                       if (verify && !std::all_of(data, data + length, [](char c) { return c == 'x'; }))
                                           result->valid_bytes = false;
                                       return length;
                                   },
                                   [result](auto& o) { result->Done(o); });
            return result;
        };
        auto whole = get(true);
        whole->Wait();
        if (whole->bytes != bytes || !whole->valid_bytes) throw std::runtime_error("full GET data mismatch");
        auto range = get(true, "bytes=17-1040");
        range->Wait();
        if (range->bytes != 1024 || !range->valid_bytes) throw std::runtime_error("Range GET mismatch");

        // Timed GET consumes without body copies/checksums. Full content was
        // verified above, outside the measurement. CPU includes all SDK threads.
        const double cpu_start = CpuSeconds();
        const auto start = Clock::now();
        for (size_t offset = 0; offset < operations; offset += concurrency) {
            std::vector<std::shared_ptr<Result>> wave;
            for (size_t i = 0; i < std::min(concurrency, operations - offset); ++i) wave.push_back(get(false));
            for (auto& result : wave) {
                result->Wait();
                if (result->bytes != bytes) throw std::runtime_error("benchmark GET length mismatch");
            }
        }
        const double wall = std::chrono::duration<double>(Clock::now() - start).count();
        const double cpu = CpuSeconds() - cpu_start;
        const long rss = PeakRssKiB();
        remove();
        client->close();
        for (const auto& observed : observations) {
            std::lock_guard<std::mutex> lock(observed->mutex);
            if (observed->calls != 1) throw std::runtime_error("duplicate callback after drain");
        }
        std::cout << std::fixed << std::setprecision(3) << "PASS bytes=" << bytes << " operations=" << operations
                  << " concurrency=" << concurrency << " workers=" << workers << " wall_s=" << wall << " cpu_s=" << cpu
                  << " avg_cpu_cores=" << cpu / wall << " cpu_us_per_op=" << cpu * 1e6 / operations
                  << " MiB_per_s=" << bytes * static_cast<double>(operations) / 1048576 / wall
                  << " peak_RSS_KiB=" << rss << " cleanup=verified_404 versioned_cleanup=" << versioned_cleanup << '\n';
        return 0;
    } catch (const std::exception& error) {
        // The exception messages above never contain headers or credential values.
        std::cerr << "FAIL: " << error.what() << '\n';
        if (client) client->close();
        // Drain ambiguous PUTs before DELETE, so a late upload cannot recreate
        // an object after cleanup reported 404. Use a fresh client after drain.
        try {
            if (may_exist && make_client) {
                client = make_client();
                remove();
                client->close();
            }
        } catch (...) {
            std::cerr << "cleanup incomplete: " << key << '\n';
        }
        return 1;
    }
}
