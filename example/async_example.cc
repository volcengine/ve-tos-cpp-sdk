#include "TosAsyncClient.h"
#include "logger/logger.h"
#include "utils/crc64.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace VolcengineTos;

namespace {

template <typename Output>
using OutcomeCallback = std::function<void(Outcome<TosError, Output>&)>;

template <typename Output>
using TransferCallback = std::function<void(Outcome<TosError, Output>&, FileTransferResult)>;

struct Options {
    std::string bucket;
    std::string region;
    std::string endpoint;
    std::string access_key;
    std::string secret_key;
    std::string prefix = "async-example";
    std::string workdir = "/tmp/ve-tos-cpp-sdk-async-example";
    bool test_create_bucket = false;
};

struct SendState {
    explicit SendState(std::string in) : data(std::move(in)) {}
    std::string data;
    std::size_t offset{0};
};

struct PausingSendState {
    explicit PausingSendState(std::string in, int pauses, int delay_ms)
            : data(std::move(in)), pauses_remaining(pauses), pause_delay_ms(delay_ms) {
    }
    std::string data;
    std::size_t offset{0};
    int pauses_remaining{0};
    int pause_delay_ms{0};
};

struct PausingReceiveState {
    PausingReceiveState(std::shared_ptr<std::string> out, int pauses, int delay_ms)
            : data(std::move(out)), pauses_remaining(pauses), pause_delay_ms(delay_ms) {
    }
    std::shared_ptr<std::string> data;
    int pauses_remaining{0};
    int pause_delay_ms{0};
};

template <typename Output>
struct TransferResultHolder {
    Outcome<TosError, Output> outcome;
    FileTransferResult transfer;
};

class Reporter {
public:
    void pass(const std::string& name) {
        ++passed_;
        std::cout << "[PASS] " << name << std::endl;
    }

    void skip(const std::string& name, const std::string& reason) {
        ++skipped_;
        std::cout << "[SKIP] " << name << ": " << reason << std::endl;
    }

    void fail(const std::string& name, const std::string& error) {
        ++failed_;
        std::cerr << "[FAIL] " << name << ": " << error << std::endl;
    }

    template <typename Output>
    bool expectSuccess(const std::string& name, const Outcome<TosError, Output>& outcome) {
        if (outcome.isSuccess()) {
            pass(name);
            return true;
        }
        fail(name, outcome.error().String());
        return false;
    }

    template <typename Output>
    bool expectFailure(const std::string& name, const Outcome<TosError, Output>& outcome) {
        if (!outcome.isSuccess()) {
            pass(name);
            return true;
        }
        fail(name, "unexpected success");
        return false;
    }

    bool expectTransfer(const std::string& name, const FileTransferResult& transfer,
                        std::uint64_t expected_bytes) {
        if (!transfer.status.ok) {
            fail(name, transfer.status.message);
            return false;
        }
        if (transfer.bytes != expected_bytes) {
            std::ostringstream ss;
            ss << "transfer bytes mismatch, expected=" << expected_bytes << ", actual=" << transfer.bytes;
            fail(name, ss.str());
            return false;
        }
        pass(name + " transfer");
        return true;
    }

    int failures() const {
        return failed_;
    }

    void summary() const {
        std::cout << "summary: passed=" << passed_ << ", skipped=" << skipped_
                  << ", failed=" << failed_ << std::endl;
    }

private:
    int passed_{0};
    int skipped_{0};
    int failed_{0};
};

class DelayedIoUringHook : public FileTransferHook {
public:
    explicit DelayedIoUringHook(int delay_ms) : delay_ms_(delay_ms) {}

    FileTransferStatus OnAfterRead(const FileIoEvent& ev, const FileIoResult&) override {
        if (ev.backend == FileIoBackend::IoUring) {
            ++after_reads_;
            Delay();
        }
        return FileTransferStatus::Ok();
    }

    FileTransferStatus OnAfterWrite(const FileIoEvent& ev, const FileIoResult&) override {
        if (ev.backend == FileIoBackend::IoUring) {
            ++after_writes_;
            Delay();
        }
        return FileTransferStatus::Ok();
    }

    int afterReads() const {
        return after_reads_.load();
    }

    int afterWrites() const {
        return after_writes_.load();
    }

private:
    void Delay() const {
        if (delay_ms_ > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));
        }
    }

    int delay_ms_{0};
    std::atomic<int> after_reads_{0};
    std::atomic<int> after_writes_{0};
};

const char* Env(const char* name) {
    return std::getenv(name);
}

std::string GetEnv(const char* name, const std::string& default_value) {
    const char* value = Env(name);
    return value != nullptr ? std::string(value) : default_value;
}

std::string RequireEnv(const char* name) {
    const char* value = Env(name);
    if (value == nullptr || std::string(value).empty()) {
        throw std::runtime_error(std::string("missing required environment: ") + name);
    }
    return std::string(value);
}

Options ParseOptions(int argc, char** argv) {
    Options options;
    options.bucket = GetEnv("TOS_E2E_BUCKET", "");
    options.region = GetEnv("TOS_E2E_REGION", "");
    options.endpoint = GetEnv("TOS_E2E_ENDPOINT", "");
    options.access_key = GetEnv("TOS_E2E_AK", "");
    options.secret_key = GetEnv("TOS_E2E_SK", "");
    options.prefix = GetEnv("TOS_E2E_PREFIX", options.prefix);
    options.workdir = GetEnv("TOS_ASYNC_API_SMOKE_WORKDIR", options.workdir);
    options.workdir = GetEnv("TOS_ASYNC_EXAMPLE_WORKDIR", options.workdir);
    options.test_create_bucket = GetEnv("TOS_E2E_TEST_CREATE_BUCKET", "") == "1";

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--bucket" && i + 1 < argc) {
            options.bucket = argv[++i];
        } else if (arg == "--region" && i + 1 < argc) {
            options.region = argv[++i];
        } else if (arg == "--endpoint" && i + 1 < argc) {
            options.endpoint = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.prefix = argv[++i];
        } else if (arg == "--workdir" && i + 1 < argc) {
            options.workdir = argv[++i];
        } else if (arg == "--test-create-bucket") {
            options.test_create_bucket = true;
        } else {
            throw std::runtime_error("unknown or incomplete argument: " + arg);
        }
    }

    if (options.bucket.empty()) {
        options.bucket = RequireEnv("TOS_E2E_BUCKET");
    }
    if (options.region.empty()) {
        options.region = RequireEnv("TOS_E2E_REGION");
    }
    if (options.endpoint.empty()) {
        options.endpoint = RequireEnv("TOS_E2E_ENDPOINT");
    }
    if (options.access_key.empty()) {
        options.access_key = RequireEnv("TOS_E2E_AK");
    }
    if (options.secret_key.empty()) {
        options.secret_key = RequireEnv("TOS_E2E_SK");
    }
    return options;
}

void EnsureDirectory(const std::string& path) {
    if (::mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        throw std::runtime_error("mkdir failed for " + path + ": " + std::strerror(errno));
    }
}

void WriteAll(int fd, const char* data, std::size_t size) {
    std::size_t written = 0;
    while (written < size) {
        ssize_t n = ::write(fd, data + written, size - written);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
        }
        written += static_cast<std::size_t>(n);
    }
}

void WriteFile(const std::string& path, const std::string& data) {
    int fd = ::open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) {
        throw std::runtime_error("open failed for " + path + ": " + std::strerror(errno));
    }
    try {
        WriteAll(fd, data.data(), data.size());
        if (::close(fd) != 0) {
            throw std::runtime_error(std::string("close failed: ") + std::strerror(errno));
        }
    } catch (...) {
        ::close(fd);
        throw;
    }
}

std::string ReadFile(const std::string& path) {
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("open failed for " + path + ": " + std::strerror(errno));
    }
    std::string out;
    char buffer[4096];
    for (;;) {
        ssize_t n = ::read(fd, buffer, sizeof(buffer));
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ::close(fd);
            throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
        }
        if (n == 0) {
            break;
        }
        out.append(buffer, static_cast<std::size_t>(n));
    }
    ::close(fd);
    return out;
}

std::string RepeatData(const std::string& seed, std::size_t size) {
    std::string data;
    data.reserve(size);
    while (data.size() < size) {
        data.append(seed);
    }
    data.resize(size);
    return data;
}

bool WaitForEventResume(AsyncEvent& ev, int timeout_ms) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        if (ev.isResumed() || ev.isFailed()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}

std::uint64_t CalculateCrc64(const std::string& data) {
    if (data.empty()) {
        return 0;
    }
    std::vector<char> buffer(data.begin(), data.end());
    return CRC64::CalcCRC(0, buffer.data(), buffer.size());
}

template <typename Output>
bool ExpectCrc64(const std::string& name, const Outcome<TosError, Output>& outcome,
                 const std::string& data, Reporter& reporter, bool require_calculated = true) {
    if (!outcome.isSuccess()) {
        return false;
    }

    const std::uint64_t expected = CalculateCrc64(data);
    const std::uint64_t service_crc = outcome.result().getHashCrc64Ecma();
    if (service_crc != expected) {
        std::ostringstream ss;
        ss << "service crc mismatch, expected=" << expected << ", actual=" << service_crc;
        reporter.fail(name, ss.str());
        return false;
    }

    if (require_calculated) {
        const std::uint64_t calculated_crc = outcome.result().getCalHashCrc64Ecma();
        if (calculated_crc != expected) {
            std::ostringstream ss;
            ss << "calculated crc mismatch, expected=" << expected
               << ", actual=" << calculated_crc;
            reporter.fail(name, ss.str());
            return false;
        }
    }

    reporter.pass(name);
    return true;
}

std::string JoinPath(const std::string& dir, const std::string& leaf) {
    if (!dir.empty() && dir[dir.size() - 1] == '/') {
        return dir + leaf;
    }
    return dir + "/" + leaf;
}

std::string Key(const Options& options, const std::string& leaf) {
    return options.prefix + "-" + std::to_string(static_cast<long long>(::getpid())) + "/" + leaf;
}

OnDataSendWithEvent MakeSender(const std::string& data) {
    auto state = std::make_shared<SendState>(data);
    return [state](char* out, size_t len, AsyncEvent*) -> size_t {
        if (state->offset >= state->data.size()) {
            return 0;
        }
        const std::size_t n = std::min(len, state->data.size() - state->offset);
        std::memcpy(out, state->data.data() + state->offset, n);
        state->offset += n;
        return n;
    };
}

OnDataReceiveWithEvent MakeReceiver(const std::shared_ptr<std::string>& data) {
    return [data](char* in, size_t len, AsyncEvent*) -> size_t {
        data->append(in, len);
        return len;
    };
}

OnDataSendWithEvent MakePauseThenSender(const std::string& data, int pauses, int pause_delay_ms) {
    auto state = std::make_shared<PausingSendState>(data, pauses, pause_delay_ms);
    return [state](char* out, size_t len, AsyncEvent* ev) -> size_t {
        if (state->offset >= state->data.size()) {
            return 0;
        }
        if (state->pauses_remaining > 0) {
            --state->pauses_remaining;
            if (ev != nullptr) {
                ev->PauseFor(state->pause_delay_ms);
            }
            return 0;
        }
        const std::size_t n = std::min(len, state->data.size() - state->offset);
        std::memcpy(out, state->data.data() + state->offset, n);
        state->offset += n;
        return n;
    };
}

OnDataSendWithEvent MakeMinusOneSender(const std::string& data) {
    auto state = std::make_shared<SendState>(data);
    return [state](char* out, size_t len, AsyncEvent*) -> size_t {
        if (state->offset >= state->data.size() || len == 0) {
            return 0;
        }
        const std::size_t remaining = state->data.size() - state->offset;
        const std::size_t limit = len > 1 ? len - 1 : 1;
        const std::size_t n = std::min(limit, remaining);
        std::memcpy(out, state->data.data() + state->offset, n);
        state->offset += n;
        return n;
    };
}

OnDataSendWithEvent MakeNoDataSender() {
    return [](char*, size_t, AsyncEvent*) -> size_t {
        return 0;
    };
}

OnDataReceiveWithEvent MakePauseThenReceiver(const std::shared_ptr<std::string>& data, int pauses,
                                             int pause_delay_ms) {
    auto state = std::make_shared<PausingReceiveState>(data, pauses, pause_delay_ms);
    return [state](char* in, size_t len, AsyncEvent* ev) -> size_t {
        if (state->pauses_remaining > 0) {
            --state->pauses_remaining;
            if (ev != nullptr) {
                ev->PauseFor(state->pause_delay_ms);
            }
            return 0;
        }
        state->data->append(in, len);
        return len;
    };
}

OnDataReceiveWithEvent MakeMinusOneReceiver(const std::shared_ptr<std::string>& data) {
    return [data](char* in, size_t len, AsyncEvent*) -> size_t {
        if (len == 0) {
            return 0;
        }
        const std::size_t n = len > 1 ? len - 1 : 1;
        data->append(in, n);
        return n;
    };
}

OnDataReceiveWithEvent MakeNoConsumeReceiver() {
    return [](char*, size_t, AsyncEvent*) -> size_t {
        return 0;
    };
}

template <typename Output, typename Invoke>
Outcome<TosError, Output> AwaitOutcome(Invoke invoke) {
    auto promise = std::make_shared<std::promise<Outcome<TosError, Output>>>();
    std::future<Outcome<TosError, Output>> future = promise->get_future();
    OutcomeCallback<Output> callback = [promise](Outcome<TosError, Output>& outcome) mutable {
        promise->set_value(outcome);
    };
    invoke(callback);
    if (future.wait_for(std::chrono::seconds(120)) != std::future_status::ready) {
        throw std::runtime_error("async operation timed out");
    }
    return future.get();
}

bool VerifyObjectData(TosAsyncClient& client, const Options& options, const std::string& key,
                      const std::string& expected, Reporter& reporter, const std::string& name) {
    auto received = std::make_shared<std::string>();
    GetObjectAsyncInput input(options.bucket, key);
    auto output = AwaitOutcome<GetObjectAsyncOutput>(
            [&](const OutcomeCallback<GetObjectAsyncOutput>& cb) {
                client.getObjectAsync(input, MakeReceiver(received), cb);
            });
    if (!reporter.expectSuccess(name, output)) {
        return false;
    }
    if (*received != expected) {
        reporter.fail(name + " data", "downloaded data mismatch");
        return false;
    }
    reporter.pass(name + " data");
    ExpectCrc64(name + " crc64", output, expected, reporter);
    return true;
}

template <typename Output, typename Invoke>
TransferResultHolder<Output> AwaitTransfer(Invoke invoke) {
    auto promise = std::make_shared<std::promise<TransferResultHolder<Output>>>();
    std::future<TransferResultHolder<Output>> future = promise->get_future();
    TransferCallback<Output> callback =
            [promise](Outcome<TosError, Output>& outcome, FileTransferResult transfer) mutable {
                TransferResultHolder<Output> result;
                result.outcome = outcome;
                result.transfer = transfer;
                promise->set_value(result);
            };
    invoke(callback);
    if (future.wait_for(std::chrono::seconds(120)) != std::future_status::ready) {
        throw std::runtime_error("async transfer operation timed out");
    }
    return future.get();
}

FileTransferOptions PosixTransferOptions() {
    FileTransferOptions options;
    options.backend = FileIoBackend::Posix;
    options.chunk_size = 1024 * 1024;
    options.io_depth = 4;
    options.close_fd_on_done = true;
    options.truncate_file_on_done = true;
    return options;
}

FileTransferOptions IoUringTransferOptions(const std::shared_ptr<FileTransferHook>& hook = nullptr) {
    FileTransferOptions options;
    options.backend = FileIoBackend::IoUring;
    options.chunk_size = 1024;
    options.io_depth = 1;
    options.close_fd_on_done = true;
    options.truncate_file_on_done = true;
    if (hook) {
        options.hooks.push_back(hook);
    }
    return options;
}

bool ProbeIoUringUploadPause(const std::string& path, const std::string& data, Reporter& reporter) {
    auto hook = std::make_shared<DelayedIoUringHook>(10);
    FileRange range;
    range.path = path;
    range.length = data.size();
    auto source = CreateFileRangeAsyncDataSource(range, IoUringTransferOptions(hook));
    if (!source) {
        reporter.fail("io_uring upload source pause/resume", "source missing");
        return false;
    }
    FileTransferResult initial = source->result();
    if (!initial.status.ok) {
        reporter.skip("io_uring upload source pause/resume", initial.status.message);
        return false;
    }

    std::vector<char> buffer(4096);
    std::string received;
    std::size_t pauses = 0;
    while (received.size() < data.size()) {
        AsyncEvent ev;
        const std::size_t n = source->Read(buffer.data(), buffer.size(), &ev);
        if (n > 0) {
            received.append(buffer.data(), n);
            continue;
        }
        if (!ev.isPaused()) {
            reporter.fail("io_uring upload source pause/resume",
                          "source returned 0 without pausing before EOF");
            return false;
        }
        ++pauses;
        if (!WaitForEventResume(ev, 5000) || ev.isFailed()) {
            reporter.fail("io_uring upload source pause/resume", "source did not resume");
            return false;
        }
    }

    FileTransferResult result = source->Finish(FileTransferStatus::Ok());
    if (!result.status.ok) {
        reporter.fail("io_uring upload source pause/resume", result.status.message);
        return false;
    }
    if (received != data) {
        reporter.fail("io_uring upload source pause/resume", "read data mismatch");
        return false;
    }
    if (pauses == 0 || hook->afterReads() == 0) {
        reporter.fail("io_uring upload source pause/resume", "pause or read hook was not observed");
        return false;
    }
    reporter.pass("io_uring upload source pause/resume");
    return true;
}

bool ProbeIoUringDownloadPause(const std::string& path, const std::string& data, Reporter& reporter) {
    auto hook = std::make_shared<DelayedIoUringHook>(10);
    FileRange range;
    range.path = path;
    range.length = data.size();
    auto sink = CreateFileRangeDownloadSink(range, IoUringTransferOptions(hook));
    if (!sink) {
        reporter.fail("io_uring download sink pause/resume", "sink missing");
        return false;
    }
    FileTransferResult initial = sink->result();
    if (!initial.status.ok) {
        reporter.skip("io_uring download sink pause/resume", initial.status.message);
        return false;
    }

    std::vector<char> payload(data.begin(), data.end());
    std::size_t offset = 0;
    std::size_t pauses = 0;
    while (offset < payload.size()) {
        AsyncEvent ev;
        const std::size_t n =
                sink->Write(payload.data() + offset, payload.size() - offset, &ev);
        offset += n;
        if (ev.isPaused()) {
            ++pauses;
            if (!WaitForEventResume(ev, 5000) || ev.isFailed()) {
                reporter.fail("io_uring download sink pause/resume", "sink did not resume");
                return false;
            }
        } else if (n == 0) {
            reporter.fail("io_uring download sink pause/resume",
                          "sink returned 0 without pausing before EOF");
            return false;
        }
    }

    auto promise = std::make_shared<std::promise<FileTransferResult>>();
    std::future<FileTransferResult> future = promise->get_future();
    sink->FinishAsync(FileTransferStatus::Ok(),
                      [promise](FileTransferResult result) mutable { promise->set_value(result); });
    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
        reporter.fail("io_uring download sink pause/resume", "finish did not complete");
        return false;
    }
    FileTransferResult result = future.get();
    if (!result.status.ok) {
        reporter.fail("io_uring download sink pause/resume", result.status.message);
        return false;
    }
    if (ReadFile(path) != data) {
        reporter.fail("io_uring download sink pause/resume", "written data mismatch");
        return false;
    }
    if (pauses == 0 || hook->afterWrites() == 0) {
        reporter.fail("io_uring download sink pause/resume", "pause or write hook was not observed");
        return false;
    }
    reporter.pass("io_uring download sink pause/resume");
    return true;
}

void CleanupKeys(TosAsyncClient& client, const Options& options, const std::vector<std::string>& keys) {
    for (const auto& key : keys) {
        DeleteObjectAsyncInput input(options.bucket, key);
        (void)AwaitOutcome<DeleteObjectAsyncOutput>(
                [&](const OutcomeCallback<DeleteObjectAsyncOutput>& cb) {
                    client.deleteObjectAsync(input, cb);
                });
    }
}

void TestCreateBucketOnExistingBucket(TosAsyncClient& client, const Options& options, Reporter& reporter) {
    CreateBucketAsyncInput input(options.bucket);
    auto output = AwaitOutcome<CreateBucketAsyncOutput>(
            [&](const OutcomeCallback<CreateBucketAsyncOutput>& cb) {
                client.createBucketAsync(input, cb);
            });
    if (output.isSuccess()) {
        reporter.pass("createBucketAsync(existing bucket)");
        return;
    }
    const int status = output.error().getStatusCode();
    if (status == 409 || status == 400) {
        reporter.pass("createBucketAsync(existing bucket expected failure)");
        return;
    }
    reporter.fail("createBucketAsync(existing bucket)", output.error().String());
}

ClientConfig MakeConfig(const Options& options) {
    ClientConfig config;
    config.endPoint = options.endpoint;
    config.enableCRC = true;
    config.detail_log_ = false;
    config.maxConnections = 64;
    config.event_thread_count_ = 8;
    config.requestTimeout = 120000;
    config.socketTimeout = 30000;
    config.fileTransferIoDepth = 4;
    return config;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    Reporter reporter;
    std::vector<std::string> cleanup_keys;
    try {
        options = ParseOptions(argc, argv);
        EnsureDirectory(options.workdir);
        Logger::getInstance().setAsyncLogLevel(ERROR);

        InitializeTosAsyncClient();
        TosAsyncClient client(options.region, options.access_key, options.secret_key, MakeConfig(options));

        std::cout << "bucket=" << options.bucket << ", region=" << options.region
                  << ", endpoint=" << options.endpoint << std::endl;

        if (!options.test_create_bucket) {
            reporter.skip("createBucketAsync", "requires explicit --test-create-bucket");
        }

        HeadBucketAsyncInput head_bucket(options.bucket);
        auto head_bucket_out = AwaitOutcome<HeadBucketAsyncOutput>(
                [&](const OutcomeCallback<HeadBucketAsyncOutput>& cb) {
                    client.headBucketAsync(head_bucket, cb);
                });
        const bool has_bucket = reporter.expectSuccess("headBucketAsync", head_bucket_out);

        auto bucket_type_out = AwaitOutcome<BucketType>(
                [&](const OutcomeCallback<BucketType>& cb) {
                    client.getBucketTypeAsync(options.bucket, cb);
                });
        bool is_hns = false;
        if (reporter.expectSuccess("getBucketTypeAsync", bucket_type_out)) {
            is_hns = bucket_type_out.result() == BucketType::HNS;
            std::cout << "bucket_type=" << (is_hns ? "hns" : "fns") << std::endl;
        }

        ListBucketsAsyncInput list_buckets;
        auto list_buckets_out = AwaitOutcome<ListBucketsAsyncOutput>(
                [&](const OutcomeCallback<ListBucketsAsyncOutput>& cb) {
                    client.listBucketsAsync(list_buckets, cb);
                });
        reporter.expectSuccess("listBucketsAsync", list_buckets_out);

        if (!has_bucket) {
            reporter.fail("setup", "bucket is not accessible");
            reporter.summary();
            CloseTosAsyncClient();
            return 2;
        }

        const std::string iouring_probe_data =
                RepeatData("async-iouring-pause-probe-", 256 * 1024 + 19);
        const std::string iouring_probe_upload_path =
                JoinPath(options.workdir, "iouring-probe-upload.bin");
        const std::string iouring_probe_download_path =
                JoinPath(options.workdir, "iouring-probe-download.bin");
        WriteFile(iouring_probe_upload_path, iouring_probe_data);
        const bool iouring_upload_probe_ok =
                ProbeIoUringUploadPause(iouring_probe_upload_path, iouring_probe_data, reporter);
        const bool iouring_download_probe_ok =
                ProbeIoUringDownloadPause(iouring_probe_download_path, iouring_probe_data, reporter);
        const bool iouring_local_probe_ok = iouring_upload_probe_ok && iouring_download_probe_ok;

        const std::string base_key = Key(options, "base.txt");
        const std::string base_data = "async example base object";
        cleanup_keys.push_back(base_key);

        PutObjectAsyncInput put_input(options.bucket, base_key, TransferEncoding::ContentLength);
        put_input.setContentLength(static_cast<int64_t>(base_data.size()));
        auto put_out = AwaitOutcome<PutObjectAsyncOutput>(
                [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectAsync(put_input, MakeSender(base_data), cb);
                });
        if (reporter.expectSuccess("putObjectAsync", put_out)) {
            ExpectCrc64("putObjectAsync crc64", put_out, base_data, reporter);
        }

        auto received = std::make_shared<std::string>();
        GetObjectAsyncInput get_input(options.bucket, base_key);
        auto get_out = AwaitOutcome<GetObjectAsyncOutput>(
                [&](const OutcomeCallback<GetObjectAsyncOutput>& cb) {
                    client.getObjectAsync(get_input, MakeReceiver(received), cb);
                });
        if (reporter.expectSuccess("getObjectAsync", get_out)) {
            if (*received != base_data) {
                reporter.fail("getObjectAsync data", "downloaded data mismatch");
            } else {
                reporter.pass("getObjectAsync data");
                ExpectCrc64("getObjectAsync crc64", get_out, base_data, reporter);
            }
        }

        const std::string event_data = RepeatData("async-event-data-", 256 * 1024 + 17);
        const std::string event_pause_upload_key = Key(options, "event-pause-upload.txt");
        cleanup_keys.push_back(event_pause_upload_key);
        PutObjectAsyncInput event_pause_put(
                options.bucket, event_pause_upload_key, TransferEncoding::ContentLength);
        event_pause_put.setContentLength(static_cast<int64_t>(event_data.size()));
        auto event_pause_put_out = AwaitOutcome<PutObjectAsyncOutput>(
                [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectAsync(
                            event_pause_put, MakePauseThenSender(event_data, 2, 10), cb);
                });
        if (reporter.expectSuccess("putObjectAsync AsyncEvent PauseFor zero-byte producer",
                                   event_pause_put_out)) {
            ExpectCrc64("putObjectAsync AsyncEvent PauseFor zero-byte producer crc64",
                        event_pause_put_out, event_data, reporter);
            VerifyObjectData(client, options, event_pause_upload_key, event_data, reporter,
                             "putObjectAsync AsyncEvent PauseFor zero-byte producer verify");
        }

        const std::string event_minus_one_key = Key(options, "event-minus-one-upload.txt");
        cleanup_keys.push_back(event_minus_one_key);
        PutObjectAsyncInput event_minus_one_put(
                options.bucket, event_minus_one_key, TransferEncoding::ContentLength);
        event_minus_one_put.setContentLength(static_cast<int64_t>(event_data.size()));
        auto event_minus_one_put_out = AwaitOutcome<PutObjectAsyncOutput>(
                [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectAsync(event_minus_one_put, MakeMinusOneSender(event_data), cb);
                });
        if (reporter.expectSuccess("putObjectAsync short producer(len-1)", event_minus_one_put_out)) {
            ExpectCrc64("putObjectAsync short producer(len-1) crc64",
                        event_minus_one_put_out, event_data, reporter);
            VerifyObjectData(client, options, event_minus_one_key, event_data, reporter,
                             "putObjectAsync short producer(len-1) verify");
        }

        const std::string event_no_data_key = Key(options, "event-no-data-upload.txt");
        cleanup_keys.push_back(event_no_data_key);
        PutObjectAsyncInput event_no_data_put(
                options.bucket, event_no_data_key, TransferEncoding::ContentLength);
        event_no_data_put.setContentLength(static_cast<int64_t>(event_data.size()));
        auto event_no_data_put_out = AwaitOutcome<PutObjectAsyncOutput>(
                [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectAsync(event_no_data_put, MakeNoDataSender(), cb);
                });
        reporter.expectFailure("putObjectAsync zero-byte producer without pause", event_no_data_put_out);

        auto pause_receive_data = std::make_shared<std::string>();
        GetObjectAsyncInput pause_receive_input(options.bucket, base_key);
        auto pause_receive_out = AwaitOutcome<GetObjectAsyncOutput>(
                [&](const OutcomeCallback<GetObjectAsyncOutput>& cb) {
                    client.getObjectAsync(
                            pause_receive_input,
                            MakePauseThenReceiver(pause_receive_data, 2, 10), cb);
                });
        if (reporter.expectSuccess("getObjectAsync AsyncEvent PauseFor zero-byte consumer",
                                   pause_receive_out)) {
            if (*pause_receive_data != base_data) {
                reporter.fail("getObjectAsync AsyncEvent PauseFor zero-byte consumer data",
                              "downloaded data mismatch");
            } else {
                reporter.pass("getObjectAsync AsyncEvent PauseFor zero-byte consumer data");
                ExpectCrc64("getObjectAsync AsyncEvent PauseFor zero-byte consumer crc64",
                            pause_receive_out, base_data, reporter);
            }
        }

        auto minus_one_receive_data = std::make_shared<std::string>();
        GetObjectAsyncInput minus_one_receive_input(options.bucket, base_key);
        auto minus_one_receive_out = AwaitOutcome<GetObjectAsyncOutput>(
                [&](const OutcomeCallback<GetObjectAsyncOutput>& cb) {
                    client.getObjectAsync(
                            minus_one_receive_input, MakeMinusOneReceiver(minus_one_receive_data), cb);
                });
        if (reporter.expectSuccess("getObjectAsync short consumer(len-1)", minus_one_receive_out)) {
            if (*minus_one_receive_data != base_data) {
                reporter.fail("getObjectAsync short consumer(len-1) data",
                              "downloaded data mismatch");
            } else {
                reporter.pass("getObjectAsync short consumer(len-1) data");
                ExpectCrc64("getObjectAsync short consumer(len-1) crc64",
                            minus_one_receive_out, base_data, reporter);
            }
        }

        auto no_consume_data = std::make_shared<std::string>();
        GetObjectAsyncInput no_consume_input(options.bucket, base_key);
        auto no_consume_out = AwaitOutcome<GetObjectAsyncOutput>(
                [&](const OutcomeCallback<GetObjectAsyncOutput>& cb) {
                    client.getObjectAsync(no_consume_input, MakeNoConsumeReceiver(), cb);
                });
        reporter.expectFailure("getObjectAsync zero-byte consumer without pause", no_consume_out);
        if (!no_consume_data->empty()) {
            reporter.fail("getObjectAsync zero-byte consumer without pause data",
                          "consumer unexpectedly received data");
        }

        HeadObjectAsyncInput head_object(options.bucket, base_key);
        auto head_object_out = AwaitOutcome<HeadObjectAsyncOutput>(
                [&](const OutcomeCallback<HeadObjectAsyncOutput>& cb) {
                    client.headObjectAsync(head_object, cb);
                });
        reporter.expectSuccess("headObjectAsync", head_object_out);

        SetObjectMetaAsyncInput meta_input(options.bucket, base_key);
        meta_input.setMeta({{"async-example", "yes"}});
        auto meta_out = AwaitOutcome<SetObjectMetaAsyncOutput>(
                [&](const OutcomeCallback<SetObjectMetaAsyncOutput>& cb) {
                    client.setObjectMetaAsync(meta_input, cb);
                });
        reporter.expectSuccess("setObjectMetaAsync", meta_out);

        ListObjectsAsyncInput list_objects(options.bucket);
        list_objects.setPrefix(options.prefix);
        list_objects.setMaxKeys(100);
        auto list_objects_out = AwaitOutcome<ListObjectsAsyncOutput>(
                [&](const OutcomeCallback<ListObjectsAsyncOutput>& cb) {
                    client.listObjectsAsync(list_objects, cb);
                });
        reporter.expectSuccess("listObjectsAsync", list_objects_out);

        ListObjectsType2Input list_objects_v2(options.bucket);
        list_objects_v2.setPrefix(options.prefix);
        list_objects_v2.setMaxKeys(100);
        list_objects_v2.setListOnlyOnce(true);
        auto list_objects_v2_out = AwaitOutcome<ListObjectsType2Output>(
                [&](const OutcomeCallback<ListObjectsType2Output>& cb) {
                    client.listObjectsType2Async(list_objects_v2, cb);
                });
        reporter.expectSuccess("listObjectsType2Async", list_objects_v2_out);

        ListObjectVersionsAsyncInput list_versions(options.bucket);
        list_versions.setPrefix(options.prefix);
        list_versions.setMaxKeys(100);
        auto list_versions_out = AwaitOutcome<ListObjectVersionsAsyncOutput>(
                [&](const OutcomeCallback<ListObjectVersionsAsyncOutput>& cb) {
                    client.listObjectVersionsAsync(list_versions, cb);
                });
        reporter.expectSuccess("listObjectVersionsAsync", list_versions_out);

        const std::string get_file_path = JoinPath(options.workdir, "get-to-file.out");
        GetObjectToFileAsyncInput get_file_input(options.bucket, base_key, get_file_path);
        auto get_file_out = AwaitOutcome<GetObjectToFileAsyncOutput>(
                [&](const OutcomeCallback<GetObjectToFileAsyncOutput>& cb) {
                    client.getObjectToFileAsync(get_file_input, cb);
                });
        if (reporter.expectSuccess("getObjectToFileAsync", get_file_out)) {
            if (ReadFile(get_file_path) != base_data) {
                reporter.fail("getObjectToFileAsync data", "downloaded file mismatch");
            } else {
                reporter.pass("getObjectToFileAsync data");
                ExpectCrc64("getObjectToFileAsync crc64", get_file_out, base_data, reporter);
            }
        }

        const std::string fd_download_path = JoinPath(options.workdir, "get-fd-range.out");
        FileRange download_range;
        download_range.path = fd_download_path;
        download_range.length = base_data.size();
        auto get_fd_out = AwaitTransfer<GetObjectAsyncOutput>(
                [&](const TransferCallback<GetObjectAsyncOutput>& cb) {
                    client.getObjectToFdRangeAsync(get_input, download_range, PosixTransferOptions(), cb);
                });
        if (reporter.expectSuccess("getObjectToFdRangeAsync", get_fd_out.outcome)) {
            reporter.expectTransfer("getObjectToFdRangeAsync", get_fd_out.transfer, base_data.size());
            ExpectCrc64("getObjectToFdRangeAsync crc64", get_fd_out.outcome, base_data, reporter);
        }

        const std::string file_data = "async example put from file";
        const std::string upload_file_path = JoinPath(options.workdir, "upload-file.txt");
        WriteFile(upload_file_path, file_data);
        const std::string file_key = Key(options, "put-from-file.txt");
        cleanup_keys.push_back(file_key);
        PutObjectFromFileAsyncInput put_file_input(options.bucket, file_key, upload_file_path);
        auto put_file_out = AwaitOutcome<PutObjectFromFileAsyncOutput>(
                [&](const OutcomeCallback<PutObjectFromFileAsyncOutput>& cb) {
                    client.putObjectFromFileAsync(put_file_input, cb);
                });
        if (reporter.expectSuccess("putObjectFromFileAsync", put_file_out)) {
            ExpectCrc64("putObjectFromFileAsync crc64", put_file_out, file_data, reporter);
        }

        const std::string fd_key = Key(options, "put-from-fd.txt");
        cleanup_keys.push_back(fd_key);
        int fd = ::open(upload_file_path.c_str(), O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error("open upload file failed: " + std::string(std::strerror(errno)));
        }
        FileRange upload_range;
        upload_range.fd = fd;
        upload_range.length = file_data.size();
        upload_range.owns_fd = true;
        PutObjectAsyncInput put_fd_input(options.bucket, fd_key, TransferEncoding::ContentLength);
        auto put_fd_out = AwaitTransfer<PutObjectAsyncOutput>(
                [&](const TransferCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectFromFdRangeAsync(
                            put_fd_input, upload_range, PosixTransferOptions(), cb);
                });
        if (reporter.expectSuccess("putObjectFromFdRangeAsync", put_fd_out.outcome)) {
            reporter.expectTransfer("putObjectFromFdRangeAsync", put_fd_out.transfer, file_data.size());
            ExpectCrc64("putObjectFromFdRangeAsync crc64", put_fd_out.outcome, file_data, reporter);
        }

        if (iouring_local_probe_ok) {
            const std::string iouring_file_data =
                    RepeatData("async-iouring-file-transfer-e2e-", 512 * 1024 + 31);
            const std::string iouring_upload_path =
                    JoinPath(options.workdir, "iouring-e2e-upload.bin");
            const std::string iouring_download_path =
                    JoinPath(options.workdir, "iouring-e2e-download.bin");
            WriteFile(iouring_upload_path, iouring_file_data);

            const std::string iouring_key = Key(options, "iouring-file-transfer.bin");
            cleanup_keys.push_back(iouring_key);
            int iouring_upload_fd = ::open(iouring_upload_path.c_str(), O_RDONLY);
            if (iouring_upload_fd < 0) {
                throw std::runtime_error("open iouring upload file failed: " +
                                         std::string(std::strerror(errno)));
            }
            FileRange iouring_upload_range;
            iouring_upload_range.fd = iouring_upload_fd;
            iouring_upload_range.length = iouring_file_data.size();
            iouring_upload_range.owns_fd = true;
            auto iouring_upload_hook = std::make_shared<DelayedIoUringHook>(10);
            PutObjectAsyncInput iouring_put_input(
                    options.bucket, iouring_key, TransferEncoding::ContentLength);
            auto iouring_put_out = AwaitTransfer<PutObjectAsyncOutput>(
                    [&](const TransferCallback<PutObjectAsyncOutput>& cb) {
                        client.putObjectFromFdRangeAsync(
                                iouring_put_input, iouring_upload_range,
                                IoUringTransferOptions(iouring_upload_hook), cb);
                    });
            if (reporter.expectSuccess("putObjectFromFdRangeAsync(io_uring)",
                                       iouring_put_out.outcome)) {
                reporter.expectTransfer(
                        "putObjectFromFdRangeAsync(io_uring)", iouring_put_out.transfer,
                        iouring_file_data.size());
                ExpectCrc64("putObjectFromFdRangeAsync(io_uring) crc64",
                            iouring_put_out.outcome, iouring_file_data, reporter);
                if (iouring_upload_hook->afterReads() == 0) {
                    reporter.fail("putObjectFromFdRangeAsync(io_uring) hook",
                                  "io_uring read hook was not observed");
                } else {
                    reporter.pass("putObjectFromFdRangeAsync(io_uring) hook");
                }
                VerifyObjectData(client, options, iouring_key, iouring_file_data, reporter,
                                 "putObjectFromFdRangeAsync(io_uring) verify");
            }

            FileRange iouring_download_range;
            iouring_download_range.path = iouring_download_path;
            iouring_download_range.length = iouring_file_data.size();
            auto iouring_download_hook = std::make_shared<DelayedIoUringHook>(10);
            GetObjectAsyncInput iouring_get_input(options.bucket, iouring_key);
            auto iouring_get_out = AwaitTransfer<GetObjectAsyncOutput>(
                    [&](const TransferCallback<GetObjectAsyncOutput>& cb) {
                        client.getObjectToFdRangeAsync(
                                iouring_get_input, iouring_download_range,
                                IoUringTransferOptions(iouring_download_hook), cb);
                    });
            if (reporter.expectSuccess("getObjectToFdRangeAsync(io_uring)",
                                       iouring_get_out.outcome)) {
                reporter.expectTransfer(
                        "getObjectToFdRangeAsync(io_uring)", iouring_get_out.transfer,
                        iouring_file_data.size());
                ExpectCrc64("getObjectToFdRangeAsync(io_uring) crc64",
                            iouring_get_out.outcome, iouring_file_data, reporter);
                if (iouring_download_hook->afterWrites() == 0) {
                    reporter.fail("getObjectToFdRangeAsync(io_uring) hook",
                                  "io_uring write hook was not observed");
                } else {
                    reporter.pass("getObjectToFdRangeAsync(io_uring) hook");
                }
                if (ReadFile(iouring_download_path) != iouring_file_data) {
                    reporter.fail("getObjectToFdRangeAsync(io_uring) data",
                                  "downloaded file mismatch");
                } else {
                    reporter.pass("getObjectToFdRangeAsync(io_uring) data");
                }
            }
        } else {
            reporter.skip("io_uring file transfer e2e", "local io_uring probe did not pass");
        }

        const std::string copy_key = Key(options, "copy.txt");
        cleanup_keys.push_back(copy_key);
        CopyObjectAsyncInput copy_input(options.bucket, copy_key);
        copy_input.setSourceBucket(options.bucket);
        copy_input.setSourceKey(base_key);
        auto copy_out = AwaitOutcome<CopyObjectAsyncOutput>(
                [&](const OutcomeCallback<CopyObjectAsyncOutput>& cb) {
                    client.copyObjectAsync(copy_input, cb);
                });
        reporter.expectSuccess("copyObjectAsync", copy_out);

        if (is_hns) {
            SetObjectTimeInput time_input(options.bucket, base_key);
            timespec ts;
            ts.tv_sec = std::time(nullptr);
            ts.tv_nsec = 0;
            time_input.setModifyTimestamp(ts);
            auto time_out = AwaitOutcome<SetObjectTimeOutput>(
                    [&](const OutcomeCallback<SetObjectTimeOutput>& cb) {
                        client.setObjectTimeAsync(time_input, cb);
                    });
            reporter.expectSuccess("setObjectTimeAsync", time_out);
        } else {
            reporter.skip("setObjectTimeAsync", "bucket is not HNS");
        }

        GetFileStatusAsyncInput stat_input(options.bucket, base_key);
        auto stat_out = AwaitOutcome<GetFileStatusAsyncOutput>(
                [&](const OutcomeCallback<GetFileStatusAsyncOutput>& cb) {
                    client.getFileStatusAsync(stat_input, cb);
                });
        reporter.expectSuccess("getFileStatusAsync", stat_out);

        const std::string append_key = Key(options, "append-or-modify.txt");
        cleanup_keys.push_back(append_key);
        if (is_hns) {
            PutObjectAsyncInput seed_append(options.bucket, append_key, TransferEncoding::ContentLength);
            seed_append.setContentLength(16);
            (void)AwaitOutcome<PutObjectAsyncOutput>(
                    [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                        client.putObjectAsync(seed_append, MakeSender("0000000000000000"), cb);
                    });
        }
        const std::string append_data = "append-data";
        AppendObjectAsyncInput append_input(options.bucket, append_key, 0, TransferEncoding::ContentLength);
        append_input.setContentLength(static_cast<int64_t>(append_data.size()));
        auto append_out = AwaitOutcome<AppendObjectAsyncOutput>(
                [&](const OutcomeCallback<AppendObjectAsyncOutput>& cb) {
                    client.appendObjectAsync(append_input, MakeSender(append_data), cb);
                });
        reporter.expectSuccess("appendObjectAsync", append_out);

        const std::string append_fd_key = Key(options, "append-fd.txt");
        cleanup_keys.push_back(append_fd_key);
        if (is_hns) {
            PutObjectAsyncInput seed_append_fd(
                    options.bucket, append_fd_key, TransferEncoding::ContentLength);
            seed_append_fd.setContentLength(16);
            (void)AwaitOutcome<PutObjectAsyncOutput>(
                    [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                        client.putObjectAsync(seed_append_fd, MakeSender("1111111111111111"), cb);
                    });
        }
        int append_fd = ::open(upload_file_path.c_str(), O_RDONLY);
        if (append_fd < 0) {
            throw std::runtime_error("open append file failed: " + std::string(std::strerror(errno)));
        }
        FileRange append_range;
        append_range.fd = append_fd;
        append_range.length = file_data.size();
        append_range.owns_fd = true;
        AppendObjectAsyncInput append_fd_input(
                options.bucket, append_fd_key, 0, TransferEncoding::ContentLength);
        auto append_fd_out = AwaitTransfer<AppendObjectAsyncOutput>(
                [&](const TransferCallback<AppendObjectAsyncOutput>& cb) {
                    client.appendObjectFromFdRangeAsync(
                            append_fd_input, append_range, PosixTransferOptions(), cb);
                });
        if (reporter.expectSuccess("appendObjectFromFdRangeAsync", append_fd_out.outcome)) {
            reporter.expectTransfer(
                    "appendObjectFromFdRangeAsync", append_fd_out.transfer, file_data.size());
        }

        if (is_hns) {
            const std::string modify_key = Key(options, "modify.txt");
            cleanup_keys.push_back(modify_key);
            PutObjectAsyncInput seed_modify(options.bucket, modify_key, TransferEncoding::ContentLength);
            seed_modify.setContentLength(32);
            (void)AwaitOutcome<PutObjectAsyncOutput>(
                    [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                        client.putObjectAsync(seed_modify, MakeSender(std::string(32, 'm')), cb);
                    });

            const std::string modify_data = "MOD";
            ModifyObjectAsyncInput modify_input(
                    options.bucket, modify_key, 0, TransferEncoding::ContentLength);
            modify_input.setContentLength(static_cast<int64_t>(modify_data.size()));
            auto modify_out = AwaitOutcome<ModifyObjectAsyncOutput>(
                    [&](const OutcomeCallback<ModifyObjectAsyncOutput>& cb) {
                        client.modifyObjectAsync(modify_input, MakeSender(modify_data), cb);
                    });
            reporter.expectSuccess("modifyObjectAsync", modify_out);

            int modify_fd = ::open(upload_file_path.c_str(), O_RDONLY);
            if (modify_fd < 0) {
                throw std::runtime_error("open modify file failed: " + std::string(std::strerror(errno)));
            }
            FileRange modify_range;
            modify_range.fd = modify_fd;
            modify_range.length = file_data.size();
            modify_range.owns_fd = true;
            ModifyObjectAsyncInput modify_fd_input(
                    options.bucket, modify_key, 0, TransferEncoding::ContentLength);
            auto modify_fd_out = AwaitTransfer<ModifyObjectAsyncOutput>(
                    [&](const TransferCallback<ModifyObjectAsyncOutput>& cb) {
                        client.modifyObjectFromFdRangeAsync(
                                modify_fd_input, modify_range, PosixTransferOptions(), cb);
                    });
            if (reporter.expectSuccess("modifyObjectFromFdRangeAsync", modify_fd_out.outcome)) {
                reporter.expectTransfer(
                        "modifyObjectFromFdRangeAsync", modify_fd_out.transfer, file_data.size());
            }

            auto modify_file_out = AwaitTransfer<ModifyObjectAsyncOutput>(
                    [&](const TransferCallback<ModifyObjectAsyncOutput>& cb) {
                        client.modifyObjectFromFileAsync(
                                modify_fd_input, upload_file_path, PosixTransferOptions(), cb);
                    });
            if (reporter.expectSuccess("modifyObjectFromFileAsync", modify_file_out.outcome)) {
                reporter.expectTransfer(
                        "modifyObjectFromFileAsync", modify_file_out.transfer, file_data.size());
            }

            const std::string symlink_key = Key(options, "symlink");
            cleanup_keys.push_back(symlink_key);
            PutSymlinkAsyncInput put_symlink(options.bucket, symlink_key);
            put_symlink.setSymlinkTargetKey(base_key);
            put_symlink.setSymlinkTargetBucket(options.bucket);
            auto put_symlink_out = AwaitOutcome<PutSymlinkAsyncOutput>(
                    [&](const OutcomeCallback<PutSymlinkAsyncOutput>& cb) {
                        client.putSymlinkAsync(put_symlink, cb);
                    });
            reporter.expectSuccess("putSymlinkAsync", put_symlink_out);

            GetSymlinkAsyncInput get_symlink(options.bucket, symlink_key);
            auto get_symlink_out = AwaitOutcome<GetSymlinkAsyncOutput>(
                    [&](const OutcomeCallback<GetSymlinkAsyncOutput>& cb) {
                        client.getSymlinkAsync(get_symlink, cb);
                    });
            if (reporter.expectSuccess("getSymlinkAsync", get_symlink_out) &&
                get_symlink_out.result().getSymlinkTargetKey() != base_key) {
                reporter.fail("getSymlinkAsync target", "target key mismatch");
            }

            const std::string rename_src = Key(options, "rename-src.txt");
            const std::string rename_dst = Key(options, "rename-dst.txt");
            cleanup_keys.push_back(rename_src);
            cleanup_keys.push_back(rename_dst);
            PutObjectAsyncInput put_rename(options.bucket, rename_src, TransferEncoding::ContentLength);
            put_rename.setContentLength(6);
            (void)AwaitOutcome<PutObjectAsyncOutput>(
                    [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                        client.putObjectAsync(put_rename, MakeSender("rename"), cb);
                    });
            RenameObjectAsyncInput rename_input(options.bucket, rename_src);
            rename_input.setNewKey(rename_dst);
            auto rename_out = AwaitOutcome<RenameObjectAsyncOutput>(
                    [&](const OutcomeCallback<RenameObjectAsyncOutput>& cb) {
                        client.renameObjectAsync(rename_input, cb);
                    });
            reporter.expectSuccess("renameObjectAsync", rename_out);
        } else {
            reporter.skip("modifyObjectAsync", "bucket is not HNS");
            reporter.skip("modifyObjectFromFdRangeAsync", "bucket is not HNS");
            reporter.skip("modifyObjectFromFileAsync", "bucket is not HNS");
            reporter.skip("putSymlinkAsync", "bucket is not HNS");
            reporter.skip("getSymlinkAsync", "bucket is not HNS");
            reporter.skip("renameObjectAsync", "bucket is not HNS");
        }

        const std::string part_data_1 = RepeatData("part-one-", 6 * 1024 * 1024);
        const std::string part_data_2 = RepeatData("part-two-", 6 * 1024 * 1024);
        const std::string part_file_path = JoinPath(options.workdir, "part-file.bin");
        WriteFile(part_file_path, part_data_2);

        const std::string multipart_key = Key(options, "multipart.txt");
        cleanup_keys.push_back(multipart_key);
        CreateMultipartUploadAsyncInput create_multipart(options.bucket, multipart_key);
        auto create_multipart_out = AwaitOutcome<CreateMultipartUploadAsyncOutput>(
                [&](const OutcomeCallback<CreateMultipartUploadAsyncOutput>& cb) {
                    client.createMultipartUploadAsync(create_multipart, cb);
                });
        if (reporter.expectSuccess("createMultipartUploadAsync", create_multipart_out)) {
            const std::string upload_id = create_multipart_out.result().getUploadId();
            UploadPartAsyncInput part1(
                    options.bucket, multipart_key, 1, upload_id, TransferEncoding::ContentLength);
            part1.setContentLength(static_cast<int64_t>(part_data_1.size()));
            auto part1_out = AwaitOutcome<UploadPartAsyncOutput>(
                    [&](const OutcomeCallback<UploadPartAsyncOutput>& cb) {
                        client.uploadPartAsync(part1, MakeSender(part_data_1), cb);
                    });
            if (reporter.expectSuccess("uploadPartAsync", part1_out)) {
                ExpectCrc64("uploadPartAsync crc64", part1_out, part_data_1, reporter);
            }

            int part_fd = ::open(part_file_path.c_str(), O_RDONLY);
            if (part_fd < 0) {
                throw std::runtime_error("open part file failed: " + std::string(std::strerror(errno)));
            }
            FileRange part_range;
            part_range.fd = part_fd;
            part_range.length = part_data_2.size();
            part_range.owns_fd = true;
            UploadPartAsyncInput part2(
                    options.bucket, multipart_key, 2, upload_id, TransferEncoding::ContentLength);
            auto part2_out = AwaitTransfer<UploadPartAsyncOutput>(
                    [&](const TransferCallback<UploadPartAsyncOutput>& cb) {
                        client.uploadPartFromFdRangeAsync(part2, part_range, PosixTransferOptions(), cb);
                    });
            if (reporter.expectSuccess("uploadPartFromFdRangeAsync", part2_out.outcome)) {
                reporter.expectTransfer(
                        "uploadPartFromFdRangeAsync", part2_out.transfer, part_data_2.size());
                ExpectCrc64("uploadPartFromFdRangeAsync crc64", part2_out.outcome, part_data_2, reporter);
            }

            if (part1_out.isSuccess() && part2_out.outcome.isSuccess()) {
                std::vector<UploadedPartV2> parts;
                parts.emplace_back(1, part1_out.result().getEtag());
                parts.emplace_back(2, part2_out.outcome.result().getEtag());
                CompleteMultipartUploadAsyncInput complete(options.bucket, multipart_key, upload_id);
                complete.setParts(parts);
                auto complete_out = AwaitOutcome<CompleteMultipartUploadAsyncOutput>(
                        [&](const OutcomeCallback<CompleteMultipartUploadAsyncOutput>& cb) {
                            client.completeMultipartUploadAsync(complete, cb);
                        });
                reporter.expectSuccess("completeMultipartUploadAsync", complete_out);
            }
        }

        const std::string abort_key = Key(options, "multipart-abort.txt");
        CreateMultipartUploadAsyncInput create_abort(options.bucket, abort_key);
        auto create_abort_out = AwaitOutcome<CreateMultipartUploadAsyncOutput>(
                [&](const OutcomeCallback<CreateMultipartUploadAsyncOutput>& cb) {
                    client.createMultipartUploadAsync(create_abort, cb);
                });
        if (reporter.expectSuccess("createMultipartUploadAsync(for abort)", create_abort_out)) {
            AbortMultipartUploadInput abort_input(
                    options.bucket, abort_key, create_abort_out.result().getUploadId());
            auto abort_out = AwaitOutcome<AbortMultipartUploadOutput>(
                    [&](const OutcomeCallback<AbortMultipartUploadOutput>& cb) {
                        client.abortMultipartUploadAsync(abort_input, cb);
                    });
            reporter.expectSuccess("abortMultipartUploadAsync", abort_out);
        }

        const std::string copy_part_key = Key(options, "multipart-copy.txt");
        cleanup_keys.push_back(copy_part_key);
        CreateMultipartUploadAsyncInput create_copy_part(options.bucket, copy_part_key);
        auto create_copy_part_out = AwaitOutcome<CreateMultipartUploadAsyncOutput>(
                [&](const OutcomeCallback<CreateMultipartUploadAsyncOutput>& cb) {
                    client.createMultipartUploadAsync(create_copy_part, cb);
                });
        if (reporter.expectSuccess("createMultipartUploadAsync(for copy part)", create_copy_part_out)) {
            const std::string upload_id = create_copy_part_out.result().getUploadId();
            UploadPartCopyAsyncInput copy_part(
                    options.bucket, copy_part_key, options.bucket, multipart_key, 1, upload_id);
            auto copy_part_out = AwaitOutcome<UploadPartCopyAsyncOutput>(
                    [&](const OutcomeCallback<UploadPartCopyAsyncOutput>& cb) {
                        client.uploadPartCopyUploadAsync(copy_part, cb);
                    });
            reporter.expectSuccess("uploadPartCopyUploadAsync", copy_part_out);
            if (copy_part_out.isSuccess()) {
                CompleteMultipartUploadAsyncInput complete_copy(options.bucket, copy_part_key, upload_id);
                complete_copy.setParts({UploadedPartV2(1, copy_part_out.result().getEtag())});
                auto complete_copy_out = AwaitOutcome<CompleteMultipartUploadAsyncOutput>(
                        [&](const OutcomeCallback<CompleteMultipartUploadAsyncOutput>& cb) {
                            client.completeMultipartUploadAsync(complete_copy, cb);
                        });
                reporter.expectSuccess("completeMultipartUploadAsync(copy part)", complete_copy_out);
            }
        }

        const std::string delete_key = Key(options, "delete.txt");
        PutObjectAsyncInput put_delete(options.bucket, delete_key, TransferEncoding::ContentLength);
        put_delete.setContentLength(6);
        (void)AwaitOutcome<PutObjectAsyncOutput>(
                [&](const OutcomeCallback<PutObjectAsyncOutput>& cb) {
                    client.putObjectAsync(put_delete, MakeSender("delete"), cb);
                });
        DeleteObjectAsyncInput delete_input(options.bucket, delete_key);
        auto delete_out = AwaitOutcome<DeleteObjectAsyncOutput>(
                [&](const OutcomeCallback<DeleteObjectAsyncOutput>& cb) {
                    client.deleteObjectAsync(delete_input, cb);
                });
        reporter.expectSuccess("deleteObjectAsync", delete_out);

        CleanupKeys(client, options, cleanup_keys);
        client.close();

        if (options.test_create_bucket) {
            TosAsyncClient create_client(options.region, options.access_key, options.secret_key,
                                         MakeConfig(options));
            TestCreateBucketOnExistingBucket(create_client, options, reporter);
            create_client.close();
        }

        CloseTosAsyncClient();
        reporter.summary();
        return reporter.failures() == 0 ? 0 : 2;
    } catch (const std::exception& ex) {
        reporter.fail("exception", ex.what());
        reporter.summary();
        try {
            InitializeTosAsyncClient();
        } catch (...) {
        }
        return 1;
    }
}
