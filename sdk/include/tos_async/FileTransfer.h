#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "tos_async/AsyncDataSource.h"

namespace VolcengineTos {

enum class FileTransferDirection {
    Upload,
    Download,
};

enum class FileIoBackend {
    Auto,
    Mmap,
    IoUring,
    Posix,
};

constexpr std::uint64_t kUnknownFileRangeLength = std::numeric_limits<std::uint64_t>::max();

struct FileTransferStatus {
    bool ok{true};
    std::string message;
    int err_no{0};

    static FileTransferStatus Ok() { return {}; }

    static FileTransferStatus Failed(std::string message, int err_no = 0) {
        FileTransferStatus status;
        status.ok = false;
        status.message = std::move(message);
        status.err_no = err_no;
        return status;
    }
};

struct ByteView {
    ByteView() = default;
    ByteView(const char* data, std::size_t size) : data(data), size(size) {}

    const char* data{nullptr};
    std::size_t size{0};
};

struct MemoryRange {
    char* data{nullptr};
    std::uint64_t offset{0};
    std::uint64_t length{0};
    // Optional owner keeps externally supplied memory alive until async file IO is finished.
    std::shared_ptr<void> owner;
};

struct FileRange {
    int fd{-1};
    std::string path;
    std::uint64_t offset{0};
    std::uint64_t length{0};
    bool owns_fd{false};
};

struct FileTransferContext {
    FileTransferDirection direction{FileTransferDirection::Upload};
    FileIoBackend backend{FileIoBackend::Auto};
    FileRange range;
    std::string operation;
};

struct FileIoEvent {
    std::uint64_t seq{0};
    std::uint64_t file_offset{0};
    std::size_t request_len{0};
    FileTransferDirection direction{FileTransferDirection::Upload};
    FileIoBackend backend{FileIoBackend::Auto};
    ByteView data;
};

struct FileIoResult {
    FileTransferStatus status;
    std::size_t bytes{0};
    ByteView data;
};

struct FileTransferResult {
    FileTransferResult() = default;
    explicit FileTransferResult(FileTransferStatus status) : status(std::move(status)) {}

    FileTransferStatus status;
    std::uint64_t bytes{0};
    std::uint64_t crc64{0};
    FileIoBackend backend{FileIoBackend::Auto};
};

class FileTransferHook {
 public:
    virtual ~FileTransferHook() = default;
    virtual FileTransferStatus OnTransferBegin(const FileTransferContext&) {
        return FileTransferStatus::Ok();
    }
    virtual FileTransferStatus OnBeforeRead(const FileIoEvent&) { return FileTransferStatus::Ok(); }
    virtual FileTransferStatus OnAfterRead(const FileIoEvent&, const FileIoResult&) {
        return FileTransferStatus::Ok();
    }
    virtual FileTransferStatus OnBeforeWrite(const FileIoEvent&) { return FileTransferStatus::Ok(); }
    virtual FileTransferStatus OnAfterWrite(const FileIoEvent&, const FileIoResult&) {
        return FileTransferStatus::Ok();
    }
    virtual void OnTransferFinish(const FileTransferContext&, const FileTransferResult&) {}
};

class FileTransferControl {
 public:
    using CancelCallback = std::function<void(const std::string&)>;

    void Cancel(std::string reason);
    void AttachCancelCallback(CancelCallback callback);
    bool canceled() const;
    std::string cancel_reason() const;

 private:
    mutable std::mutex mu_;
    CancelCallback cancel_callback_;
    bool canceled_{false};
    std::string cancel_reason_;
};

struct FileTransferOptions {
    FileIoBackend backend{FileIoBackend::Auto};
    std::size_t chunk_size{1024 * 1024};
    std::size_t io_depth{8};
    std::uint64_t auto_upload_iouring_max_bytes{0};
    bool close_fd_on_done{false};
    bool truncate_file_on_done{false};
    std::shared_ptr<FileTransferControl> control;
    std::vector<std::shared_ptr<FileTransferHook>> hooks;
};

class FileRangeAsyncDataSource : public AsyncDataSource {
 public:
    ~FileRangeAsyncDataSource() override = default;

    virtual FileTransferResult result() const = 0;
    virtual FileTransferResult Finish(FileTransferStatus network_status) = 0;
};

using FileRangeAsyncDataSourcePtr = std::shared_ptr<FileRangeAsyncDataSource>;

class FileRangeDownloadSink {
 public:
    virtual ~FileRangeDownloadSink() = default;

    virtual std::size_t Write(char* data, std::size_t len, AsyncEvent* ev) = 0;
    virtual void Cancel(std::string reason) = 0;
    virtual FileTransferResult result() const = 0;
    virtual FileTransferResult Finish(FileTransferStatus network_status) = 0;
    virtual void FinishAsync(FileTransferStatus network_status,
                             std::function<void(FileTransferResult)> done) {
        if (done) {
            done(Finish(std::move(network_status)));
        }
    }
};

using FileRangeDownloadSinkPtr = std::shared_ptr<FileRangeDownloadSink>;

FileRangeAsyncDataSourcePtr CreateFileRangeAsyncDataSource(
    FileRange range, FileTransferOptions options = FileTransferOptions());

FileRangeDownloadSinkPtr CreateFileRangeDownloadSink(FileRange range,
                                                     FileTransferOptions options = FileTransferOptions());

FileRangeDownloadSinkPtr CreateMemoryFileRangeDownloadSink(
    FileRange range, MemoryRange memory, FileTransferOptions options = FileTransferOptions());

const char* FileIoBackendName(FileIoBackend backend);

}  // namespace VolcengineTos
