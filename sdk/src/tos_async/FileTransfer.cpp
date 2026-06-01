#include "tos_async/FileTransfer.h"

#ifndef VOLCENGINE_TOS_ASYNC_HAS_IO_URING
#define VOLCENGINE_TOS_ASYNC_HAS_IO_URING 0
#endif

#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
#include <liburing.h>
#endif

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <limits>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

namespace VolcengineTos {
namespace {

constexpr std::size_t kDefaultPageSize = 4096;
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
constexpr std::size_t kDefaultSharedIoUringQueueDepth = 1024;
constexpr std::size_t kMaxSharedIoUringQueueDepth = 4096;
constexpr std::size_t kDefaultSharedIoUringShardCount = 1;
constexpr std::size_t kMaxSharedIoUringShardCount = 32;
constexpr std::uint64_t kDefaultAutoUploadIoUringMaxBytes = 1024 * 1024;
#endif

bool IsUnknownLength(std::uint64_t length) {
    return length == kUnknownFileRangeLength;
}

std::string ErrnoMessage(const char* op, int err) {
    std::ostringstream oss;
    oss << op << " failed: " << strerror(err);
    return oss.str();
}

std::size_t PageSize() {
    const long value = sysconf(_SC_PAGESIZE);
    return value > 0 ? static_cast<std::size_t>(value) : kDefaultPageSize;
}

std::uint64_t AlignUp(std::uint64_t value, std::uint64_t align) {
    if (align == 0) {
        return value;
    }
    const std::uint64_t rem = value % align;
    if (rem == 0) {
        return value;
    }
    return value + (align - rem);
}

bool AddOverflow(std::uint64_t a, std::uint64_t b, std::uint64_t* out) {
    if (out == nullptr) {
        return true;
    }
    if (a > std::numeric_limits<std::uint64_t>::max() - b) {
        return true;
    }
    *out = a + b;
    return false;
}

FileTransferStatus CallBeginHooks(const FileTransferContext& ctx,
                                  const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (!hook) {
            continue;
        }
        FileTransferStatus status = hook->OnTransferBegin(ctx);
        if (!status.ok) {
            return status;
        }
    }
    return FileTransferStatus::Ok();
}

FileTransferStatus CallBeforeReadHooks(const FileIoEvent& ev,
                                       const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (!hook) {
            continue;
        }
        FileTransferStatus status = hook->OnBeforeRead(ev);
        if (!status.ok) {
            return status;
        }
    }
    return FileTransferStatus::Ok();
}

FileTransferStatus CallAfterReadHooks(const FileIoEvent& ev, const FileIoResult& result,
                                      const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (!hook) {
            continue;
        }
        FileTransferStatus status = hook->OnAfterRead(ev, result);
        if (!status.ok) {
            return status;
        }
    }
    return FileTransferStatus::Ok();
}

FileTransferStatus CallBeforeWriteHooks(const FileIoEvent& ev,
                                        const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (!hook) {
            continue;
        }
        FileTransferStatus status = hook->OnBeforeWrite(ev);
        if (!status.ok) {
            return status;
        }
    }
    return FileTransferStatus::Ok();
}

FileTransferStatus CallAfterWriteHooks(const FileIoEvent& ev, const FileIoResult& result,
                                       const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (!hook) {
            continue;
        }
        FileTransferStatus status = hook->OnAfterWrite(ev, result);
        if (!status.ok) {
            return status;
        }
    }
    return FileTransferStatus::Ok();
}

void CallFinishHooks(const FileTransferContext& ctx, const FileTransferResult& result,
                     const std::vector<std::shared_ptr<FileTransferHook>>& hooks) {
    for (const auto& hook : hooks) {
        if (hook) {
            hook->OnTransferFinish(ctx, result);
        }
    }
}

int OpenPathIfNeeded(FileRange* range, bool write) {
    if (range == nullptr) {
        errno = EINVAL;
        return -1;
    }
    if (range->fd >= 0) {
        return range->fd;
    }
    if (range->path.empty()) {
        errno = EINVAL;
        return -1;
    }
    const int flags = write ? (O_CREAT | O_RDWR | O_CLOEXEC) : (O_RDONLY | O_CLOEXEC);
    const int fd = open(range->path.c_str(), flags, 0644);
    if (fd < 0) {
        return -1;
    }
    range->fd = fd;
    range->owns_fd = true;
    return fd;
}

FileTransferStatus TruncateToCompletedBytes(int fd, std::uint64_t offset, std::uint64_t bytes) {
    if (fd < 0) {
        return FileTransferStatus::Failed("truncate target fd is invalid", EBADF);
    }
    std::uint64_t end = 0;
    if (AddOverflow(offset, bytes, &end) ||
        end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
        return FileTransferStatus::Failed("file range download truncate size overflows off_t");
    }
    if (ftruncate(fd, static_cast<off_t>(end)) != 0) {
        return FileTransferStatus::Failed(ErrnoMessage("ftruncate", errno), errno);
    }
    return FileTransferStatus::Ok();
}

class BaseFileTransfer {
 public:
    BaseFileTransfer(FileRange range, FileTransferOptions options, FileTransferDirection direction,
                     FileIoBackend backend, std::string operation)
        : range_(std::move(range)),
          options_(std::move(options)),
          direction_(direction),
          backend_(backend),
          operation_(std::move(operation)) {
        result_.backend = backend_;
    }

    virtual ~BaseFileTransfer() {
        FinishHooksOnce();
        CloseFdIfOwned();
    }

    FileTransferResult result() const {
        std::lock_guard<std::mutex> lock(mu_);
        return result_;
    }

 protected:
    FileTransferResult FinishTransfer(FileTransferStatus status) {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!status.ok && result_.status.ok) {
                result_.status = std::move(status);
            }
        }
        FinishHooksOnce();
        return BaseFileTransfer::result();
    }

    FileTransferContext Context() const {
        std::lock_guard<std::mutex> lock(mu_);
        return ContextLocked();
    }

    bool StartIoLocked(std::string* reason) {
        if (active_io_) {
            if (reason != nullptr) {
                *reason = "concurrent SDK file range io callback is not supported";
            }
            return false;
        }
        active_io_ = true;
        return true;
    }

    void FinishIoLocked() { active_io_ = false; }

    FileTransferContext ContextLocked() const {
        FileTransferContext ctx;
        ctx.direction = direction_;
        ctx.backend = backend_;
        ctx.range = range_;
        ctx.operation = operation_;
        return ctx;
    }

    void FailLocked(FileTransferStatus status) {
        if (status.ok) {
            return;
        }
        result_.status = std::move(status);
    }

    bool FailedLocked() const { return !result_.status.ok; }

    void FinishHooksOnce() {
        FileTransferContext ctx;
        FileTransferResult result;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (finish_hooks_called_) {
                return;
            }
            finish_hooks_called_ = true;
            ctx = ContextLocked();
            result = result_;
        }
        CallFinishHooks(ctx, result, options_.hooks);
    }

    void CloseFdIfOwned() {
        if ((range_.owns_fd || options_.close_fd_on_done) && range_.fd >= 0) {
            close(range_.fd);
            range_.fd = -1;
        }
    }

    mutable std::mutex mu_;
    FileRange range_;
    FileTransferOptions options_;
    FileTransferDirection direction_;
    FileIoBackend backend_;
    std::string operation_;
    FileTransferResult result_;
    bool finish_hooks_called_{false};
    bool active_io_{false};
};

class UnsupportedFileRangeSource final : public FileRangeAsyncDataSource {
 public:
    explicit UnsupportedFileRangeSource(std::string reason) {
        result_.status = FileTransferStatus::Failed(std::move(reason));
    }

    std::size_t Read(char*, std::size_t, AsyncEvent* ev) override {
        if (ev != nullptr) {
            ev->Fail(result_.status.message);
        }
        return 0;
    }

    FileTransferResult result() const override { return result_; }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        if (!network_status.ok && result_.status.ok) {
            result_.status = std::move(network_status);
        }
        return result_;
    }

 private:
    FileTransferResult result_;
};

class UnsupportedDownloadSink final : public FileRangeDownloadSink {
 public:
    explicit UnsupportedDownloadSink(std::string reason) {
        result_.status = FileTransferStatus::Failed(std::move(reason));
    }

    std::size_t Write(char*, std::size_t, AsyncEvent* ev) override {
        if (ev != nullptr) {
            ev->Fail(result_.status.message);
        }
        return 0;
    }

    void Cancel(std::string reason) override {
        result_.status = FileTransferStatus::Failed(std::move(reason));
    }

    FileTransferResult result() const override { return result_; }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        if (!network_status.ok && result_.status.ok) {
            result_.status = std::move(network_status);
        }
        return result_;
    }

 private:
    FileTransferResult result_;
};

class MmapFileRangeSource final : public FileRangeAsyncDataSource, public BaseFileTransfer {
 public:
    MmapFileRangeSource(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Upload,
                           FileIoBackend::Mmap, "mmap file range upload") {
        Init();
    }

    ~MmapFileRangeSource() override {
        if (mapping_ != nullptr && mapping_ != MAP_FAILED) {
            munmap(mapping_, map_size_);
            mapping_ = nullptr;
        }
    }

    std::size_t Read(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        FileIoEvent ev_before;
        std::size_t request = 0;
        std::uint64_t source_offset = 0;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (FailedLocked()) {
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (delivered_ >= range_.length) {
                return 0;
            }
            std::string active_error;
            if (!StartIoLocked(&active_error)) {
                FailLocked(FileTransferStatus::Failed(active_error));
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            request = static_cast<std::size_t>(
                std::min<std::uint64_t>(static_cast<std::uint64_t>(len), range_.length - delivered_));
            source_offset = delivered_;
            ev_before.seq = ++seq_;
            ev_before.file_offset = range_.offset + source_offset;
            ev_before.request_len = request;
            ev_before.direction = direction_;
            ev_before.backend = backend_;
        }

        FileTransferStatus hook_status = CallBeforeReadHooks(ev_before, options_.hooks);
        if (!hook_status.ok) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
            }
            return 0;
        }
        const char* src = static_cast<const char*>(mapping_) + map_delta_ + source_offset;
        memcpy(data, src, request);

        FileIoResult io_result;
        io_result.status = FileTransferStatus::Ok();
        io_result.bytes = request;
        io_result.data = {data, request};
        hook_status = CallAfterReadHooks(ev_before, io_result, options_.hooks);
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!hook_status.ok) {
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (FailedLocked()) {
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            delivered_ += request;
            result_.bytes += request;
            FinishIoLocked();
        }
        return request;
    }

    void Cancel() override {
        std::lock_guard<std::mutex> lock(mu_);
        FailLocked(FileTransferStatus::Failed("mmap file range upload canceled"));
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        return FinishTransfer(std::move(network_status));
    }

 private:
    void Init() {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, false) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (IsUnknownLength(range_.length)) {
                FailLocked(FileTransferStatus::Failed("mmap file range upload requires a known length"));
                return;
            }
            if (range_.length > 0) {
                struct stat st {};
                std::uint64_t end = 0;
                if (fstat(range_.fd, &st) != 0) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("fstat", errno), errno));
                    return;
                }
                if (AddOverflow(range_.offset, range_.length, &end) ||
                    end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
                    FailLocked(FileTransferStatus::Failed("mmap file range upload size overflows off_t"));
                    return;
                }
                if (S_ISREG(st.st_mode) && end > static_cast<std::uint64_t>(st.st_size)) {
                    FailLocked(FileTransferStatus::Failed("mmap file range upload exceeds file size"));
                    return;
                }
                const std::uint64_t page = PageSize();
                const std::uint64_t aligned_offset = range_.offset - (range_.offset % page);
                map_delta_ = static_cast<std::size_t>(range_.offset - aligned_offset);
                const std::uint64_t map_need = static_cast<std::uint64_t>(map_delta_) + range_.length;
                map_size_ = static_cast<std::size_t>(AlignUp(map_need, page));
                mapping_ = mmap(nullptr, map_size_, PROT_READ, MAP_SHARED, range_.fd,
                                static_cast<off_t>(aligned_offset));
                if (mapping_ == MAP_FAILED) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("mmap", errno), errno));
                    mapping_ = nullptr;
                    return;
                }
            }
        }

        FileTransferStatus status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    void* mapping_{nullptr};
    std::size_t map_size_{0};
    std::size_t map_delta_{0};
    std::uint64_t delivered_{0};
    std::uint64_t seq_{0};
};

class MmapFileRangeSink final : public FileRangeDownloadSink, public BaseFileTransfer {
 public:
    MmapFileRangeSink(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Download,
                           FileIoBackend::Mmap, "mmap file range download") {
        Init();
    }

    ~MmapFileRangeSink() override {
        if (mapping_ != nullptr && mapping_ != MAP_FAILED) {
            munmap(mapping_, map_size_);
            mapping_ = nullptr;
        }
    }

    std::size_t Write(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        FileIoEvent ev_before;
        std::uint64_t target_offset = 0;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (FailedLocked()) {
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (!IsUnknownLength(range_.length) && result_.bytes + len > range_.length) {
                FailLocked(FileTransferStatus::Failed(
                    "mmap file range download received more bytes than expected"));
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            std::string active_error;
            if (!StartIoLocked(&active_error)) {
                FailLocked(FileTransferStatus::Failed(active_error));
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            target_offset = result_.bytes;
            ev_before.seq = ++seq_;
            ev_before.file_offset = range_.offset + target_offset;
            ev_before.request_len = len;
            ev_before.direction = direction_;
            ev_before.backend = backend_;
            ev_before.data = {data, len};
        }

        FileTransferStatus hook_status = CallBeforeWriteHooks(ev_before, options_.hooks);
        if (!hook_status.ok) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
            }
            return 0;
        }
        char* dst = static_cast<char*>(mapping_) + map_delta_ + target_offset;
        memcpy(dst, data, len);

        FileIoResult io_result;
        io_result.status = FileTransferStatus::Ok();
        io_result.bytes = len;
        io_result.data = {data, len};
        hook_status = CallAfterWriteHooks(ev_before, io_result, options_.hooks);
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!hook_status.ok) {
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (FailedLocked()) {
                FinishIoLocked();
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            result_.bytes += len;
            FinishIoLocked();
        }
        return len;
    }

    void Cancel(std::string reason) override {
        std::lock_guard<std::mutex> lock(mu_);
        FailLocked(FileTransferStatus::Failed(std::move(reason)));
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!network_status.ok && result_.status.ok) {
                result_.status = std::move(network_status);
            }
            if (network_status.ok && result_.status.ok && options_.truncate_file_on_done) {
                FileTransferStatus truncate_status =
                    TruncateToCompletedBytes(range_.fd, range_.offset, result_.bytes);
                if (!truncate_status.ok) {
                    FailLocked(std::move(truncate_status));
                }
            }
        }
        FinishHooksOnce();
        return BaseFileTransfer::result();
    }

 private:
    void Init() {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, true) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (IsUnknownLength(range_.length)) {
                FailLocked(FileTransferStatus::Failed("mmap file range download requires a known length"));
                return;
            }
            if (range_.length > 0) {
                std::uint64_t end = 0;
                if (AddOverflow(range_.offset, range_.length, &end) ||
                    end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
                    FailLocked(FileTransferStatus::Failed("mmap file range download size overflows off_t"));
                    return;
                }
                if (ftruncate(range_.fd, static_cast<off_t>(end)) != 0) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("ftruncate", errno), errno));
                    return;
                }
                const std::uint64_t page = PageSize();
                const std::uint64_t aligned_offset = range_.offset - (range_.offset % page);
                map_delta_ = static_cast<std::size_t>(range_.offset - aligned_offset);
                const std::uint64_t map_need = static_cast<std::uint64_t>(map_delta_) + range_.length;
                map_size_ = static_cast<std::size_t>(AlignUp(map_need, page));
                mapping_ = mmap(nullptr, map_size_, PROT_WRITE, MAP_SHARED, range_.fd,
                                static_cast<off_t>(aligned_offset));
                if (mapping_ == MAP_FAILED) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("mmap", errno), errno));
                    mapping_ = nullptr;
                    return;
                }
            }
        }

        FileTransferStatus status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    void* mapping_{nullptr};
    std::size_t map_size_{0};
    std::size_t map_delta_{0};
    std::uint64_t seq_{0};
};

class PosixFileRangeSource final : public FileRangeAsyncDataSource, public BaseFileTransfer {
 public:
    PosixFileRangeSource(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Upload,
                           FileIoBackend::Posix, "posix file range upload") {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, false) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (IsUnknownLength(range_.length)) {
                FailLocked(FileTransferStatus::Failed("posix file range upload requires a known length"));
                return;
            }
        }
        FileTransferStatus status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    std::size_t Read(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        FileIoEvent ev_before;
        std::size_t request = 0;
        std::uint64_t source_offset = 0;
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (FailedLocked()) {
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            if (delivered_ >= range_.length) {
                return 0;
            }
            std::string active_error;
            if (!StartIoLocked(&active_error)) {
                FailLocked(FileTransferStatus::Failed(active_error));
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            request = static_cast<std::size_t>(
                std::min<std::uint64_t>(static_cast<std::uint64_t>(len), range_.length - delivered_));
            source_offset = delivered_;
            fd = range_.fd;
            ev_before.seq = ++seq_;
            ev_before.file_offset = range_.offset + source_offset;
            ev_before.request_len = request;
            ev_before.direction = direction_;
            ev_before.backend = backend_;
        }

        FileTransferStatus hook_status = CallBeforeReadHooks(ev_before, options_.hooks);
        if (!hook_status.ok) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }
        const ssize_t nread = pread(fd, data, request, static_cast<off_t>(range_.offset + source_offset));
        if (nread < 0) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("pread", errno), errno));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }
        if (nread == 0 && request > 0) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(FileTransferStatus::Failed("posix file range upload short read"));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }

        FileIoResult io_result;
        io_result.status = FileTransferStatus::Ok();
        io_result.bytes = static_cast<std::size_t>(nread);
        io_result.data = {data, static_cast<std::size_t>(nread)};
        hook_status = CallAfterReadHooks(ev_before, io_result, options_.hooks);
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!hook_status.ok) {
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            if (FailedLocked()) {
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            delivered_ += static_cast<std::uint64_t>(nread);
            result_.bytes += static_cast<std::uint64_t>(nread);
            FinishIoLocked();
        }
        return static_cast<std::size_t>(nread);
    }

    void Cancel() override {
        std::lock_guard<std::mutex> lock(mu_);
        FailLocked(FileTransferStatus::Failed("posix file range upload canceled"));
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        return FinishTransfer(std::move(network_status));
    }

 private:
    std::uint64_t delivered_{0};
    std::uint64_t seq_{0};
};

class PosixFileRangeSink final : public FileRangeDownloadSink, public BaseFileTransfer {
 public:
    PosixFileRangeSink(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Download,
                           FileIoBackend::Posix, "posix file range download") {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, true) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
        }
        FileTransferStatus status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    std::size_t Write(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        FileIoEvent ev_before;
        std::uint64_t target_offset = 0;
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (FailedLocked()) {
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            if (!IsUnknownLength(range_.length) && result_.bytes + len > range_.length) {
                FailLocked(FileTransferStatus::Failed(
                    "posix file range download received more bytes than expected"));
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            std::string active_error;
            if (!StartIoLocked(&active_error)) {
                FailLocked(FileTransferStatus::Failed(active_error));
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            target_offset = result_.bytes;
            fd = range_.fd;
            ev_before.seq = ++seq_;
            ev_before.file_offset = range_.offset + target_offset;
            ev_before.request_len = len;
            ev_before.direction = direction_;
            ev_before.backend = backend_;
            ev_before.data = {data, len};
        }

        FileTransferStatus hook_status = CallBeforeWriteHooks(ev_before, options_.hooks);
        if (!hook_status.ok) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }
        const ssize_t written = pwrite(fd, data, len, static_cast<off_t>(range_.offset + target_offset));
        if (written < 0) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("pwrite", errno), errno));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }
        if (static_cast<std::size_t>(written) != len) {
            {
                std::lock_guard<std::mutex> lock(mu_);
                FailLocked(FileTransferStatus::Failed("posix file range download short write"));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
            }
            return 0;
        }

        FileIoResult io_result;
        io_result.status = FileTransferStatus::Ok();
        io_result.bytes = static_cast<std::size_t>(written);
        io_result.data = {data, static_cast<std::size_t>(written)};
        hook_status = CallAfterWriteHooks(ev_before, io_result, options_.hooks);
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!hook_status.ok) {
                FailLocked(std::move(hook_status));
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            if (FailedLocked()) {
                FinishIoLocked();
                if (ev != nullptr) ev->Fail(result_.status.message);
                return 0;
            }
            result_.bytes += static_cast<std::uint64_t>(written);
            FinishIoLocked();
        }
        return static_cast<std::size_t>(written);
    }

    void Cancel(std::string reason) override {
        std::lock_guard<std::mutex> lock(mu_);
        FailLocked(FileTransferStatus::Failed(std::move(reason)));
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!network_status.ok && result_.status.ok) {
                result_.status = std::move(network_status);
            }
            if (network_status.ok && result_.status.ok && options_.truncate_file_on_done) {
                FileTransferStatus truncate_status =
                    TruncateToCompletedBytes(range_.fd, range_.offset, result_.bytes);
                if (!truncate_status.ok) {
                    FailLocked(std::move(truncate_status));
                }
            }
        }
        FinishHooksOnce();
        return BaseFileTransfer::result();
    }

 private:
    std::uint64_t seq_{0};
};

#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
std::size_t NormalizeIoDepth(std::size_t depth) { return depth > 0 ? depth : 1; }

std::size_t SharedIoUringQueueDepth() {
    const char* value = std::getenv("TOS_ASYNC_FILE_TRANSFER_IOURING_QUEUE_DEPTH");
    if (value == nullptr || value[0] == '\0') {
        return kDefaultSharedIoUringQueueDepth;
    }
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(value, &end, 10);
    if (end == value || parsed == 0) {
        return kDefaultSharedIoUringQueueDepth;
    }
    return std::min<std::size_t>(static_cast<std::size_t>(parsed), kMaxSharedIoUringQueueDepth);
}

std::size_t SharedIoUringShardCount() {
    const char* value = std::getenv("TOS_ASYNC_FILE_TRANSFER_IOURING_SHARDS");
    if (value == nullptr || value[0] == '\0') {
        return kDefaultSharedIoUringShardCount;
    }
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(value, &end, 10);
    if (end == value || parsed == 0) {
        return kDefaultSharedIoUringShardCount;
    }
    return std::min<std::size_t>(static_cast<std::size_t>(parsed), kMaxSharedIoUringShardCount);
}

std::uint64_t AutoUploadIoUringMaxBytes() {
    const char* value = std::getenv("TOS_ASYNC_FILE_TRANSFER_UPLOAD_IOURING_MAX_BYTES");
    if (value == nullptr || value[0] == '\0') {
        return kDefaultAutoUploadIoUringMaxBytes;
    }
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(value, &end, 10);
    if (end == value) {
        return kDefaultAutoUploadIoUringMaxBytes;
    }
    return static_cast<std::uint64_t>(parsed);
}

bool IoUringRuntimeAvailable() {
    static std::once_flag once;
    static bool available = false;
    std::call_once(once, [] {
        io_uring ring {};
        const int ret = io_uring_queue_init(1, &ring, 0);
        if (ret >= 0) {
            io_uring_queue_exit(&ring);
            available = true;
        }
    });
    return available;
}

FileTransferStatus IoUringStatus(const char* op, int err) {
    if (err < 0) {
        err = -err;
    }
    return FileTransferStatus::Failed(ErrnoMessage(op, err), err);
}

class IoUringExecutor {
 public:
    using Completion = std::function<void(int)>;
    struct Submission {
        Submission() = default;
        Submission(int fd_in, char* data_in, std::size_t len_in, off_t offset_in, bool write_in,
                   Completion completion_in)
            : fd(fd_in),
              data(data_in),
              len(len_in),
              offset(offset_in),
              write(write_in),
              completion(std::move(completion_in)) {}

        int fd{-1};
        char* data{nullptr};
        std::size_t len{0};
        off_t offset{0};
        bool write{false};
        Completion completion;
    };

    explicit IoUringExecutor(std::size_t queue_depth) : queue_depth_(NormalizeIoDepth(queue_depth)) {}

    ~IoUringExecutor() { Shutdown(); }

    IoUringExecutor(const IoUringExecutor&) = delete;
    IoUringExecutor& operator=(const IoUringExecutor&) = delete;

    FileTransferStatus Init() {
        std::lock_guard<std::mutex> lock(init_mu_);
        if (ring_ready_.load(std::memory_order_acquire)) {
            return FileTransferStatus::Ok();
        }
        if (init_attempted_) {
            return init_status_;
        }
        init_attempted_ = true;
        const int ret = io_uring_queue_init(static_cast<unsigned>(queue_depth_), &ring_, 0);
        if (ret < 0) {
            init_status_ = IoUringStatus("io_uring_queue_init", ret);
            return init_status_;
        }
        ring_ready_.store(true, std::memory_order_release);
        worker_ = std::thread([this]() { CompletionLoop(); });
        init_status_ = FileTransferStatus::Ok();
        return init_status_;
    }

    bool SubmitRead(int fd, char* data, std::size_t len, off_t offset, Completion completion,
                    std::string* error) {
        std::vector<Submission> submissions;
        submissions.push_back(Submission{fd, data, len, offset, false, std::move(completion)});
        return SubmitBatch(std::move(submissions), error);
    }

    bool SubmitWrite(int fd, char* data, std::size_t len, off_t offset, Completion completion,
                     std::string* error) {
        std::vector<Submission> submissions;
        submissions.push_back(Submission{fd, data, len, offset, true, std::move(completion)});
        return SubmitBatch(std::move(submissions), error);
    }

    bool SubmitBatch(std::vector<Submission> submissions, std::string* error) {
        if (submissions.empty()) {
            return true;
        }
        if (!ring_ready_.load(std::memory_order_acquire) || stopping_.load(std::memory_order_acquire)) {
            if (error != nullptr) {
                *error = "io_uring executor is not running";
            }
            return false;
        }
        for (const auto& submission : submissions) {
            if (submission.fd < 0 || submission.data == nullptr || submission.len == 0) {
                if (error != nullptr) {
                    *error = "io_uring request has invalid fd/buffer/length";
                }
                return false;
            }
        }

        std::lock_guard<std::mutex> ring_lock(ring_mu_);
        std::vector<Request*> prepared;
        prepared.reserve(submissions.size());
        for (auto& submission : submissions) {
            io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
            if (sqe == nullptr) {
                if (!SubmitPreparedLocked(&prepared, error)) {
                    return false;
                }
                sqe = io_uring_get_sqe(&ring_);
            }
            if (sqe == nullptr) {
                if (error != nullptr) {
                    *error = "io_uring submission queue is full";
                }
                return false;
            }

            std::unique_ptr<Request> request(new Request());
            request->completion = std::move(submission.completion);
            Request* raw = request.get();
            {
                std::lock_guard<std::mutex> request_lock(request_mu_);
                inflight_[raw] = std::move(request);
            }

            if (submission.write) {
                io_uring_prep_write(sqe, submission.fd, submission.data,
                                    static_cast<unsigned>(submission.len), submission.offset);
            } else {
                io_uring_prep_read(sqe, submission.fd, submission.data,
                                   static_cast<unsigned>(submission.len), submission.offset);
            }
            io_uring_sqe_set_data(sqe, raw);
            prepared.push_back(raw);
        }

        return SubmitPreparedLocked(&prepared, error);
    }

    bool IsWorkerThread() const {
        return worker_.joinable() && worker_.get_id() == std::this_thread::get_id();
    }

 private:
    struct Request {
        Completion completion;
    };

    bool SubmitPreparedLocked(std::vector<Request*>* prepared, std::string* error) {
        if (prepared == nullptr || prepared->empty()) {
            return true;
        }

        std::size_t submitted = 0;
        while (submitted < prepared->size()) {
            const int ret = io_uring_submit(&ring_);
            if (ret <= 0) {
                RemoveInflight(prepared->begin() + static_cast<std::ptrdiff_t>(submitted),
                               prepared->end());
                prepared->clear();
                if (error != nullptr) {
                    *error = ret < 0 ? ErrnoMessage("io_uring_submit", -ret)
                                     : "io_uring_submit submitted zero requests";
                }
                return false;
            }
            submitted += std::min<std::size_t>(static_cast<std::size_t>(ret),
                                               prepared->size() - submitted);
        }
        prepared->clear();
        return true;
    }

    template <typename It>
    void RemoveInflight(It begin, It end) {
        std::lock_guard<std::mutex> request_lock(request_mu_);
        for (auto it = begin; it != end; ++it) {
            inflight_.erase(*it);
        }
    }

    void Shutdown() {
        std::lock_guard<std::mutex> init_lock(init_mu_);
        if (!ring_ready_.load(std::memory_order_acquire)) {
            return;
        }
        stopping_.store(true, std::memory_order_release);
        Wake();
        if (worker_.joinable()) {
            worker_.join();
        }
        io_uring_queue_exit(&ring_);
        ring_ready_.store(false, std::memory_order_release);
    }

    void Wake() {
        std::lock_guard<std::mutex> lock(ring_mu_);
        io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        if (sqe == nullptr) {
            (void)io_uring_submit(&ring_);
            sqe = io_uring_get_sqe(&ring_);
        }
        if (sqe == nullptr) {
            return;
        }
        io_uring_prep_nop(sqe);
        io_uring_sqe_set_data(sqe, nullptr);
        (void)io_uring_submit(&ring_);
    }

    bool HasInflight() const {
        std::lock_guard<std::mutex> lock(request_mu_);
        return !inflight_.empty();
    }

    void CompletionLoop() {
        while (true) {
            if (stopping_.load(std::memory_order_acquire) && !HasInflight()) {
                return;
            }

            io_uring_cqe* cqe = nullptr;
            const int ret = io_uring_wait_cqe(&ring_, &cqe);
            if (ret < 0) {
                if (stopping_.load(std::memory_order_acquire)) {
                    continue;
                }
                continue;
            }
            Request* raw = static_cast<Request*>(io_uring_cqe_get_data(cqe));
            const int result = cqe->res;
            io_uring_cqe_seen(&ring_, cqe);
            if (raw == nullptr) {
                continue;
            }

            std::unique_ptr<Request> request;
            {
                std::lock_guard<std::mutex> lock(request_mu_);
                auto it = inflight_.find(raw);
                if (it != inflight_.end()) {
                    request = std::move(it->second);
                    inflight_.erase(it);
                }
            }
            if (request && request->completion) {
                request->completion(result);
            }
        }
    }

    const std::size_t queue_depth_;
    io_uring ring_{};
    std::atomic<bool> ring_ready_{false};
    std::atomic<bool> stopping_{false};
    std::thread worker_;
    mutable std::mutex init_mu_;
    mutable std::mutex ring_mu_;
    mutable std::mutex request_mu_;
    std::unordered_map<Request*, std::unique_ptr<Request>> inflight_;
    bool init_attempted_{false};
    FileTransferStatus init_status_;
};

std::shared_ptr<IoUringExecutor> SharedIoUringExecutor() {
    // FileTransfer can run many object transfers concurrently. Share a small set
    // of rings/workers so iouring does not create one ring/thread per transfer,
    // while sharding avoids a single ring mutex becoming the high-concurrency
    // bottleneck. Per-transfer backpressure is still options.io_depth.
    static std::vector<std::shared_ptr<IoUringExecutor>> executors = [] {
        std::vector<std::shared_ptr<IoUringExecutor>> shards;
        const std::size_t shard_count = SharedIoUringShardCount();
        shards.reserve(shard_count);
        for (std::size_t i = 0; i < shard_count; ++i) {
            shards.push_back(std::make_shared<IoUringExecutor>(SharedIoUringQueueDepth()));
        }
        return shards;
    }();
    static std::atomic<std::size_t> next_shard{0};
    const std::size_t shard = next_shard.fetch_add(1, std::memory_order_relaxed) % executors.size();
    return executors[shard];
}

class IoUringFileRangeSource final : public FileRangeAsyncDataSource, public BaseFileTransfer {
 public:
    IoUringFileRangeSource(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Upload,
                           FileIoBackend::IoUring, "io_uring file range upload"),
          executor_(SharedIoUringExecutor()) {
        Init();
    }

    std::size_t Read(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        PrimeReadQueue();
        std::lock_guard<std::mutex> lock(mu_);
        if (canceled_ || FailedLocked()) {
            if (ev != nullptr) {
                ev->Fail(result_.status.message);
            }
            return 0;
        }

        const std::size_t produced = DrainReadyLocked(data, len);
        if (produced > 0) {
            return produced;
        }
        if (delivered_bytes_ >= range_.length) {
            return 0;
        }
        waiting_event_ = ev;
        if (ev != nullptr) {
            ev->setReason("SDK io_uring upload waiting for local read");
            ev->Pause();
        }
        return 0;
    }

    void Cancel() override {
        AsyncEvent* event = nullptr;
        {
            std::lock_guard<std::mutex> lock(mu_);
            canceled_ = true;
            FailLocked(FileTransferStatus::Failed("io_uring file range upload canceled"));
            event = waiting_event_;
            waiting_event_ = nullptr;
            ready_reads_.clear();
            completed_reads_.clear();
        }
        if (event != nullptr) {
            event->Fail(BaseFileTransfer::result().status.message);
        }
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        return FinishTransfer(std::move(network_status));
    }

 private:
    struct PendingRead {
        std::uint64_t seq{0};
        std::uint64_t file_offset{0};
        std::vector<char> buffer;
        std::size_t read_pos{0};
        FileIoEvent event;
    };
    using ReadPtr = std::shared_ptr<PendingRead>;

    void Init() {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, false) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (IsUnknownLength(range_.length)) {
                FailLocked(FileTransferStatus::Failed("io_uring file range upload requires a known length"));
                return;
            }
            if (range_.length > 0) {
                struct stat st {};
                std::uint64_t end = 0;
                if (fstat(range_.fd, &st) != 0) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("fstat", errno), errno));
                    return;
                }
                if (AddOverflow(range_.offset, range_.length, &end) ||
                    end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
                    FailLocked(
                        FileTransferStatus::Failed("io_uring file range upload size overflows off_t"));
                    return;
                }
                if (S_ISREG(st.st_mode) && end > static_cast<std::uint64_t>(st.st_size)) {
                    FailLocked(FileTransferStatus::Failed("io_uring file range upload exceeds file size"));
                    return;
                }
            }
        }

        FileTransferStatus status = executor_->Init();
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
            return;
        }
        status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    std::size_t BufferedReadCountLocked() const {
        return inflight_reads_.size() + ready_reads_.size() + completed_reads_.size();
    }

    std::size_t DrainReadyLocked(char* data, std::size_t len) {
        std::size_t produced = 0;
        while (produced < len && !ready_reads_.empty()) {
            const ReadPtr& read = ready_reads_.front();
            if (!read || read->read_pos >= read->buffer.size()) {
                RecycleReadBufferLocked(read);
                ready_reads_.pop_front();
                continue;
            }
            const std::size_t available = read->buffer.size() - read->read_pos;
            const std::size_t to_copy = std::min(len - produced, available);
            memcpy(data + produced, read->buffer.data() + read->read_pos, to_copy);
            read->read_pos += to_copy;
            delivered_bytes_ += static_cast<std::uint64_t>(to_copy);
            result_.bytes += static_cast<std::uint64_t>(to_copy);
            produced += to_copy;
            if (read->read_pos == read->buffer.size()) {
                RecycleReadBufferLocked(read);
                ready_reads_.pop_front();
            }
        }
        return produced;
    }

    void MoveCompletedToReadyLocked() {
        while (true) {
            auto it = completed_reads_.find(next_deliver_seq_);
            if (it == completed_reads_.end()) {
                return;
            }
            ready_reads_.push_back(std::move(it->second));
            completed_reads_.erase(it);
            ++next_deliver_seq_;
        }
    }

    void PrimeReadQueue() {
        std::vector<IoUringExecutor::Submission> submissions;
        while (true) {
            ReadPtr read;
            {
                std::lock_guard<std::mutex> lock(mu_);
                if (canceled_ || FailedLocked() || submitted_bytes_ >= range_.length ||
                    inflight_reads_.size() >= NormalizeIoDepth(options_.io_depth) ||
                    BufferedReadCountLocked() >= NormalizeIoDepth(options_.io_depth) * 2) {
                    break;
                }
                const std::uint64_t remain = range_.length - submitted_bytes_;
                const std::size_t chunk = static_cast<std::size_t>(std::min<std::uint64_t>(
                    remain, options_.chunk_size > 0 ? options_.chunk_size : 1024 * 1024));
                read = std::make_shared<PendingRead>();
                read->seq = ++submit_seq_;
                read->file_offset = range_.offset + submitted_bytes_;
                read->buffer = NewReadBufferLocked(chunk);
                read->event.seq = read->seq;
                read->event.file_offset = read->file_offset;
                read->event.request_len = chunk;
                read->event.direction = direction_;
                read->event.backend = backend_;
                submitted_bytes_ += static_cast<std::uint64_t>(chunk);
                inflight_reads_[read->seq] = read;
            }

            FileTransferStatus hook_status = CallBeforeReadHooks(read->event, options_.hooks);
            if (!hook_status.ok) {
                FailReadQueue(std::move(hook_status));
                return;
            }

            submissions.push_back(IoUringExecutor::Submission{
                range_.fd, read->buffer.data(), read->buffer.size(), static_cast<off_t>(read->file_offset),
                false, [this, read](int result) { CompleteRead(read, result); }});
        }

        if (!submissions.empty()) {
            std::string error;
            if (!executor_->SubmitBatch(std::move(submissions), &error)) {
                FailReadQueue(FileTransferStatus::Failed(std::move(error)));
            }
        }
    }

    std::vector<char> NewReadBufferLocked(std::size_t chunk) {
        std::vector<char> buffer;
        if (!free_read_buffers_.empty()) {
            buffer = std::move(free_read_buffers_.front());
            free_read_buffers_.pop_front();
        }
        buffer.resize(chunk);
        return buffer;
    }

    void RecycleReadBufferLocked(const ReadPtr& read) {
        if (!read || read->buffer.empty()) {
            return;
        }
        const std::size_t max_cached_buffers = NormalizeIoDepth(options_.io_depth) * 2;
        const std::size_t target = options_.chunk_size > 0 ? options_.chunk_size : 1024 * 1024;
        if (free_read_buffers_.size() >= max_cached_buffers || read->buffer.capacity() > target * 2) {
            return;
        }
        read->buffer.clear();
        free_read_buffers_.push_back(std::move(read->buffer));
    }

    void FailReadQueue(FileTransferStatus status) {
        AsyncEvent* event = nullptr;
        {
            std::lock_guard<std::mutex> lock(mu_);
            inflight_reads_.clear();
            ready_reads_.clear();
            completed_reads_.clear();
            FailLocked(std::move(status));
            event = waiting_event_;
            waiting_event_ = nullptr;
        }
        if (event != nullptr) {
            event->Fail(BaseFileTransfer::result().status.message);
        }
    }

    void FailRead(const ReadPtr& read, FileTransferStatus status) {
        AsyncEvent* event = nullptr;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (read) {
                inflight_reads_.erase(read->seq);
            }
            FailLocked(std::move(status));
            event = waiting_event_;
            waiting_event_ = nullptr;
        }
        if (event != nullptr) {
            event->Fail(BaseFileTransfer::result().status.message);
        }
    }

    void CompleteRead(const ReadPtr& read, int io_result) {
        if (!read) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = inflight_reads_.find(read->seq);
            if (it == inflight_reads_.end()) {
                return;
            }
            if (canceled_) {
                inflight_reads_.erase(it);
                return;
            }
        }

        FileTransferStatus status = FileTransferStatus::Ok();
        if (io_result < 0) {
            status = IoUringStatus("io_uring read", io_result);
        } else if (static_cast<std::size_t>(io_result) != read->buffer.size()) {
            status = FileTransferStatus::Failed("io_uring file range upload short read");
        }
        if (status.ok) {
            read->buffer.resize(static_cast<std::size_t>(io_result));
            FileIoResult result;
            result.status = FileTransferStatus::Ok();
            result.bytes = read->buffer.size();
            result.data = {read->buffer.data(), read->buffer.size()};
            status = CallAfterReadHooks(read->event, result, options_.hooks);
        }

        AsyncEvent* event = nullptr;
        bool fail_event = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = inflight_reads_.find(read->seq);
            if (it == inflight_reads_.end()) {
                return;
            }
            inflight_reads_.erase(it);
            if (canceled_) {
                return;
            }
            if (!status.ok) {
                FailLocked(std::move(status));
                event = waiting_event_;
                waiting_event_ = nullptr;
                fail_event = true;
            } else {
                completed_reads_[read->seq] = read;
                MoveCompletedToReadyLocked();
                if (!ready_reads_.empty()) {
                    event = waiting_event_;
                    waiting_event_ = nullptr;
                }
            }
        }
        if (event != nullptr) {
            if (fail_event) {
                event->Fail("SDK io_uring upload local read failed");
            } else {
                event->Resume();
            }
        }
    }

    bool canceled_{false};
    std::uint64_t delivered_bytes_{0};
    std::uint64_t submitted_bytes_{0};
    std::uint64_t submit_seq_{0};
    std::uint64_t next_deliver_seq_{1};
    std::deque<ReadPtr> ready_reads_;
    std::unordered_map<std::uint64_t, ReadPtr> inflight_reads_;
    std::map<std::uint64_t, ReadPtr> completed_reads_;
    std::deque<std::vector<char>> free_read_buffers_;
    AsyncEvent* waiting_event_{nullptr};
    std::shared_ptr<IoUringExecutor> executor_;
};

class IoUringFileRangeSink final : public FileRangeDownloadSink, public BaseFileTransfer {
 public:
    IoUringFileRangeSink(FileRange range, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Download,
                           FileIoBackend::IoUring, "io_uring file range download"),
          executor_(SharedIoUringExecutor()) {
        Init();
    }

    std::size_t Write(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        std::vector<WritePtr> writes_to_submit;
        std::size_t consumed = 0;
        bool should_pause = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            active_event_ = ev;
            if (canceled_ || finished_ || FailedLocked()) {
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (!IsUnknownLength(range_.length) && accepted_bytes_ + len > range_.length) {
                FailLocked(FileTransferStatus::Failed(
                    "io_uring file range download received more bytes than expected"));
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (inflight_writes_.size() >= NormalizeIoDepth(options_.io_depth)) {
                waiting_event_ = ev;
                if (ev != nullptr) {
                    ev->setReason("SDK io_uring download waiting for local write");
                    ev->Pause();
                }
                return 0;
            }

            while (consumed < len) {
                if (!active_write_) {
                    active_write_ = NewPendingWriteLocked();
                }

                const std::size_t target = TargetWriteSize();
                if (active_write_->buffer.size() >= target) {
                    WritePtr ready = TakeActiveWriteForSubmitLocked();
                    if (!ready) {
                        should_pause = true;
                        break;
                    }
                    writes_to_submit.push_back(std::move(ready));
                    continue;
                }

                const std::size_t space = target - active_write_->buffer.size();
                const std::size_t to_copy = std::min(space, len - consumed);
                active_write_->buffer.insert(active_write_->buffer.end(), data + consumed,
                                             data + consumed + to_copy);
                accepted_bytes_ += static_cast<std::uint64_t>(to_copy);
                consumed += to_copy;

                if (active_write_->buffer.size() >= target) {
                    WritePtr ready = TakeActiveWriteForSubmitLocked();
                    if (ready) {
                        writes_to_submit.push_back(std::move(ready));
                    } else if (consumed < len) {
                        should_pause = true;
                        break;
                    }
                }
            }

            if (should_pause || consumed < len) {
                waiting_event_ = ev;
                if (ev != nullptr) {
                    ev->setReason("SDK io_uring download waiting for local write");
                    ev->Pause();
                }
            }
        }

        if (!SubmitPreparedWrites(writes_to_submit, ev)) {
            return consumed;
        }
        return consumed;
    }

    void Cancel(std::string reason) override {
        AsyncEvent* event = nullptr;
        bool finalize = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            canceled_ = true;
            FailLocked(FileTransferStatus::Failed(std::move(reason)));
            event = waiting_event_ != nullptr ? waiting_event_ : active_event_;
            waiting_event_ = nullptr;
            active_write_.reset();
            finalize = finished_ && inflight_writes_.empty();
        }
        if (event != nullptr) {
            event->Fail(BaseFileTransfer::result().status.message);
        }
        if (finalize) {
            Finalize();
        }
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        FileTransferResult transfer;
        bool done = false;
        FinishAsync(std::move(network_status), [&transfer, &done](FileTransferResult result) {
            transfer = std::move(result);
            done = true;
        });
        return done ? transfer : BaseFileTransfer::result();
    }

    void FinishAsync(FileTransferStatus network_status,
                     std::function<void(FileTransferResult)> done) override {
        bool finalize = false;
        WritePtr write_to_submit;
        {
            std::lock_guard<std::mutex> lock(mu_);
            finished_ = true;
            final_network_status_ = std::move(network_status);
            finish_callback_ = std::move(done);
            if (final_network_status_.ok) {
                write_to_submit = TakeActiveWriteForSubmitLocked();
            } else {
                active_write_.reset();
            }
            finalize = !HasActiveWriteLocked() && inflight_writes_.empty();
        }
        if (write_to_submit) {
            (void)SubmitPreparedWrite(write_to_submit, nullptr);
        }
        if (finalize) {
            Finalize();
        }
    }

 private:
    struct PendingWrite {
        std::uint64_t seq{0};
        std::uint64_t file_offset{0};
        std::vector<char> buffer;
        FileIoEvent event;
    };
    using WritePtr = std::shared_ptr<PendingWrite>;

    std::size_t TargetWriteSize() const {
        return options_.chunk_size > 0 ? options_.chunk_size : 1024 * 1024;
    }

    bool HasActiveWriteLocked() const {
        return active_write_ && !active_write_->buffer.empty();
    }

    WritePtr NewPendingWriteLocked() {
        WritePtr write = std::make_shared<PendingWrite>();
        if (!free_buffers_.empty()) {
            write->buffer = std::move(free_buffers_.front());
            free_buffers_.pop_front();
            write->buffer.clear();
        }
        write->buffer.reserve(TargetWriteSize());
        write->file_offset = range_.offset + accepted_bytes_;
        return write;
    }

    bool PrepareWriteForSubmitLocked(const WritePtr& write) {
        if (!write || write->buffer.empty() ||
            inflight_writes_.size() >= NormalizeIoDepth(options_.io_depth)) {
            return false;
        }
        write->seq = ++submit_seq_;
        write->event.seq = write->seq;
        write->event.file_offset = write->file_offset;
        write->event.request_len = write->buffer.size();
        write->event.direction = direction_;
        write->event.backend = backend_;
        write->event.data = {write->buffer.data(), write->buffer.size()};
        inflight_writes_[write->seq] = write;
        return true;
    }

    WritePtr TakeActiveWriteForSubmitLocked() {
        if (!HasActiveWriteLocked() || !PrepareWriteForSubmitLocked(active_write_)) {
            return nullptr;
        }
        WritePtr write = std::move(active_write_);
        active_write_.reset();
        return write;
    }

    void RecycleWriteBufferLocked(const WritePtr& write) {
        if (!write || write->buffer.empty()) {
            return;
        }
        const std::size_t max_cached_buffers = NormalizeIoDepth(options_.io_depth) * 2;
        if (free_buffers_.size() >= max_cached_buffers ||
            write->buffer.capacity() > TargetWriteSize() * 2) {
            return;
        }
        write->buffer.clear();
        free_buffers_.push_back(std::move(write->buffer));
    }

    bool SubmitPreparedWrite(const WritePtr& write, AsyncEvent* ev) {
        if (!write) {
            return true;
        }

        FileTransferStatus hook_status = CallBeforeWriteHooks(write->event, options_.hooks);
        if (!hook_status.ok) {
            FailWrite(write, std::move(hook_status), ev);
            return false;
        }

        std::string error;
        if (!executor_->SubmitWrite(range_.fd, write->buffer.data(), write->buffer.size(),
                                   static_cast<off_t>(write->file_offset),
                                   [this, write](int result) { CompleteWrite(write, result); }, &error)) {
            FailWrite(write, FileTransferStatus::Failed(std::move(error)), ev);
            return false;
        }
        return true;
    }

    bool SubmitPreparedWrites(const std::vector<WritePtr>& writes, AsyncEvent* ev) {
        if (writes.empty()) {
            return true;
        }

        std::vector<IoUringExecutor::Submission> submissions;
        submissions.reserve(writes.size());
        for (const auto& write : writes) {
            if (!write) {
                continue;
            }
            FileTransferStatus hook_status = CallBeforeWriteHooks(write->event, options_.hooks);
            if (!hook_status.ok) {
                FailWrite(write, std::move(hook_status), ev);
                return false;
            }
            submissions.push_back(IoUringExecutor::Submission{
                range_.fd, write->buffer.data(), write->buffer.size(), static_cast<off_t>(write->file_offset),
                true, [this, write](int result) { CompleteWrite(write, result); }});
        }

        std::string error;
        if (!executor_->SubmitBatch(std::move(submissions), &error)) {
            FailWrite(nullptr, FileTransferStatus::Failed(std::move(error)), ev);
            return false;
        }
        return true;
    }

    void Init() {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (OpenPathIfNeeded(&range_, true) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (!IsUnknownLength(range_.length) && range_.length > 0) {
                std::uint64_t end = 0;
                if (AddOverflow(range_.offset, range_.length, &end) ||
                    end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
                    FailLocked(
                        FileTransferStatus::Failed("io_uring file range download size overflows off_t"));
                    return;
                }
                if (ftruncate(range_.fd, static_cast<off_t>(end)) != 0) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("ftruncate", errno), errno));
                    return;
                }
            }
        }

        FileTransferStatus status = executor_->Init();
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
            return;
        }
        status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    void FailWrite(const WritePtr& write, FileTransferStatus status, AsyncEvent* ev) {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (write) {
                inflight_writes_.erase(write->seq);
            }
            active_write_.reset();
            inflight_writes_.clear();
            FailLocked(std::move(status));
        }
        if (ev != nullptr) {
            ev->Fail(BaseFileTransfer::result().status.message);
        }
        Finalize();
    }

    void CompleteWrite(const WritePtr& write, int io_result) {
        if (!write) {
            return;
        }
        const std::size_t write_size = write->buffer.size();
        bool canceled_seen = false;
        bool finalize_canceled = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = inflight_writes_.find(write->seq);
            if (it == inflight_writes_.end()) {
                return;
            }
            if (canceled_) {
                canceled_seen = true;
                inflight_writes_.erase(it);
                finalize_canceled = finished_ && inflight_writes_.empty();
            }
        }
        if (canceled_seen) {
            // FinishAsync may already be waiting on the last local write when
            // cancellation arrives. Complete the SDK-level finish callback once
            // the abandoned io_uring requests have drained from the ring.
            if (finalize_canceled) {
                FinalizeAfterCompletion();
            }
            return;
        }

        FileTransferStatus status = FileTransferStatus::Ok();
        if (io_result < 0) {
            status = IoUringStatus("io_uring write", io_result);
        } else if (static_cast<std::size_t>(io_result) != write_size) {
            status = FileTransferStatus::Failed("io_uring file range download short write");
        }
        if (status.ok) {
            FileIoResult result;
            result.status = FileTransferStatus::Ok();
            result.bytes = write_size;
            result.data = {write->buffer.data(), write_size};
            status = CallAfterWriteHooks(write->event, result, options_.hooks);
        }

        AsyncEvent* event = nullptr;
        bool fail_event = false;
        bool finalize = false;
        WritePtr next_write;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = inflight_writes_.find(write->seq);
            if (it == inflight_writes_.end()) {
                return;
            }
            inflight_writes_.erase(it);
            RecycleWriteBufferLocked(write);
            if (!status.ok) {
                FailLocked(std::move(status));
                event = waiting_event_ != nullptr ? waiting_event_ : active_event_;
                waiting_event_ = nullptr;
                fail_event = true;
            } else {
                completed_bytes_ += static_cast<std::uint64_t>(write_size);
                if (HasActiveWriteLocked() &&
                    (finished_ || active_write_->buffer.size() >= TargetWriteSize())) {
                    next_write = TakeActiveWriteForSubmitLocked();
                }
                if (waiting_event_ != nullptr) {
                    event = waiting_event_;
                    waiting_event_ = nullptr;
                }
            }
            finalize = finished_ && !HasActiveWriteLocked() && inflight_writes_.empty();
        }

        if (event != nullptr) {
            if (fail_event) {
                event->Fail("SDK io_uring download local write failed");
            } else {
                event->Resume();
            }
        }
        if (next_write) {
            (void)SubmitPreparedWrite(next_write, event);
        }
        if (finalize) {
            FinalizeAfterCompletion();
        }
    }

    void FinalizeAfterCompletion() {
        if (!executor_->IsWorkerThread()) {
            Finalize();
            return;
        }

        // Finalize runs the SDK finish callback. That callback may release the
        // last shared_ptr owning this sink, so do not run it on the io_uring
        // completion thread; otherwise IoUringExecutor would destruct and try
        // to join the same thread.
        std::thread([this]() { Finalize(); }).detach();
    }

    void Finalize() {
        std::function<void(FileTransferResult)> done;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!finished_ || HasActiveWriteLocked() || !inflight_writes_.empty()) {
                return;
            }
            if (!final_network_status_.ok && result_.status.ok) {
                result_.status = final_network_status_;
            }
            result_.bytes = completed_bytes_;
            if (final_network_status_.ok && result_.status.ok && options_.truncate_file_on_done) {
                FileTransferStatus truncate_status =
                    TruncateToCompletedBytes(range_.fd, range_.offset, result_.bytes);
                if (!truncate_status.ok) {
                    FailLocked(std::move(truncate_status));
                }
            }
            done = std::move(finish_callback_);
            finish_callback_ = nullptr;
        }
        FinishHooksOnce();
        if (done) {
            done(BaseFileTransfer::result());
        }
    }

    bool canceled_{false};
    bool finished_{false};
    std::uint64_t accepted_bytes_{0};
    std::uint64_t completed_bytes_{0};
    std::uint64_t submit_seq_{0};
    WritePtr active_write_;
    std::unordered_map<std::uint64_t, WritePtr> inflight_writes_;
    std::deque<std::vector<char>> free_buffers_;
    FileTransferStatus final_network_status_;
    AsyncEvent* waiting_event_{nullptr};
    AsyncEvent* active_event_{nullptr};
    std::function<void(FileTransferResult)> finish_callback_;
    std::shared_ptr<IoUringExecutor> executor_;
};

class IoUringMemoryFileRangeSink final : public FileRangeDownloadSink, public BaseFileTransfer {
 public:
    IoUringMemoryFileRangeSink(FileRange range, MemoryRange memory, FileTransferOptions options)
        : BaseFileTransfer(std::move(range), std::move(options), FileTransferDirection::Download,
                           FileIoBackend::IoUring, "io_uring memory file range download"),
          memory_(std::move(memory)),
          executor_(SharedIoUringExecutor()) {
        Init();
    }

    std::size_t Write(char* data, std::size_t len, AsyncEvent* ev) override {
        if (data == nullptr || len == 0) {
            return 0;
        }

        std::vector<WritePtr> writes_to_submit;
        {
            std::lock_guard<std::mutex> lock(mu_);
            active_event_ = ev;
            if (canceled_ || finished_ || FailedLocked()) {
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }
            if (accepted_bytes_ + len > memory_.length ||
                (!IsUnknownLength(range_.length) && accepted_bytes_ + len > range_.length)) {
                FailLocked(FileTransferStatus::Failed(
                    "io_uring memory file range download received more bytes than expected"));
                if (ev != nullptr) {
                    ev->Fail(result_.status.message);
                }
                return 0;
            }

            std::memcpy(memory_.data + memory_.offset + accepted_bytes_, data, len);
            accepted_bytes_ += static_cast<std::uint64_t>(len);
            writes_to_submit = TakeReadyWritesLocked(false);
        }

        if (!SubmitPreparedWrites(writes_to_submit, ev)) {
            return 0;
        }
        return len;
    }

    void Cancel(std::string reason) override {
        AsyncEvent* event = nullptr;
        bool finalize = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            canceled_ = true;
            FailLocked(FileTransferStatus::Failed(std::move(reason)));
            event = active_event_;
            finalize = finished_ && inflight_writes_.empty();
        }
        if (event != nullptr) {
            event->Fail(BaseFileTransfer::result().status.message);
        }
        if (finalize) {
            Finalize();
        }
    }

    FileTransferResult result() const override { return BaseFileTransfer::result(); }

    FileTransferResult Finish(FileTransferStatus network_status) override {
        FileTransferResult transfer;
        bool done = false;
        FinishAsync(std::move(network_status), [&transfer, &done](FileTransferResult result) {
            transfer = std::move(result);
            done = true;
        });
        return done ? transfer : BaseFileTransfer::result();
    }

    void FinishAsync(FileTransferStatus network_status,
                     std::function<void(FileTransferResult)> done) override {
        std::vector<WritePtr> writes_to_submit;
        bool finalize = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            finished_ = true;
            final_network_status_ = std::move(network_status);
            finish_callback_ = std::move(done);
            if (!final_network_status_.ok) {
                FailLocked(final_network_status_);
                next_submit_offset_ = accepted_bytes_;
            } else if (!IsUnknownLength(range_.length) && accepted_bytes_ != range_.length) {
                FailLocked(FileTransferStatus::Failed("io_uring memory file range download short body"));
                next_submit_offset_ = accepted_bytes_;
            } else {
                writes_to_submit = TakeReadyWritesLocked(true);
            }
            finalize = inflight_writes_.empty() && next_submit_offset_ >= accepted_bytes_;
        }
        if (!SubmitPreparedWrites(writes_to_submit, nullptr)) {
            return;
        }
        if (finalize) {
            Finalize();
        }
    }

 private:
    struct PendingWrite {
        std::uint64_t seq{0};
        std::uint64_t transfer_offset{0};
        std::size_t len{0};
        FileIoEvent event;
    };
    using WritePtr = std::shared_ptr<PendingWrite>;

    std::size_t TargetWriteSize() const {
        return options_.chunk_size > 0 ? options_.chunk_size : 1024 * 1024;
    }

    void Init() {
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (memory_.data == nullptr || memory_.length == 0) {
                FailLocked(FileTransferStatus::Failed("io_uring memory file range download target is invalid",
                                                      EINVAL));
                return;
            }
            std::uint64_t memory_end = 0;
            if (AddOverflow(memory_.offset, memory_.length, &memory_end) ||
                memory_end > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
                FailLocked(FileTransferStatus::Failed("io_uring memory file range download target overflows",
                                                      EINVAL));
                return;
            }
            if (!IsUnknownLength(range_.length) && range_.length > memory_.length) {
                FailLocked(FileTransferStatus::Failed(
                    "io_uring memory file range download target is smaller than file range", EINVAL));
                return;
            }
            if (OpenPathIfNeeded(&range_, true) < 0) {
                FailLocked(FileTransferStatus::Failed(ErrnoMessage("open", errno), errno));
                return;
            }
            if (!IsUnknownLength(range_.length) && range_.length > 0) {
                std::uint64_t end = 0;
                if (AddOverflow(range_.offset, range_.length, &end) ||
                    end > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
                    FailLocked(FileTransferStatus::Failed(
                        "io_uring memory file range download size overflows off_t"));
                    return;
                }
                if (ftruncate(range_.fd, static_cast<off_t>(end)) != 0) {
                    FailLocked(FileTransferStatus::Failed(ErrnoMessage("ftruncate", errno), errno));
                    return;
                }
            }
        }

        FileTransferStatus status = executor_->Init();
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
            return;
        }
        status = CallBeginHooks(Context(), options_.hooks);
        if (!status.ok) {
            std::lock_guard<std::mutex> lock(mu_);
            FailLocked(std::move(status));
        }
    }

    std::vector<WritePtr> TakeReadyWritesLocked(bool flush_tail) {
        std::vector<WritePtr> writes;
        const std::size_t depth = NormalizeIoDepth(options_.io_depth);
        const std::size_t target = TargetWriteSize();
        while (inflight_writes_.size() + writes.size() < depth && next_submit_offset_ < accepted_bytes_) {
            const std::uint64_t ready = accepted_bytes_ - next_submit_offset_;
            if (!flush_tail && ready < target) {
                break;
            }
            const std::size_t write_len =
                static_cast<std::size_t>(std::min<std::uint64_t>(ready, target));
            auto write = std::make_shared<PendingWrite>();
            write->seq = ++submit_seq_;
            write->transfer_offset = next_submit_offset_;
            write->len = write_len;
            write->event.seq = write->seq;
            write->event.file_offset = range_.offset + write->transfer_offset;
            write->event.request_len = write_len;
            write->event.direction = direction_;
            write->event.backend = backend_;
            write->event.data = {memory_.data + memory_.offset + write->transfer_offset, write_len};
            inflight_writes_[write->seq] = write;
            next_submit_offset_ += write_len;
            writes.push_back(std::move(write));
        }
        return writes;
    }

    bool SubmitPreparedWrites(const std::vector<WritePtr>& writes, AsyncEvent* ev) {
        if (writes.empty()) {
            return true;
        }
        std::vector<IoUringExecutor::Submission> submissions;
        submissions.reserve(writes.size());
        for (const auto& write : writes) {
            FileTransferStatus hook_status = CallBeforeWriteHooks(write->event, options_.hooks);
            if (!hook_status.ok) {
                FailWrite(std::move(hook_status), ev);
                return false;
            }
            submissions.push_back(IoUringExecutor::Submission{
                range_.fd, memory_.data + memory_.offset + write->transfer_offset, write->len,
                static_cast<off_t>(range_.offset + write->transfer_offset), true,
                [this, write](int result) { CompleteWrite(write, result); }});
        }
        std::string error;
        if (!executor_->SubmitBatch(std::move(submissions), &error)) {
            FailWrite(FileTransferStatus::Failed(std::move(error)), ev);
            return false;
        }
        return true;
    }

    void FailWrite(FileTransferStatus status, AsyncEvent* ev) {
        {
            std::lock_guard<std::mutex> lock(mu_);
            inflight_writes_.clear();
            FailLocked(std::move(status));
            next_submit_offset_ = accepted_bytes_;
        }
        if (ev != nullptr) {
            ev->Fail(BaseFileTransfer::result().status.message);
        }
        Finalize();
    }

    void CompleteWrite(const WritePtr& write, int io_result) {
        if (!write) {
            return;
        }

        FileTransferStatus status = FileTransferStatus::Ok();
        if (io_result < 0) {
            status = IoUringStatus("io_uring write", io_result);
        } else if (static_cast<std::size_t>(io_result) != write->len) {
            status = FileTransferStatus::Failed("io_uring memory file range download short write");
        }
        if (status.ok) {
            FileIoResult result;
            result.status = FileTransferStatus::Ok();
            result.bytes = write->len;
            result.data = {memory_.data + memory_.offset + write->transfer_offset, write->len};
            status = CallAfterWriteHooks(write->event, result, options_.hooks);
        }

        std::vector<WritePtr> writes_to_submit;
        AsyncEvent* event = nullptr;
        bool fail_event = false;
        bool finalize = false;
        {
            std::lock_guard<std::mutex> lock(mu_);
            auto it = inflight_writes_.find(write->seq);
            if (it == inflight_writes_.end()) {
                return;
            }
            inflight_writes_.erase(it);
            if (!status.ok) {
                FailLocked(std::move(status));
                next_submit_offset_ = accepted_bytes_;
                event = active_event_;
                fail_event = true;
            } else {
                completed_bytes_ += static_cast<std::uint64_t>(write->len);
                if (!canceled_ && !FailedLocked()) {
                    writes_to_submit = TakeReadyWritesLocked(finished_);
                }
            }
            finalize = finished_ && inflight_writes_.empty() && next_submit_offset_ >= accepted_bytes_;
        }

        if (event != nullptr && fail_event) {
            event->Fail("SDK io_uring memory download local write failed");
        }
        if (!SubmitPreparedWrites(writes_to_submit, event)) {
            return;
        }
        if (finalize) {
            FinalizeAfterCompletion();
        }
    }

    void FinalizeAfterCompletion() {
        if (!executor_->IsWorkerThread()) {
            Finalize();
            return;
        }
        std::thread([this]() { Finalize(); }).detach();
    }

    void Finalize() {
        std::function<void(FileTransferResult)> done;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (!finished_ || !inflight_writes_.empty() || next_submit_offset_ < accepted_bytes_) {
                return;
            }
            if (!final_network_status_.ok && result_.status.ok) {
                result_.status = final_network_status_;
            }
            result_.bytes = completed_bytes_;
            done = std::move(finish_callback_);
            finish_callback_ = nullptr;
        }
        FinishHooksOnce();
        if (done) {
            done(BaseFileTransfer::result());
        }
    }

    MemoryRange memory_;
    bool canceled_{false};
    bool finished_{false};
    std::uint64_t accepted_bytes_{0};
    std::uint64_t next_submit_offset_{0};
    std::uint64_t completed_bytes_{0};
    std::uint64_t submit_seq_{0};
    std::unordered_map<std::uint64_t, WritePtr> inflight_writes_;
    FileTransferStatus final_network_status_;
    AsyncEvent* active_event_{nullptr};
    std::function<void(FileTransferResult)> finish_callback_;
    std::shared_ptr<IoUringExecutor> executor_;
};
#endif

FileIoBackend SelectUploadBackend(FileIoBackend requested, const FileRange& range,
                                  std::uint64_t auto_upload_iouring_max_bytes) {
    if (requested != FileIoBackend::Auto) {
        return requested;
    }
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
    // Upload through libcurl still copies from the SDK read buffer into curl's
    // callback buffer. Keep iouring for latency-sensitive small objects, and use
    // posix for larger uploads where direct pread into curl's buffer costs less CPU.
    const std::uint64_t iouring_max_bytes =
        auto_upload_iouring_max_bytes > 0 ? auto_upload_iouring_max_bytes : AutoUploadIoUringMaxBytes();
    if (IoUringRuntimeAvailable() && !IsUnknownLength(range.length) &&
        range.length <= iouring_max_bytes) {
        return FileIoBackend::IoUring;
    }
#endif
    return FileIoBackend::Posix;
}

FileIoBackend SelectDownloadBackend(FileIoBackend requested, const FileRange& range) {
    if (requested != FileIoBackend::Auto) {
        return requested;
    }
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
    if (IoUringRuntimeAvailable()) {
        return FileIoBackend::IoUring;
    }
#endif
    return IsUnknownLength(range.length) ? FileIoBackend::Posix : FileIoBackend::Mmap;
}

}  // namespace

void FileTransferControl::Cancel(std::string reason) {
    CancelCallback callback;
    std::string callback_reason;
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (reason.empty()) {
            reason = "file transfer canceled";
        }
        canceled_ = true;
        cancel_reason_ = std::move(reason);
        callback = cancel_callback_;
        callback_reason = cancel_reason_;
    }
    if (callback) {
        callback(callback_reason);
    }
}

void FileTransferControl::AttachCancelCallback(CancelCallback callback) {
    bool call_now = false;
    std::string reason;
    {
        std::lock_guard<std::mutex> lock(mu_);
        cancel_callback_ = std::move(callback);
        call_now = canceled_;
        reason = cancel_reason_;
    }
    if (call_now) {
        CancelCallback callback_copy;
        {
            std::lock_guard<std::mutex> lock(mu_);
            callback_copy = cancel_callback_;
        }
        if (callback_copy) {
            callback_copy(reason);
        }
    }
}

bool FileTransferControl::canceled() const {
    std::lock_guard<std::mutex> lock(mu_);
    return canceled_;
}

std::string FileTransferControl::cancel_reason() const {
    std::lock_guard<std::mutex> lock(mu_);
    return cancel_reason_;
}

const char* FileIoBackendName(FileIoBackend backend) {
    switch (backend) {
        case FileIoBackend::Auto:
            return "auto";
        case FileIoBackend::Mmap:
            return "mmap";
        case FileIoBackend::IoUring:
            return "io_uring";
        case FileIoBackend::Posix:
            return "posix";
    }
    return "unknown";
}

FileRangeAsyncDataSourcePtr CreateFileRangeAsyncDataSource(FileRange range, FileTransferOptions options) {
    const auto control = options.control;
    const FileIoBackend backend =
        SelectUploadBackend(options.backend, range, options.auto_upload_iouring_max_bytes);
    FileRangeAsyncDataSourcePtr source;
    switch (backend) {
        case FileIoBackend::Mmap:
            source = std::make_shared<MmapFileRangeSource>(std::move(range), std::move(options));
            break;
        case FileIoBackend::Posix:
            source = std::make_shared<PosixFileRangeSource>(std::move(range), std::move(options));
            break;
        case FileIoBackend::IoUring:
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
            source = std::make_shared<IoUringFileRangeSource>(std::move(range), std::move(options));
            break;
#else
            source = std::make_shared<UnsupportedFileRangeSource>(
                "SDK io_uring file range upload engine is not implemented yet");
            break;
#endif
        case FileIoBackend::Auto:
            break;
    }
    if (!source) {
        source = std::make_shared<UnsupportedFileRangeSource>("invalid SDK file range upload backend");
    }
    if (control) {
        std::weak_ptr<FileRangeAsyncDataSource> weak_source = source;
        control->AttachCancelCallback([weak_source](const std::string&) {
            if (auto source = weak_source.lock()) {
                source->Cancel();
            }
        });
    }
    return source;
}

FileRangeDownloadSinkPtr CreateFileRangeDownloadSink(FileRange range, FileTransferOptions options) {
    const auto control = options.control;
    const FileIoBackend backend = SelectDownloadBackend(options.backend, range);
    FileRangeDownloadSinkPtr sink;
    switch (backend) {
        case FileIoBackend::Mmap:
            sink = std::make_shared<MmapFileRangeSink>(std::move(range), std::move(options));
            break;
        case FileIoBackend::Posix:
            sink = std::make_shared<PosixFileRangeSink>(std::move(range), std::move(options));
            break;
        case FileIoBackend::IoUring:
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
            sink = std::make_shared<IoUringFileRangeSink>(std::move(range), std::move(options));
            break;
#else
            sink = std::make_shared<UnsupportedDownloadSink>(
                "SDK io_uring file range download engine is not implemented yet");
            break;
#endif
        case FileIoBackend::Auto:
            break;
    }
    if (!sink) {
        sink = std::make_shared<UnsupportedDownloadSink>("invalid SDK file range download backend");
    }
    if (control) {
        std::weak_ptr<FileRangeDownloadSink> weak_sink = sink;
        control->AttachCancelCallback([weak_sink](const std::string& reason) {
            if (auto sink = weak_sink.lock()) {
                sink->Cancel(reason);
            }
        });
    }
    return sink;
}

FileRangeDownloadSinkPtr CreateMemoryFileRangeDownloadSink(FileRange range, MemoryRange memory,
                                                           FileTransferOptions options) {
    const auto control = options.control;
    const FileIoBackend backend = SelectDownloadBackend(options.backend, range);
    FileRangeDownloadSinkPtr sink;
    switch (backend) {
        case FileIoBackend::IoUring:
#if VOLCENGINE_TOS_ASYNC_HAS_IO_URING
            sink = std::make_shared<IoUringMemoryFileRangeSink>(std::move(range), std::move(memory),
                                                                std::move(options));
            break;
#else
            sink = std::make_shared<UnsupportedDownloadSink>(
                "SDK io_uring memory file range download engine is not implemented yet");
            break;
#endif
        case FileIoBackend::Mmap:
        case FileIoBackend::Posix:
        case FileIoBackend::Auto:
            sink = std::make_shared<UnsupportedDownloadSink>(
                "SDK memory file range download requires io_uring backend");
            break;
    }
    if (!sink) {
        sink = std::make_shared<UnsupportedDownloadSink>("invalid SDK memory file range download backend");
    }
    if (control) {
        std::weak_ptr<FileRangeDownloadSink> weak_sink = sink;
        control->AttachCancelCallback([weak_sink](const std::string& reason) {
            if (auto sink = weak_sink.lock()) {
                sink->Cancel(reason);
            }
        });
    }
    return sink;
}

}  // namespace VolcengineTos
