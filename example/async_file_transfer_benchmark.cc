#include "TosAsyncClient.h"
#include "logger/logger.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace VolcengineTos;

namespace {

struct BenchOptions {
    std::string bucket;
    std::string region;
    std::string endpoint;
    std::string access_key;
    std::string secret_key;
    std::string prefix = "async-file-transfer-bench";
    std::string workdir = "/tmp/ve-tos-cpp-sdk-file-transfer-bench";
    std::string backends = "posix,mmap,iouring";
    std::size_t size_mb = 500;
    std::uint64_t size_bytes = 500ULL * 1024ULL * 1024ULL;
    int threads = 1;
    std::size_t chunk_mb = 8;
    std::size_t io_depth = 8;
    bool keep_objects = false;
};

struct OperationResult {
    bool ok = false;
    std::string error;
    std::uint64_t bytes = 0;
    FileIoBackend backend = FileIoBackend::Auto;
    double seconds = 0.0;
};

struct CpuUsage {
    double user_seconds = 0.0;
    double system_seconds = 0.0;
};

struct PhaseStats {
    bool ok = true;
    std::string first_error;
    double wall_seconds = 0.0;
    double min_seconds = 0.0;
    double max_seconds = 0.0;
    double avg_seconds = 0.0;
    double cpu_user_seconds = 0.0;
    double cpu_system_seconds = 0.0;
    std::uint64_t total_bytes = 0;
    std::vector<OperationResult> results;
};

const char* Env(const char* name) {
    return std::getenv(name);
}

std::string GetEnv(const char* name, const std::string& default_value) {
    const char* value = Env(name);
    return value != nullptr ? std::string(value) : default_value;
}

std::size_t GetSizeEnv(const char* name, std::size_t default_value) {
    const char* value = Env(name);
    return value != nullptr ? static_cast<std::size_t>(std::strtoull(value, nullptr, 10)) : default_value;
}

int GetIntEnv(const char* name, int default_value) {
    const char* value = Env(name);
    return value != nullptr ? std::atoi(value) : default_value;
}

bool GetBoolEnv(const char* name, bool default_value) {
    const char* value = Env(name);
    if (value == nullptr) {
        return default_value;
    }
    std::string text(value);
    std::transform(text.begin(), text.end(), text.begin(), [](char ch) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    });
    return text == "1" || text == "true" || text == "yes" || text == "on";
}

void PrintUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0
              << " [--size-mb N|--size-kb N] [--threads N] [--backends posix,mmap,iouring]"
                 " [--workdir PATH] [--chunk-mb N] [--io-depth N] [--keep-objects]\n"
              << "\nRequired environment:\n"
              << "  TOS_E2E_BUCKET, TOS_E2E_REGION, TOS_E2E_ENDPOINT, TOS_E2E_AK, TOS_E2E_SK\n"
              << "\nOptional environment:\n"
              << "  TOS_E2E_PREFIX, TOS_FILE_TRANSFER_BENCH_SIZE_MB, TOS_FILE_TRANSFER_BENCH_THREADS,\n"
              << "  TOS_FILE_TRANSFER_BENCH_SIZE_KB,\n"
              << "  TOS_FILE_TRANSFER_BENCH_BACKENDS, TOS_FILE_TRANSFER_BENCH_WORKDIR,\n"
              << "  TOS_FILE_TRANSFER_BENCH_CHUNK_MB, TOS_FILE_TRANSFER_BENCH_IO_DEPTH,\n"
              << "  TOS_FILE_TRANSFER_BENCH_KEEP_OBJECTS\n";
}

std::string RequireEnv(const char* name) {
    const char* value = Env(name);
    if (value == nullptr || std::string(value).empty()) {
        std::ostringstream ss;
        ss << "missing required environment: " << name;
        throw std::runtime_error(ss.str());
    }
    return std::string(value);
}

BenchOptions ParseOptions(int argc, char** argv) {
    BenchOptions options;
    options.bucket = GetEnv("TOS_E2E_BUCKET", "");
    options.region = GetEnv("TOS_E2E_REGION", "");
    options.endpoint = GetEnv("TOS_E2E_ENDPOINT", "");
    options.access_key = GetEnv("TOS_E2E_AK", "");
    options.secret_key = GetEnv("TOS_E2E_SK", "");
    options.prefix = GetEnv("TOS_E2E_PREFIX", options.prefix);
    options.workdir = GetEnv("TOS_FILE_TRANSFER_BENCH_WORKDIR", options.workdir);
    options.backends = GetEnv("TOS_FILE_TRANSFER_BENCH_BACKENDS", options.backends);
    options.size_mb = GetSizeEnv("TOS_FILE_TRANSFER_BENCH_SIZE_MB", options.size_mb);
    options.size_bytes = static_cast<std::uint64_t>(options.size_mb) * 1024ULL * 1024ULL;
    if (Env("TOS_FILE_TRANSFER_BENCH_SIZE_KB") != nullptr) {
        options.size_bytes = GetSizeEnv("TOS_FILE_TRANSFER_BENCH_SIZE_KB", 0) * 1024ULL;
    }
    options.threads = GetIntEnv("TOS_FILE_TRANSFER_BENCH_THREADS", options.threads);
    options.chunk_mb = GetSizeEnv("TOS_FILE_TRANSFER_BENCH_CHUNK_MB", options.chunk_mb);
    options.io_depth = GetSizeEnv("TOS_FILE_TRANSFER_BENCH_IO_DEPTH", options.io_depth);
    options.keep_objects = GetBoolEnv("TOS_FILE_TRANSFER_BENCH_KEEP_OBJECTS", options.keep_objects);

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            std::exit(0);
        }
        if (i + 1 >= argc && arg != "--keep-objects") {
            throw std::runtime_error("missing value for argument: " + arg);
        }
        if (arg == "--size-mb") {
            options.size_mb = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
            options.size_bytes = static_cast<std::uint64_t>(options.size_mb) * 1024ULL * 1024ULL;
        } else if (arg == "--size-kb") {
            options.size_bytes =
                    static_cast<std::uint64_t>(std::strtoull(argv[++i], nullptr, 10)) * 1024ULL;
        } else if (arg == "--threads") {
            options.threads = std::atoi(argv[++i]);
        } else if (arg == "--backends") {
            options.backends = argv[++i];
        } else if (arg == "--workdir") {
            options.workdir = argv[++i];
        } else if (arg == "--chunk-mb") {
            options.chunk_mb = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--io-depth") {
            options.io_depth = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (arg == "--keep-objects") {
            options.keep_objects = true;
        } else {
            throw std::runtime_error("unknown argument: " + arg);
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
    if (options.size_bytes == 0) {
        throw std::runtime_error("object size must be greater than 0");
    }
    if (options.threads <= 0) {
        throw std::runtime_error("--threads must be greater than 0");
    }
    if (options.chunk_mb == 0) {
        throw std::runtime_error("--chunk-mb must be greater than 0");
    }
    if (options.io_depth == 0) {
        throw std::runtime_error("--io-depth must be greater than 0");
    }
    return options;
}

std::vector<std::string> SplitList(const std::string& text) {
    std::vector<std::string> values;
    std::string current;
    std::istringstream input(text);
    while (std::getline(input, current, ',')) {
        current.erase(std::remove_if(current.begin(), current.end(), [](char ch) {
                          return std::isspace(static_cast<unsigned char>(ch)) != 0;
                      }),
                      current.end());
        if (!current.empty()) {
            values.push_back(current);
        }
    }
    return values;
}

FileIoBackend ParseBackend(const std::string& value) {
    std::string lower(value);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](char ch) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    });
    if (lower == "auto") {
        return FileIoBackend::Auto;
    }
    if (lower == "posix") {
        return FileIoBackend::Posix;
    }
    if (lower == "mmap") {
        return FileIoBackend::Mmap;
    }
    if (lower == "iouring" || lower == "io_uring") {
        return FileIoBackend::IoUring;
    }
    throw std::runtime_error("unknown backend: " + value);
}

std::vector<FileIoBackend> ParseBackends(const std::string& text) {
    std::vector<FileIoBackend> backends;
    std::vector<std::string> values = SplitList(text);
    for (const auto& value : values) {
        backends.push_back(ParseBackend(value));
    }
    if (backends.empty()) {
        throw std::runtime_error("no backend specified");
    }
    return backends;
}

void EnsureDirectory(const std::string& path) {
    if (::mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        throw std::runtime_error("mkdir failed for " + path + ": " + std::strerror(errno));
    }
}

std::uint64_t FileSize(const std::string& path) {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<std::uint64_t>(st.st_size);
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

void CreateDataFile(const std::string& path, std::uint64_t bytes) {
    if (FileSize(path) == bytes) {
        return;
    }

    int fd = ::open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) {
        throw std::runtime_error("open failed for " + path + ": " + std::strerror(errno));
    }

    try {
        const std::size_t block_size = 1024 * 1024;
        std::vector<char> block(block_size);
        for (std::size_t i = 0; i < block.size(); ++i) {
            block[i] = static_cast<char>((i * 131 + 17) & 0xff);
        }

        std::uint64_t remaining = bytes;
        while (remaining > 0) {
            std::size_t n = static_cast<std::size_t>(std::min<std::uint64_t>(remaining, block.size()));
            WriteAll(fd, block.data(), n);
            remaining -= n;
        }
        if (::close(fd) != 0) {
            throw std::runtime_error(std::string("close failed: ") + std::strerror(errno));
        }
    } catch (...) {
        ::close(fd);
        throw;
    }
}

std::uint64_t MegabytesToBytes(std::size_t mb) {
    return static_cast<std::uint64_t>(mb) * 1024ULL * 1024ULL;
}

double SecondsSince(const std::chrono::steady_clock::time_point& start,
                    const std::chrono::steady_clock::time_point& end) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

double TimevalToSeconds(const timeval& value) {
    return static_cast<double>(value.tv_sec) + static_cast<double>(value.tv_usec) / 1000000.0;
}

CpuUsage ReadCpuUsage() {
    rusage usage;
    if (::getrusage(RUSAGE_SELF, &usage) != 0) {
        return CpuUsage();
    }
    CpuUsage result;
    result.user_seconds = TimevalToSeconds(usage.ru_utime);
    result.system_seconds = TimevalToSeconds(usage.ru_stime);
    return result;
}

double CpuTotalSeconds(const PhaseStats& stats) {
    return stats.cpu_user_seconds + stats.cpu_system_seconds;
}

double CpuSecondsPerGiB(const PhaseStats& stats) {
    const double gib = static_cast<double>(stats.total_bytes) / 1024.0 / 1024.0 / 1024.0;
    return gib > 0.0 ? CpuTotalSeconds(stats) / gib : 0.0;
}

std::string JoinPath(const std::string& dir, const std::string& leaf) {
    if (!dir.empty() && dir[dir.size() - 1] == '/') {
        return dir + leaf;
    }
    return dir + "/" + leaf;
}

std::string ObjectKey(const BenchOptions& options, FileIoBackend backend, int worker) {
    std::ostringstream ss;
    ss << options.prefix << "/" << FileIoBackendName(backend) << "/worker-" << worker << "-" << ::getpid()
       << ".bin";
    return ss.str();
}

std::string DownloadPath(const BenchOptions& options, FileIoBackend backend, int worker) {
    std::ostringstream ss;
    ss << "download-" << FileIoBackendName(backend) << "-worker-" << worker << ".bin";
    return JoinPath(options.workdir, ss.str());
}

FileTransferOptions MakeTransferOptions(FileIoBackend backend, const BenchOptions& options) {
    FileTransferOptions transfer_options;
    transfer_options.backend = backend;
    transfer_options.chunk_size = static_cast<std::size_t>(MegabytesToBytes(options.chunk_mb));
    transfer_options.io_depth = options.io_depth;
    transfer_options.close_fd_on_done = true;
    transfer_options.truncate_file_on_done = true;
    return transfer_options;
}

std::string OutcomeError(const TosError& error) {
    return error.String();
}

OperationResult UploadOne(const TosAsyncClient& client, const BenchOptions& options,
                          const std::string& source_path, FileIoBackend backend, int worker,
                          std::uint64_t bytes, const std::string& key) {
    OperationResult result;
    int fd = ::open(source_path.c_str(), O_RDONLY);
    if (fd < 0) {
        result.error = "open source failed: " + std::string(std::strerror(errno));
        return result;
    }

    FileRange range;
    range.fd = fd;
    range.offset = 0;
    range.length = bytes;
    range.owns_fd = true;

    PutObjectAsyncInput input(options.bucket, key, TransferEncoding::ContentLength);
    input.setContentLength(static_cast<int64_t>(bytes));

    std::promise<OperationResult> promise;
    std::future<OperationResult> future = promise.get_future();
    auto started = std::chrono::steady_clock::now();
    client.putObjectFromFdRangeAsync(input, range, MakeTransferOptions(backend, options),
                                     [&promise, started](Outcome<TosError, PutObjectAsyncOutput>& outcome,
                                                         FileTransferResult transfer_result) mutable {
                                         OperationResult done;
                                         done.seconds =
                                                 SecondsSince(started, std::chrono::steady_clock::now());
                                         done.bytes = transfer_result.bytes;
                                         done.backend = transfer_result.backend;
                                         if (!outcome.isSuccess()) {
                                             done.error = OutcomeError(outcome.error());
                                         } else if (!transfer_result.status.ok) {
                                             done.error = transfer_result.status.message;
                                         } else {
                                             done.ok = true;
                                         }
                                         promise.set_value(done);
                                     });
    return future.get();
}

OperationResult DownloadOne(const TosAsyncClient& client, const BenchOptions& options,
                            FileIoBackend backend, int worker, std::uint64_t bytes,
                            const std::string& key) {
    FileRange range;
    range.path = DownloadPath(options, backend, worker);
    range.offset = 0;
    range.length = bytes;

    GetObjectAsyncInput input(options.bucket, key);

    std::promise<OperationResult> promise;
    std::future<OperationResult> future = promise.get_future();
    auto started = std::chrono::steady_clock::now();
    client.getObjectToFdRangeAsync(input, range, MakeTransferOptions(backend, options),
                                   [&promise, started](Outcome<TosError, GetObjectAsyncOutput>& outcome,
                                                       FileTransferResult transfer_result) mutable {
                                       OperationResult done;
                                       done.seconds =
                                               SecondsSince(started, std::chrono::steady_clock::now());
                                       done.bytes = transfer_result.bytes;
                                       done.backend = transfer_result.backend;
                                       if (!outcome.isSuccess()) {
                                           done.error = OutcomeError(outcome.error());
                                       } else if (!transfer_result.status.ok) {
                                           done.error = transfer_result.status.message;
                                       } else {
                                           done.ok = true;
                                       }
                                       promise.set_value(done);
                                   });
    OperationResult result = future.get();
    if (result.ok) {
        std::uint64_t downloaded_size = FileSize(range.path);
        if (downloaded_size != bytes) {
            result.ok = false;
            std::ostringstream ss;
            ss << "downloaded file size mismatch, expected=" << bytes << ", actual=" << downloaded_size;
            result.error = ss.str();
        }
    }
    return result;
}

bool DeleteOne(const TosAsyncClient& client, const BenchOptions& options, const std::string& key) {
    DeleteObjectAsyncInput input(options.bucket, key);
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    client.deleteObjectAsync(
            input, [&promise](Outcome<TosError, DeleteObjectAsyncOutput>& outcome) mutable {
                promise.set_value(outcome.isSuccess());
            });
    return future.get();
}

PhaseStats RunPhase(const std::string& phase, const TosAsyncClient& client, const BenchOptions& options,
                    const std::string& source_path, FileIoBackend backend, std::uint64_t bytes,
                    const std::vector<std::string>& keys) {
    PhaseStats stats;
    stats.results.resize(static_cast<std::size_t>(options.threads));

    CpuUsage cpu_started = ReadCpuUsage();
    auto started = std::chrono::steady_clock::now();
    std::vector<std::thread> workers;
    for (int i = 0; i < options.threads; ++i) {
        workers.emplace_back([&, i]() {
            if (phase == "upload") {
                stats.results[static_cast<std::size_t>(i)] = UploadOne(
                        client, options, source_path, backend, i, bytes, keys[static_cast<std::size_t>(i)]);
            } else {
                stats.results[static_cast<std::size_t>(i)] =
                        DownloadOne(client, options, backend, i, bytes, keys[static_cast<std::size_t>(i)]);
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }
    auto ended = std::chrono::steady_clock::now();
    CpuUsage cpu_ended = ReadCpuUsage();
    stats.wall_seconds = SecondsSince(started, ended);
    stats.cpu_user_seconds = cpu_ended.user_seconds - cpu_started.user_seconds;
    stats.cpu_system_seconds = cpu_ended.system_seconds - cpu_started.system_seconds;

    double total_seconds = 0.0;
    bool first_duration = true;
    for (const auto& result : stats.results) {
        if (!result.ok) {
            stats.ok = false;
            if (stats.first_error.empty()) {
                stats.first_error = result.error;
            }
        }
        stats.total_bytes += result.bytes;
        total_seconds += result.seconds;
        if (first_duration) {
            stats.min_seconds = result.seconds;
            stats.max_seconds = result.seconds;
            first_duration = false;
        } else {
            stats.min_seconds = std::min(stats.min_seconds, result.seconds);
            stats.max_seconds = std::max(stats.max_seconds, result.seconds);
        }
    }
    if (!stats.results.empty()) {
        stats.avg_seconds = total_seconds / static_cast<double>(stats.results.size());
    }
    return stats;
}

void PrintPhaseStats(FileIoBackend requested, const std::string& phase, const PhaseStats& stats,
                     const BenchOptions& options) {
    double mib = static_cast<double>(stats.total_bytes) / 1024.0 / 1024.0;
    double throughput = stats.wall_seconds > 0.0 ? mib / stats.wall_seconds : 0.0;
    std::cout << std::left << std::setw(10) << FileIoBackendName(requested) << std::setw(10) << phase
              << std::right << std::setw(8) << options.threads << std::setw(12) << std::fixed
              << std::setprecision(2) << stats.wall_seconds << std::setw(14) << throughput << std::setw(12)
              << stats.min_seconds << std::setw(12) << stats.avg_seconds << std::setw(12)
              << stats.max_seconds << std::setw(12) << CpuTotalSeconds(stats) << std::setw(14)
              << CpuSecondsPerGiB(stats) << "  " << (stats.ok ? "ok" : stats.first_error) << std::endl;
}

void PrintConfig(const BenchOptions& options, const std::vector<FileIoBackend>& backends) {
    std::cout << "bucket=" << options.bucket << ", region=" << options.region
              << ", endpoint=" << options.endpoint << std::endl;
    std::cout << "size_bytes=" << options.size_bytes << " ("
              << static_cast<double>(options.size_bytes) / 1024.0 / 1024.0
              << " MiB), threads=" << options.threads << ", chunk_mb=" << options.chunk_mb
              << ", io_depth=" << options.io_depth << std::endl;
    std::cout << "backends=";
    for (std::size_t i = 0; i < backends.size(); ++i) {
        if (i != 0) {
            std::cout << ",";
        }
        std::cout << FileIoBackendName(backends[i]);
    }
    std::cout << std::endl;
}

ClientConfig MakeClientConfig(const BenchOptions& options) {
    ClientConfig config;
    config.endPoint = options.endpoint;
    config.enableCRC = false;
    config.detail_log_ = false;
    config.maxRetryCount = 3;
    config.connectionTimeout = 30000;
    config.socketTimeout = 600000;
    config.requestTimeout = 0;
    config.maxConnections = std::max(25, options.threads * 8);
    config.event_thread_count_ = std::max(2, options.threads);
    config.fileTransferChunkSize = static_cast<std::size_t>(MegabytesToBytes(options.chunk_mb));
    config.fileTransferIoDepth = options.io_depth;
    return config;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        BenchOptions options = ParseOptions(argc, argv);
        std::vector<FileIoBackend> backends = ParseBackends(options.backends);
        std::uint64_t bytes = options.size_bytes;

        EnsureDirectory(options.workdir);
        std::string source_path = JoinPath(options.workdir, "source.bin");
        std::cout << "preparing source file: " << source_path << std::endl;
        CreateDataFile(source_path, bytes);

        PrintConfig(options, backends);
        std::cout << std::left << std::setw(10) << "backend" << std::setw(10) << "phase" << std::right
                  << std::setw(8) << "threads" << std::setw(12) << "wall_s" << std::setw(14) << "MiB/s"
                  << std::setw(12) << "min_s" << std::setw(12) << "avg_s" << std::setw(12) << "max_s"
                  << std::setw(12) << "cpu_s" << std::setw(14) << "cpu_s/GiB" << "  status"
                  << std::endl;

        Logger::getInstance().setAsyncLogLevel(ERROR);
        InitializeTosAsyncClient();
        bool all_ok = true;
        {
            TosAsyncClient client(options.region, options.access_key, options.secret_key,
                                  MakeClientConfig(options));
            for (FileIoBackend backend : backends) {
                std::vector<std::string> keys;
                for (int i = 0; i < options.threads; ++i) {
                    keys.push_back(ObjectKey(options, backend, i));
                }

                PhaseStats upload = RunPhase("upload", client, options, source_path, backend, bytes, keys);
                PrintPhaseStats(backend, "upload", upload, options);
                all_ok = all_ok && upload.ok;

                if (upload.ok) {
                    PhaseStats download =
                            RunPhase("download", client, options, source_path, backend, bytes, keys);
                    PrintPhaseStats(backend, "download", download, options);
                    all_ok = all_ok && download.ok;
                }

                if (!options.keep_objects) {
                    for (const auto& key : keys) {
                        if (!DeleteOne(client, options, key)) {
                            std::cerr << "warning: deleteObject failed for " << key << std::endl;
                        }
                    }
                }
            }
            client.close();
        }
        CloseTosAsyncClient();
        return all_ok ? 0 : 2;
    } catch (const std::exception& ex) {
        std::cerr << "benchmark failed: " << ex.what() << std::endl;
        PrintUsage(argv[0]);
        return 1;
    }
}
