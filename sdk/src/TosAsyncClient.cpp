#include "TosAsyncClient.h"

#include "TosClientBase.h"
#include "executor/TosClientTemplate.h"
#include "TosClientV2.h"
#include "auth/SignV4.h"
#include "executor/ProcessingPipline.h"
#include "utils/MimeType.h"
#include "executor/PiplineCoordinator.h"
#include "executor/PipelineChainAdapters.h"
#include "executor/PipelineBase.h"

#include <sys/stat.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <type_traits>
#include <vector>


using namespace VolcengineTos;
namespace {

constexpr size_t kMaxPooledPipelinesPerType = 64;

template <typename T, typename O>
struct ShouldPoolPipeline : std::true_type {};

template <>
struct ShouldPoolPipeline<GetObjectToFileAsyncInput, GetObjectToFileAsyncOutput> : std::false_type {};

template <>
struct ShouldPoolPipeline<PutObjectFromFileAsyncInput, PutObjectFromFileAsyncOutput> : std::false_type {};

TosError MakeFileTransferError(const FileTransferResult& transfer, const std::string& fallback) {
    TosError error;
    error.setIsClientError(true);
    error.setCode("FileTransferError");
    error.setMessage(!transfer.status.message.empty() ? transfer.status.message : fallback);
    return error;
}

template <typename OutputT>
void ApplyUploadFileTransferResult(Outcome<TosError, OutputT>& outcome, const FileTransferResult& transfer,
                                   std::uint64_t expected_bytes) {
    if (!outcome.isSuccess()) {
        return;
    }
    if (!transfer.status.ok) {
        outcome.setSuccess(false);
        outcome.setE(MakeFileTransferError(transfer, "file range upload failed"));
        return;
    }
    if (transfer.bytes != expected_bytes) {
        FileTransferResult mismatch = transfer;
        mismatch.status = FileTransferStatus::Failed("file range upload bytes do not match content length");
        outcome.setSuccess(false);
        outcome.setE(MakeFileTransferError(mismatch, "file range upload bytes mismatch"));
    }
}

template <typename OutputT>
void ApplyDownloadFileTransferResult(Outcome<TosError, OutputT>& outcome, const FileTransferResult& transfer) {
    if (!outcome.isSuccess()) {
        return;
    }
    if (!transfer.status.ok) {
        outcome.setSuccess(false);
        outcome.setE(MakeFileTransferError(transfer, "file range download failed"));
        return;
    }
    const int64_t content_length = outcome.result().getContentLength();
    if (content_length >= 0 && transfer.bytes != static_cast<std::uint64_t>(content_length)) {
        FileTransferResult mismatch = transfer;
        mismatch.status = FileTransferStatus::Failed("file range download bytes do not match response content length");
        outcome.setSuccess(false);
        outcome.setE(MakeFileTransferError(mismatch, "file range download bytes mismatch"));
    }
}

bool GetRegularFileSize(const std::string& path, std::uint64_t* size, std::string* error) {
    if (size != nullptr) {
        *size = 0;
    }
    if (path.empty()) {
        if (error != nullptr) {
            *error = "empty file path";
        }
        return false;
    }
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) {
        if (error != nullptr) {
            *error = std::string("stat file failed: ") + strerror(errno);
        }
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        if (error != nullptr) {
            *error = "path is not a regular file";
        }
        return false;
    }
    if (st.st_size < 0) {
        if (error != nullptr) {
            *error = "file size is negative";
        }
        return false;
    }
    if (size != nullptr) {
        *size = static_cast<std::uint64_t>(st.st_size);
    }
    return true;
}

std::string Lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

FileIoBackend ParseFileTransferBackend(std::string backend) {
    backend = Lowercase(std::move(backend));
    if (backend == "posix") {
        return FileIoBackend::Posix;
    }
    if (backend == "iouring" || backend == "io_uring") {
        return FileIoBackend::IoUring;
    }
    if (backend == "mmap") {
        return FileIoBackend::Mmap;
    }
    return FileIoBackend::Auto;
}

FileTransferOptions MakeClientFileTransferOptions(const ClientConfig& config) {
    FileTransferOptions options;
    options.backend = ParseFileTransferBackend(config.fileTransferBackend);
    if (config.fileTransferChunkSize > 0) {
        options.chunk_size = config.fileTransferChunkSize;
    }
    if (config.fileTransferIoDepth > 0) {
        options.io_depth = config.fileTransferIoDepth;
    }
    options.auto_upload_iouring_max_bytes = config.fileTransferAutoUploadIoUringMaxBytes;
    return options;
}

FileTransferOptions MergeClientFileTransferOptions(FileTransferOptions options,
                                                   const FileTransferOptions& defaults) {
    const FileTransferOptions base;
    if (options.backend == FileIoBackend::Auto && defaults.backend != FileIoBackend::Auto) {
        options.backend = defaults.backend;
    }
    if (options.chunk_size == base.chunk_size && defaults.chunk_size > 0) {
        options.chunk_size = defaults.chunk_size;
    }
    if (options.io_depth == base.io_depth && defaults.io_depth > 0) {
        options.io_depth = defaults.io_depth;
    }
    if (options.auto_upload_iouring_max_bytes == 0) {
        options.auto_upload_iouring_max_bytes = defaults.auto_upload_iouring_max_bytes;
    }
    return options;
}

template <typename T, typename O>
class BoundedPipelinePool {
public:
    ~BoundedPipelinePool() {
        for (auto* pipeline : cache_) {
            delete pipeline;
        }
    }

    ProcessingPipline<T, O>* acquire(const T& input, bool& created) {
        std::lock_guard<std::mutex> guard(mu_);
        if (!cache_.empty()) {
            auto* pipeline = cache_.back();
            cache_.pop_back();
            pipeline->resetForReuse(input);
            created = false;
            return pipeline;
        }

        created = true;
        return new ProcessingPipline<T, O>(input);
    }

    void recycle(ProcessingPipline<T, O>* pipeline) {
        if (pipeline == nullptr) {
            return;
        }

        std::lock_guard<std::mutex> guard(mu_);
        if (cache_.size() >= kMaxPooledPipelinesPerType) {
            delete pipeline;
            return;
        }

        pipeline->recycle();
        cache_.push_back(pipeline);
    }

private:
    std::mutex mu_;
    std::vector<ProcessingPipline<T, O>*> cache_;
};

template <typename T, typename O>
BoundedPipelinePool<T, O>& getPipelinePool() {
    static BoundedPipelinePool<T, O> pool;
    return pool;
}

template <typename T, typename O>
ProcessingPipline<T, O>* acquirePipelineInstance(const T& input, bool use_pool) {
    bool created = false;
    ProcessingPipline<T, O>* pipeline = nullptr;
    if (use_pool) {
        pipeline = getPipelinePool<T, O>().acquire(input, created);
    } else {
        created = true;
        pipeline = new ProcessingPipline<T, O>(input);
    }

    if (created) {
        const uint32_t create_count = TosAsyncClient::g_pipline_create_cnt.fetch_add(1, std::memory_order_acq_rel);
        const uint32_t destroy_count = TosAsyncClient::g_pipline_destroy_cnt.load(std::memory_order_relaxed);
        if (create_count % 1000 == 0) {
            Logger::getInstance().info("pipline create: ", create_count, ", destroy: ", destroy_count);
        }
    }

    return pipeline;
}

template <typename T, typename O>
void attachPipelineFinalizer(ProcessingPipline<T, O>* pipeline, bool use_pool) {
    if (use_pool) {
        pipeline->afterPiplineFinish([pipeline]() { getPipelinePool<T, O>().recycle(pipeline); });
    } else {
        pipeline->afterPiplineFinish([pipeline]() { delete pipeline; });
    }
}

template <typename T, typename O>
ProcessingPipline<T, O>* newBasePipeline(const T& input, bool use_pool) {
    auto* pipeline = acquirePipelineInstance<T, O>(input, use_pool);
    attachPipelineFinalizer(pipeline, use_pool);
    pipeline->onDestroy([]() { TosAsyncClient::g_pipline_destroy_cnt.fetch_add(1, std::memory_order_acq_rel); });
    return pipeline;
}

}  // namespace

std::atomic<uint64_t> TosAsyncClient::g_pipline_create_cnt(0);
std::atomic<uint64_t> TosAsyncClient::g_pipline_destroy_cnt(0);

void VolcengineTos::InitializeTosAsyncClient() {
    AsyncHttpClient::initCurl();
}
void VolcengineTos::CloseTosAsyncClient() {
    AsyncHttpClient::cleanupCurl();
}

// 方式一：仅包含秘钥
TosAsyncClient::TosAsyncClient(const std::string& region, const std::string& accessKeyId,
                               const std::string& secretKeyId)
        : TosAsyncClient(region, StaticCredentials(accessKeyId, secretKeyId)) {
}
// 方式二：含stsToken
TosAsyncClient::TosAsyncClient(const std::string& region, const std::string& accessKeyId,
                               const std::string& secretKeyId, const std::string& securityToken)
        : TosAsyncClient(region, StaticCredentials(accessKeyId, secretKeyId, securityToken)) {
}
// 方式一、二
TosAsyncClient::TosAsyncClient(const std::string& region, const StaticCredentials& cred)
        : tosClientBase_(std::make_shared<TosClientBase>("", region, cred)) {
}

// 方式三：带config
TosAsyncClient::TosAsyncClient(const std::string& region, const std::string& accessKeyId,
                               const std::string& secretKeyId, const ClientConfig& config)
        : TosAsyncClient(region, StaticCredentials(accessKeyId, secretKeyId), config) {
}
TosAsyncClient::TosAsyncClient(const std::string& region, const StaticCredentials& cred, const ClientConfig& config)
        : tosClientBase_(std::make_shared<TosClientBase>(config.endPoint, region, cred, config)),
          fileTransferOptions_(MakeClientFileTransferOptions(config)) {
}

TosAsyncClient::TosAsyncClient(const std::string& region, const FederationCredentials& cred)
        : tosClientBase_(std::make_shared<TosClientBase>("", region, cred)) {
}

TosAsyncClient::TosAsyncClient(const std::string& region, const std::string& accessKeyId,
                               const std::string& secretKeyId, const std::string& securityToken,
                               const ClientConfig& config)
        : TosAsyncClient(region, StaticCredentials(accessKeyId, secretKeyId, securityToken), config) {
}
TosAsyncClient::TosAsyncClient(const std::string& region, const FederationCredentials& cred, const ClientConfig& config)
        : tosClientBase_(std::make_shared<TosClientBase>(config.endPoint, region, cred, config)),
          fileTransferOptions_(MakeClientFileTransferOptions(config)) {
}

TosAsyncClient::TosAsyncClient(const std::string& region, const std::shared_ptr<Credentials>& cred)
        : tosClientBase_(std::make_shared<TosClientBase>("", region, cred)) {
}

TosAsyncClient::TosAsyncClient(const std::string& region, const std::shared_ptr<Credentials>& cred,
                               const ClientConfig& config)
        : tosClientBase_(std::make_shared<TosClientBase>(config.endPoint, region, cred, config)),
          fileTransferOptions_(MakeClientFileTransferOptions(config)) {
}

TosAsyncClient::~TosAsyncClient() {
    close();
}

void TosAsyncClient::close() const {
    if (tosClientBase_ == nullptr) {
        return;
    }
    tosClientBase_->closeAsyncTransport(false);
}

static void setContentType(RequestBuilder& rb, const std::string& objectKey);

template <typename T>
std::function<std::shared_ptr<HttpRequest>(T&)> getDefaultInput2HttpRequest(
        const std::shared_ptr<TosClientBase>& tosClient, const std::string& method, const std::string& bucket,
        const std::string& key, const std::string& funcName) {
    return [tosClient, bucket, key, funcName, method](T& input) {
        static_cast<PiplineInputInterface&>(input).input2Queries();
        static_cast<PiplineInputInterface&>(input).input2Headers();
        auto rb = tosClient->newBuilder(bucket, key, input.getRequestDate(), input.getHeaders(), input.getQueries(),
                                        input.getContentLength());

        if (StringUtils::startsWithIgnoreCase(funcName, "put") ||
            StringUtils::startsWithIgnoreCase(funcName, "modify") ||
            StringUtils::startsWithIgnoreCase(funcName, "upload")
            // StringUtils::startsWithIgnoreCase(funcName, "append")
        ) {
            if (tosClient->getConfig().isAutoRecognizeContentType()) {
                setContentType(rb, key);
            }
        }

        return rb.buildHttpRequest(method);
    };
}

template <typename T, typename O>
ProcessingPipline<T, O>* getBasePipline(
        const std::vector<int>& expectStatus, const T& input, const std::shared_ptr<TosClientBase>& tosClient,
        const OnDataReceiveWithEvent& on_data_receive, const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, O>&)>& on_request_done) {
    constexpr bool kUsePool = ShouldPoolPipeline<T, O>::value;
    auto* pipline = newBasePipeline<T, O>(input, kUsePool);

    auto default_valid = [](T& ipt) { return static_cast<PiplineInputInterface&>(ipt).valid(); };

    auto default_response2_out_come = [](const std::shared_ptr<HttpRequest>& request,
                                         const std::shared_ptr<HttpResponse>& response, TosError& se, O& result) {
        static_cast<PiplineOutputInterface&>(result).headers2Output(*response);
    };

    auto default_input2json = [](T& ipt) { return static_cast<PiplineInputInterface&>(ipt).input2json(); };

    auto default_json2output = [](O& result, json& j) {
        static_cast<PiplineOutputInterface&>(result).json2Output(j);
    };

    pipline->inputCheck(default_valid)
            .input2Json(default_input2json)
            .httpRequestSender([tosClient](const std::shared_ptr<HttpRequest>& request,
                                           const OnDataReceiveWithEvent& dr, const OnDataSendWithEvent& sd,
                                           const std::function<void(std::shared_ptr<HttpResponse>)>& fi,
                                           const OnRequestStart& st, const OnHttpStatusSet& sts,
                                           const OnContentLengthSet& cls) {
                (void)tosClient->getAsyncTransport()->send(request, dr, sd, fi, st, sts, cls);
            })
            .OnDataReceiveWithEvent(on_data_receive)
            .OnDataSendWithEvent(on_data_send)
            .onRequestDone(on_request_done)
            .expectStatus(expectStatus)
            .defaultUnexpectResponse2Outcome()
            .json2Output(default_json2output)
            .expectResponse2Outcome(default_response2_out_come)
            .enableRetry(tosClient->getConfig().getMaxRetryCount(),
                         tosClient->getConfig().getRetrySleepScale())
            ;

    return pipline;
}

ProcessingPipline<GetObjectAsyncInput, GetObjectAsyncOutput>* getObject(
        const std::shared_ptr<TosClientBase>& tosClient, const GetObjectAsyncInput& input,
        const OnDataReceiveWithEvent& on_data_receive,
        const std::function<void(Outcome<TosError, GetObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<GetObjectAsyncInput, GetObjectAsyncOutput>(
            {200, 203, 206}, input, tosClient, on_data_receive, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](GetObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<GetObjectAsyncInput>(
                    tosClient, http::MethodGet, input.getBucketName(), input.getKey(), __func__));
    return pipeline;
}

ProcessingPipline<GetObjectToFileAsyncInput, GetObjectToFileAsyncOutput>* getObjectToFile(
        const std::shared_ptr<TosClientBase>& tosClient, const FileTransferOptions& fileTransferOptions,
        const GetObjectToFileAsyncInput& input,
        const std::function<void(Outcome<TosError, GetObjectToFileAsyncOutput>&)>& on_request_done) {
    FileRange range;
    range.path = input.getFilePath();
    range.offset = 0;
    range.length = kUnknownFileRangeLength;
    FileTransferOptions options;
    options.truncate_file_on_done = true;
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions);
    auto sink = CreateFileRangeDownloadSink(std::move(range), std::move(options));
    auto on_data_receive = [sink](char* data, size_t len, AsyncEvent* ev) {
        return sink ? sink->Write(data, len, ev) : static_cast<size_t>(0);
    };
    auto wrapped_done =
            [sink, on_request_done](Outcome<TosError, GetObjectToFileAsyncOutput>& outcome) mutable {
                FileTransferStatus network_status = FileTransferStatus::Ok();
                if (!outcome.isSuccess()) {
                    network_status = FileTransferStatus::Failed(outcome.error().String());
                }
                if (!sink) {
                    FileTransferResult transfer{FileTransferStatus::Failed("download sink missing")};
                    ApplyDownloadFileTransferResult(outcome, transfer);
                    on_request_done(outcome);
                    return;
                }
                auto outcome_holder =
                        std::make_shared<Outcome<TosError, GetObjectToFileAsyncOutput>>(std::move(outcome));
                sink->FinishAsync(
                        std::move(network_status),
                        [sink, outcome_holder, on_request_done](FileTransferResult transfer) mutable {
                            ApplyDownloadFileTransferResult(*outcome_holder, transfer);
                            on_request_done(*outcome_holder);
                        });
            };
    auto* pipeline = getBasePipline<GetObjectToFileAsyncInput, GetObjectToFileAsyncOutput>(
            {200, 203, 206}, input, tosClient, on_data_receive, nullptr, wrapped_done);
    pipeline->decorateInput([&tosClient](GetObjectToFileAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<GetObjectToFileAsyncInput>(
                    tosClient, http::MethodGet, input.getBucketName(), input.getKey(), __func__))
            .decorateHttpRequest([](const std::shared_ptr<HttpRequest>& request) -> void {

            });
    return pipeline;
}

ProcessingPipline<PutObjectAsyncInput, PutObjectAsyncOutput>* putObject(
        const std::shared_ptr<TosClientBase>& tosClient, const PutObjectAsyncInput& input,
        const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, PutObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<PutObjectAsyncInput, PutObjectAsyncOutput>({200}, input, tosClient, nullptr,
                                                                               on_data_send, on_request_done);
    pipeline->decorateInput([&tosClient](PutObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .transferEncodingCheck()
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<PutObjectAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__));
    return pipeline;
}

ProcessingPipline<PutObjectFromFileAsyncInput, PutObjectFromFileAsyncOutput>* putObjectFromFile(
        const std::shared_ptr<TosClientBase>& tosClient, const FileTransferOptions& fileTransferOptions,
        const PutObjectFromFileAsyncInput& input,
        const std::function<void(Outcome<TosError, PutObjectFromFileAsyncOutput>&)>& on_request_done) {
    std::uint64_t file_size = 0;
    std::string file_error;
    const bool file_ok = GetRegularFileSize(input.getFilePath(), &file_size, &file_error);
    FileRange range;
    range.path = input.getFilePath();
    range.offset = 0;
    range.length = file_size;
    auto source =
            CreateFileRangeAsyncDataSource(std::move(range),
                                           MergeClientFileTransferOptions(FileTransferOptions(),
                                                                          fileTransferOptions));
    auto on_data_send = [source](char* data, size_t len, AsyncEvent* ev) {
        return source ? source->Read(data, len, ev) : static_cast<size_t>(0);
    };
    auto wrapped_done =
            [source, file_size, on_request_done](
                    Outcome<TosError, PutObjectFromFileAsyncOutput>& outcome) mutable {
                FileTransferStatus network_status = FileTransferStatus::Ok();
                if (!outcome.isSuccess()) {
                    network_status = FileTransferStatus::Failed(outcome.error().String());
                }
                FileTransferResult transfer =
                        source ? source->Finish(std::move(network_status))
                               : FileTransferResult{FileTransferStatus::Failed("upload source missing")};
                ApplyUploadFileTransferResult(outcome, transfer, file_size);
                on_request_done(outcome);
            };
    PutObjectFromFileAsyncInput upload_input = input;
    upload_input.setContentLength(static_cast<int64_t>(file_size));
    auto* pipeline = getBasePipline<PutObjectFromFileAsyncInput, PutObjectFromFileAsyncOutput>(
            {200}, upload_input, tosClient, nullptr, on_data_send, wrapped_done);
    pipeline->inputCheck([file_ok, file_error](const PutObjectFromFileAsyncInput&) {
                return file_ok ? std::string() : file_error;
            })
            .decorateInput([&tosClient](PutObjectFromFileAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .transferEncodingCheck()
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<PutObjectFromFileAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .decorateHttpRequest([file_size](const std::shared_ptr<HttpRequest>& request) {
                request->setContentLength(static_cast<int64_t>(file_size));
            });
    return pipeline;
}

ProcessingPipline<ModifyObjectAsyncInput, ModifyObjectAsyncOutput>* modifyObject(
        const std::shared_ptr<TosClientBase>& tosClient, const ModifyObjectAsyncInput& input,
        const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<ModifyObjectAsyncInput, ModifyObjectAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                     on_data_send, on_request_done);
    pipeline->decorateInput([&tosClient](ModifyObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .transferEncodingCheck()
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<ModifyObjectAsyncInput>(
                    tosClient, http::MethodPost, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<AppendObjectAsyncInput, AppendObjectAsyncOutput>* appendObject(
        const std::shared_ptr<TosClientBase>& tosClient, const AppendObjectAsyncInput& input,
        const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, AppendObjectAsyncOutput>&)>& on_request_done) {
    const std::string func_name = __func__;
    auto* pipeline = getBasePipline<AppendObjectAsyncInput, AppendObjectAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                     on_data_send, on_request_done);
    pipeline->decorateInput([&tosClient](AppendObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .transferEncodingCheck()
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(func_name)
            .input2HttpRequest(getDefaultInput2HttpRequest<AppendObjectAsyncInput>(
                    tosClient, http::MethodPost, input.getBucketName(), input.getKey(), func_name))
            .responseJson();
    return pipeline;
}

ProcessingPipline<CopyObjectAsyncInput, CopyObjectAsyncOutput>* copyObject(
        const std::shared_ptr<TosClientBase>& tosClient, const CopyObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, CopyObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<CopyObjectAsyncInput, CopyObjectAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                 nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](CopyObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<CopyObjectAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<DeleteObjectAsyncInput, DeleteObjectAsyncOutput>* deleteObject(
        const std::shared_ptr<TosClientBase>& tosClient, const DeleteObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, DeleteObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<DeleteObjectAsyncInput, DeleteObjectAsyncOutput>({204}, input, tosClient, nullptr,
                                                                                     nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](DeleteObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<DeleteObjectAsyncInput>(
                    tosClient, http::MethodDelete, input.getBucketName(), input.getKey(), __func__));
    return pipeline;
}

ProcessingPipline<SetObjectMetaAsyncInput, SetObjectMetaAsyncOutput>* setObjectMeta(
        const std::shared_ptr<TosClientBase>& tosClient, const SetObjectMetaAsyncInput& input,
        const std::function<void(Outcome<TosError, SetObjectMetaAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<SetObjectMetaAsyncInput, SetObjectMetaAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                       nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](SetObjectMetaAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<SetObjectMetaAsyncInput>(
                    tosClient, http::MethodPost, input.getBucketName(), input.getKey(), __func__));
    return pipeline;
}

ProcessingPipline<CreateBucketAsyncInput, CreateBucketAsyncOutput>* createBucket(
        const std::shared_ptr<TosClientBase>& tosClient, const CreateBucketAsyncInput& input,
        const std::function<void(Outcome<TosError, CreateBucketAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<CreateBucketAsyncInput, CreateBucketAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                     nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](CreateBucketAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<CreateBucketAsyncInput>(tosClient, http::MethodPut,
                                                                                   input.getBucketName(), "", __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<ListBucketsAsyncInput, ListBucketsAsyncOutput>* listBuckets(
        const std::shared_ptr<TosClientBase>& tosClient, const ListBucketsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListBucketsAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<ListBucketsAsyncInput, ListBucketsAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                   nullptr, on_request_done);
    pipeline->userName(__func__)
            .input2HttpRequest(
                    getDefaultInput2HttpRequest<ListBucketsAsyncInput>(tosClient, http::MethodGet, "", "", __func__))
            .responseJson();  // 响应当json处理
    return pipeline;
}

ProcessingPipline<ListObjectsAsyncInput, ListObjectsAsyncOutput>* listObjects(
        const std::shared_ptr<TosClientBase>& tosClient, const ListObjectsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListObjectsAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<ListObjectsAsyncInput, ListObjectsAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                   nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](ListObjectsAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<ListObjectsAsyncInput>(tosClient, http::MethodGet,
                                                                                  input.getBucketName(), "", __func__))
            .responseJson();  // 响应当json处理
    return pipeline;
}

ProcessingPipline<ListObjectVersionsAsyncInput, ListObjectVersionsAsyncOutput>* listObjectVersions(
        const std::shared_ptr<TosClientBase>& tosClient, const ListObjectVersionsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListObjectVersionsAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<ListObjectVersionsAsyncInput, ListObjectVersionsAsyncOutput>(
            {200}, input, tosClient, nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](ListObjectVersionsAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<ListObjectVersionsAsyncInput>(
                    tosClient, http::MethodGet, input.getBucketName(), "", __func__))
            .responseJson();  // 响应当json处理
    return pipeline;
}

ProcessingPipline<CreateMultipartUploadAsyncInput, CreateMultipartUploadAsyncOutput>* createMultipartUpload(
        const std::shared_ptr<TosClientBase>& tosClient, const CreateMultipartUploadAsyncInput& input,
        const std::function<void(Outcome<TosError, CreateMultipartUploadAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<CreateMultipartUploadAsyncInput, CreateMultipartUploadAsyncOutput>(
            {200}, input, tosClient, nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](CreateMultipartUploadAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<CreateMultipartUploadAsyncInput>(
                    tosClient, http::MethodPost, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<UploadPartAsyncInput, UploadPartAsyncOutput>* uploadPart(
        const std::shared_ptr<TosClientBase>& tosClient, const UploadPartAsyncInput& input,
        const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, UploadPartAsyncOutput>&)>& on_request_done) {
    int part_number = input.getPartNumber();
    auto* pipeline = getBasePipline<UploadPartAsyncInput, UploadPartAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                 on_data_send, on_request_done);
    pipeline->decorateInput([&tosClient](UploadPartAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .transferEncodingCheck()
            .tryEnableCrc64Check(tosClient->getConfig().isEnableCrc())
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<UploadPartAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .decorateOutcome([part_number](Outcome<TosError, UploadPartAsyncOutput>& outcome) {
                outcome.result().setPartNumber(part_number);
            });
    return pipeline;
}

ProcessingPipline<CompleteMultipartUploadAsyncInput, CompleteMultipartUploadAsyncOutput>* completeMultipartUpload(
        const std::shared_ptr<TosClientBase>& tosClient, const CompleteMultipartUploadAsyncInput& input,
        const std::function<void(Outcome<TosError, CompleteMultipartUploadAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<CompleteMultipartUploadAsyncInput, CompleteMultipartUploadAsyncOutput>(
            {200}, input, tosClient, nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](CompleteMultipartUploadAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .requestJson()
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<CompleteMultipartUploadAsyncInput>(
                    tosClient, http::MethodPost, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<UploadPartCopyAsyncInput, UploadPartCopyAsyncOutput>* uploadPartCopyUpload(
        const std::shared_ptr<TosClientBase>& tosClient, const UploadPartCopyAsyncInput& input,
        const std::function<void(Outcome<TosError, UploadPartCopyAsyncOutput>&)>& on_request_done) {
    int part_number = input.getPartNumber();
    auto* pipeline = getBasePipline<UploadPartCopyAsyncInput, UploadPartCopyAsyncOutput>(
            {200}, input, tosClient, nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](UploadPartCopyAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<UploadPartCopyAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .decorateOutcome([part_number](Outcome<TosError, UploadPartCopyAsyncOutput>& outcome) {
                outcome.result().setPartNumber(part_number);
            })
            .responseJson();
    return pipeline;
}

ProcessingPipline<PutSymlinkAsyncInput, PutSymlinkAsyncOutput>* putSymlink(
        const std::shared_ptr<TosClientBase>& tosClient, const PutSymlinkAsyncInput& input,
        const std::function<void(Outcome<TosError, PutSymlinkAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<PutSymlinkAsyncInput, PutSymlinkAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                 nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](PutSymlinkAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .input2HttpRequest(getDefaultInput2HttpRequest<PutSymlinkAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .userName(__func__);
    return pipeline;
}

ProcessingPipline<GetSymlinkAsyncInput, GetSymlinkAsyncOutput>* getSymlink(
        const std::shared_ptr<TosClientBase>& tosClient, const GetSymlinkAsyncInput& input,
        const std::function<void(Outcome<TosError, GetSymlinkAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<GetSymlinkAsyncInput, GetSymlinkAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                 nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](GetSymlinkAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<GetSymlinkAsyncInput>(
                    tosClient, http::MethodGet, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

ProcessingPipline<RenameObjectAsyncInput, RenameObjectAsyncOutput>* renameObject(
        const std::shared_ptr<TosClientBase>& tosClient, const RenameObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, RenameObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<RenameObjectAsyncInput, RenameObjectAsyncOutput>({200, 204}, input, tosClient,
                                                                                     nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](RenameObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<RenameObjectAsyncInput>(
                    tosClient, http::MethodPut, input.getBucketName(), input.getKey(), __func__))
            .responseJson();
    return pipeline;
}

void TosAsyncClient::getObjectAsync(
        const GetObjectAsyncInput& input, const OnDataReceiveWithEvent& on_data_receive,
        const std::function<void(Outcome<TosError, GetObjectAsyncOutput>&)>& on_request_done) const {
    getObject(this->getTosClient(), input, on_data_receive, on_request_done)->asyncExecute();
}

void TosAsyncClient::getObjectToFdRangeAsync(
        const GetObjectAsyncInput& input, FileRange range, FileTransferOptions options,
        const std::function<void(Outcome<TosError, GetObjectAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions_);
    auto sink = CreateFileRangeDownloadSink(std::move(range), std::move(options));
    auto on_data_receive = [sink](char* data, size_t len, AsyncEvent* ev) {
        return sink ? sink->Write(data, len, ev) : static_cast<size_t>(0);
    };
    getObjectAsync(input, on_data_receive,
                   [sink, on_request_done](Outcome<TosError, GetObjectAsyncOutput>& outcome) mutable {
                       FileTransferStatus network_status = FileTransferStatus::Ok();
                       if (!outcome.isSuccess()) {
                           network_status = FileTransferStatus::Failed(outcome.error().String());
                       }
                       if (!sink) {
                           FileTransferResult transfer{FileTransferStatus::Failed("download sink missing")};
                           ApplyDownloadFileTransferResult(outcome, transfer);
                           on_request_done(outcome, transfer);
                           return;
                       }
                       auto outcome_holder =
                           std::make_shared<Outcome<TosError, GetObjectAsyncOutput>>(std::move(outcome));
                       sink->FinishAsync(std::move(network_status),
                                         [sink, outcome_holder, on_request_done](
                                             FileTransferResult transfer) mutable {
                                             ApplyDownloadFileTransferResult(*outcome_holder, transfer);
                                             on_request_done(*outcome_holder, transfer);
                                         });
                   });
}

void TosAsyncClient::getObjectToFileAsync(
        const GetObjectToFileAsyncInput& input,
        const std::function<void(Outcome<TosError, GetObjectToFileAsyncOutput>&)>& on_request_done) const {
    getObjectToFile(this->getTosClient(), fileTransferOptions_, input, on_request_done)->asyncExecute();
}

void TosAsyncClient::putObjectAsync(
        const PutObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, PutObjectAsyncOutput>&)>& on_request_done) const {
    putObject(this->getTosClient(), input, on_data_send, on_request_done)->asyncExecute();
}

void TosAsyncClient::putObjectFromFdRangeAsync(
        const PutObjectAsyncInput& input, FileRange range, FileTransferOptions options,
        const std::function<void(Outcome<TosError, PutObjectAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    const std::uint64_t expected_bytes = range.length;
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions_);
    auto source = CreateFileRangeAsyncDataSource(std::move(range), std::move(options));
    PutObjectAsyncInput upload_input = input;
    upload_input.setContentLength(static_cast<int64_t>(expected_bytes));
    auto on_data_send = [source](char* data, size_t len, AsyncEvent* ev) {
        return source ? source->Read(data, len, ev) : static_cast<size_t>(0);
    };
    putObjectAsync(upload_input, on_data_send,
                   [source, expected_bytes, on_request_done](
                           Outcome<TosError, PutObjectAsyncOutput>& outcome) mutable {
                       FileTransferStatus network_status = FileTransferStatus::Ok();
                       if (!outcome.isSuccess()) {
                           network_status = FileTransferStatus::Failed(outcome.error().String());
                       }
                       FileTransferResult transfer =
                               source ? source->Finish(std::move(network_status))
                                      : FileTransferResult{FileTransferStatus::Failed("upload source missing")};
                       ApplyUploadFileTransferResult(outcome, transfer, expected_bytes);
                       on_request_done(outcome, transfer);
                   });
}

void TosAsyncClient::putObjectFromFileAsync(
        const PutObjectFromFileAsyncInput& input,
        const std::function<void(Outcome<TosError, PutObjectFromFileAsyncOutput>&)>& on_request_done) const {
    putObjectFromFile(this->getTosClient(), fileTransferOptions_, input, on_request_done)->asyncExecute();
}

void TosAsyncClient::modifyObjectAsync(
        const ModifyObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&)>& on_request_done) const {
    modifyObject(this->getTosClient(), input, on_data_send, on_request_done)->asyncExecute();
}

void TosAsyncClient::modifyObjectFromFdRangeAsync(
        const ModifyObjectAsyncInput& input, FileRange range, FileTransferOptions options,
        const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    const std::uint64_t expected_bytes = range.length;
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions_);
    auto source = CreateFileRangeAsyncDataSource(std::move(range), std::move(options));
    ModifyObjectAsyncInput upload_input = input;
    upload_input.setContentLength(static_cast<int64_t>(expected_bytes));
    auto on_data_send = [source](char* data, size_t len, AsyncEvent* ev) {
        return source ? source->Read(data, len, ev) : static_cast<size_t>(0);
    };
    modifyObjectAsync(upload_input, on_data_send,
                      [source, expected_bytes, on_request_done](
                              Outcome<TosError, ModifyObjectAsyncOutput>& outcome) mutable {
                          FileTransferStatus network_status = FileTransferStatus::Ok();
                          if (!outcome.isSuccess()) {
                              network_status = FileTransferStatus::Failed(outcome.error().String());
                          }
                          FileTransferResult transfer =
                                  source ? source->Finish(std::move(network_status))
                                         : FileTransferResult{FileTransferStatus::Failed("modify source missing")};
                          ApplyUploadFileTransferResult(outcome, transfer, expected_bytes);
                          on_request_done(outcome, transfer);
                      });
}

void TosAsyncClient::modifyObjectFromFileAsync(
        const ModifyObjectAsyncInput& input, const std::string& file_path, FileTransferOptions options,
        const std::function<void(Outcome<TosError, ModifyObjectAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    std::uint64_t file_size = 0;
    std::string file_error;
    if (!GetRegularFileSize(file_path, &file_size, &file_error)) {
        FileTransferResult transfer{FileTransferStatus::Failed(file_error)};
        Outcome<TosError, ModifyObjectAsyncOutput> outcome;
        outcome.setSuccess(false);
        outcome.setE(MakeFileTransferError(transfer, "modify object source file is invalid"));
        on_request_done(outcome, transfer);
        return;
    }

    FileRange range;
    range.path = file_path;
    range.offset = 0;
    range.length = file_size;
    modifyObjectFromFdRangeAsync(input, std::move(range), std::move(options), std::move(on_request_done));
}

void TosAsyncClient::appendObjectAsync(
        const AppendObjectAsyncInput& input, const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, AppendObjectAsyncOutput>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase>& tosClient = this->getTosClient();
    const std::string& bucketName = input.getBucketName();
    const std::string& key = input.getKey();
    const uint64_t offset = input.getOffset();
    const int64_t contentLength = input.getContentLength();
    const std::string func_name = __func__;
    TransferEncoding encoding = input.getTransferEncoding();
    LockFreeCache<std::string, BucketCache>& bucketCache = tosClient->getBucketCache();

    auto afterBucketTypeGet = [bucketName, key, on_request_done, this, input, tosClient, func_name, on_data_send,
                               offset, contentLength, encoding](const BucketType bucketType) {
        if (bucketType == BucketType::HNS) {
            Logger::getInstance().info("HNS bucket, do modify: bucketName={}, key={}", bucketName, key);
            auto modifyObjectCallback = [bucketName, key, this, on_request_done](
                                                Outcome<TosError, ModifyObjectAsyncOutput> modifyObjectOutcome) {
                Outcome<TosError, AppendObjectAsyncOutput> res;
                if (!modifyObjectOutcome.isSuccess()) {
                    res.setE(modifyObjectOutcome.error());
                    res.setSuccess(false);
                    on_request_done(res);
                    return;
                }

                auto& lastResult = res.result();
                lastResult.setStatusCode(modifyObjectOutcome.result().getStatusCode());
                lastResult.setRequestId(modifyObjectOutcome.result().getRequestId());
                lastResult.setId2(modifyObjectOutcome.result().getId2());
                lastResult.setContentLength(modifyObjectOutcome.result().getContentLength());
                lastResult.setHashCrc64Ecma(modifyObjectOutcome.result().getHashCrc64Ecma());
                lastResult.setHashCrc32Ecma(modifyObjectOutcome.result().getHashCrc32Ecma());
                lastResult.setLastModified(modifyObjectOutcome.result().getLastModified());
                lastResult.setNextAppendOffset(modifyObjectOutcome.result().getNextModifyOffset());

                res.setSuccess(true);
                on_request_done(res);
            };

            ModifyObjectAsyncInput modifyObjectInput(bucketName, key, offset, encoding);
            modifyObjectInput.setContentLength(contentLength);
            modifyObjectInput.setPreHashCrc64Ecma(input.getPreHashCrc64Ecma());
            this->modifyObjectAsync(modifyObjectInput, on_data_send, modifyObjectCallback);
        } else {
            Logger::getInstance().info("FNS bucket, do append: bucketName={}, key={}", bucketName, key);
            appendObject(tosClient, input, on_data_send, on_request_done)->asyncExecute();
        }
    };

    auto headBucketCallback = [bucketName, this, on_request_done, &bucketCache,
                               afterBucketTypeGet](Outcome<TosError, HeadBucketAsyncOutput> outcome) {
        Logger::getInstance().info("head bucket callback, do next: bucketName={}", bucketName);
        if (!outcome.isSuccess()) {
            Outcome<TosError, AppendObjectAsyncOutput> res;
            res.setE(outcome.error());
            res.setSuccess(false);
            on_request_done(res);
            return;
        }

        BucketCache cache;
        cache.bucketType_ = outcome.result().getBucketType();
        cache.lastUpdateTimeNanos_ = std::chrono::steady_clock::now();
        cache.timeout_ = std::chrono::seconds(15 * 60);
        if (!bucketCache.put(bucketName, cache)) {
            Logger::getInstance().error("put to cache failed: ", bucketName);
        } else {
            Logger::getInstance().warn("put to cache: ", bucketName);
        }

        const auto& result = outcome.result();
        const auto bucketType = result.getBucketType();
        afterBucketTypeGet(bucketType);
    };

    BucketCache cache;
    if (bucketCache.get(bucketName, cache) &&
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                         cache.lastUpdateTimeNanos_) < cache.timeout_) {
        afterBucketTypeGet(cache.bucketType_);
    } else {
        const HeadBucketAsyncInput headBucketInput(bucketName);
        this->headBucketAsync(headBucketInput, headBucketCallback);
    }
}

void TosAsyncClient::appendObjectFromFdRangeAsync(
        const AppendObjectAsyncInput& input, FileRange range, FileTransferOptions options,
        const std::function<void(Outcome<TosError, AppendObjectAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    const std::uint64_t expected_bytes = range.length;
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions_);
    auto source = CreateFileRangeAsyncDataSource(std::move(range), std::move(options));
    AppendObjectAsyncInput upload_input = input;
    upload_input.setContentLength(static_cast<int64_t>(expected_bytes));
    auto on_data_send = [source](char* data, size_t len, AsyncEvent* ev) {
        return source ? source->Read(data, len, ev) : static_cast<size_t>(0);
    };
    appendObjectAsync(upload_input, on_data_send,
                      [source, expected_bytes, on_request_done](
                              Outcome<TosError, AppendObjectAsyncOutput>& outcome) mutable {
                          FileTransferStatus network_status = FileTransferStatus::Ok();
                          if (!outcome.isSuccess()) {
                              network_status = FileTransferStatus::Failed(outcome.error().String());
                          }
                          FileTransferResult transfer =
                                  source ? source->Finish(std::move(network_status))
                                         : FileTransferResult{FileTransferStatus::Failed("append source missing")};
                          ApplyUploadFileTransferResult(outcome, transfer, expected_bytes);
                          on_request_done(outcome, transfer);
                      });
}

void TosAsyncClient::copyObjectAsync(
        const CopyObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, CopyObjectAsyncOutput>&)>& on_request_done) const {
    copyObject(this->getTosClient(), input, on_request_done)->asyncExecute();
}

ProcessingPipline<HeadBucketAsyncInput, HeadBucketAsyncOutput>* headBucket(
        const std::shared_ptr<TosClientBase>& tosClient, const HeadBucketAsyncInput& input,
        const std::function<void(Outcome<TosError, HeadBucketAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<HeadBucketAsyncInput, HeadBucketAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                 nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](HeadBucketAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<HeadBucketAsyncInput>(tosClient, http::MethodHead,
                                                                                 input.getBucketName(), "", __func__));
    // HeadBucket is a HEAD request: bucket metadata is returned in headers and
    // there is no JSON body to parse. Keeping the JSON response decorator here
    // can turn an otherwise successful HTTP 200 HEAD into a client-side failure
    // when a proxy/server sends any incidental body bytes.
    return pipeline;
}

ProcessingPipline<HeadObjectAsyncInput, HeadObjectAsyncOutput>* headObject(
        const std::shared_ptr<TosClientBase>& tosClient, const HeadObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, HeadObjectAsyncOutput>&)>& on_request_done) {
    auto* pipeline = getBasePipline<HeadObjectAsyncInput, HeadObjectAsyncOutput>({200, 203, 206}, input, tosClient,
                                                                                 nullptr, nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](HeadObjectAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(__func__)
            .input2HttpRequest(getDefaultInput2HttpRequest<HeadObjectAsyncInput>(
                    tosClient, http::MethodHead, input.getBucketName(), input.getKey(), __func__));
    return pipeline;
}

void TosAsyncClient::headObjectAsync(
        const HeadObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, HeadObjectAsyncOutput>&)>& on_request_done) const {
    headObject(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::deleteObjectAsync(
        const DeleteObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, DeleteObjectAsyncOutput>&)>& on_request_done) const {
    deleteObject(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::setObjectMetaAsync(
        const SetObjectMetaAsyncInput& input,
        const std::function<void(Outcome<TosError, SetObjectMetaAsyncOutput>&)>& on_request_done) const {
    setObjectMeta(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::createBucketAsync(
        const CreateBucketAsyncInput& input,
        const std::function<void(Outcome<TosError, CreateBucketAsyncOutput>&)>& on_request_done) const {
    createBucket(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::headBucketAsync(
        const HeadBucketAsyncInput& input,
        const std::function<void(Outcome<TosError, HeadBucketAsyncOutput>&)>& on_request_done) const {
    headBucket(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::getBucketTypeAsync(
        const std::string& bucketName,
        const std::function<void(Outcome<TosError, BucketType>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase> tosClient = this->getTosClient();
    LockFreeCache<std::string, BucketCache>& bucketCache = tosClient->getBucketCache();

    BucketCache cache;
    const bool inCache = bucketCache.get(bucketName, cache) &&
                         std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::steady_clock::now() - cache.lastUpdateTimeNanos_) < cache.timeout_;
    if (inCache) {
        Outcome<TosError, BucketType> out;
        out.setR(cache.bucketType_);
        out.setSuccess(true);
        on_request_done(out);
        return;
    }

    const HeadBucketAsyncInput headBucketInput(bucketName);
    // Keep TosClientBase alive until the async HEAD callback updates the shared bucket cache.
    auto headBucketCallback = [bucketName, tosClient, on_request_done](Outcome<TosError, HeadBucketAsyncOutput>& in) {
        Outcome<TosError, BucketType> out;
        if (!in.isSuccess()) {
            out.setE(in.error());
            out.setSuccess(false);
            on_request_done(out);
            return;
        }

        BucketCache newCache;
        newCache.bucketType_ = in.result().getBucketType();
        newCache.lastUpdateTimeNanos_ = std::chrono::steady_clock::now();
        newCache.timeout_ = std::chrono::seconds(15 * 60);
        LockFreeCache<std::string, BucketCache>& bucketCache = tosClient->getBucketCache();
        if (!bucketCache.put(bucketName, newCache)) {
            Logger::getInstance().error("put to cache failed: ", bucketName);
        } else {
            Logger::getInstance().warn("put to cache: ", bucketName);
        }

        out.setR(in.result().getBucketType());
        out.setSuccess(true);
        on_request_done(out);
    };
    this->headBucketAsync(headBucketInput, headBucketCallback);
}

void TosAsyncClient::listBucketsAsync(
        const ListBucketsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListBucketsAsyncOutput>&)>& on_request_done) const {
    listBuckets(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::listObjectsAsync(
        const ListObjectsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListObjectsAsyncOutput>&)>& on_request_done) const {
    listObjects(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::listObjectsType2Async(
        const ListObjectsType2Input& input,
        const std::function<void(Outcome<TosError, ListObjectsType2Output>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase>& tosClient = this->getTosClient();
    auto* pipeline = newBasePipeline<ListObjectsType2Input, ListObjectsType2Output>(input, true);

    auto validate = [](ListObjectsType2Input& ipt) { return isValidBucketName(ipt.getBucket()); };

    auto input_to_http_request = [tosClient](ListObjectsType2Input& ipt) -> std::shared_ptr<HttpRequest> {
        const std::map<std::string, std::string> empty_headers;
        const std::map<std::string, std::string> empty_queries;
        auto rb = tosClient->newBuilder(ipt.getBucket(), "", 0, empty_headers, empty_queries, 0);
        rb.withQuery("list-type", "2");
        rb.withQuery("fetch-owner", "true");
        rb.withQueryCheckEmpty("prefix", ipt.getPrefix());
        rb.withQueryCheckEmpty("delimiter", ipt.getDelimiter());
        rb.withQueryCheckEmpty("start-after", ipt.getStartAfter());
        rb.withQueryCheckEmpty("continuation-token", ipt.getContinuationToken());
        if (ipt.getMaxKeys() != 0) {
            rb.withQueryCheckEmpty("max-keys", std::to_string(ipt.getMaxKeys()));
        }
        rb.withQueryCheckEmpty("encoding-type", ipt.getEncodingType());
        if (ipt.getFetchMeta()) {
            rb.withQuery("fetch-meta", "true");
        }
        return rb.buildHttpRequest(http::MethodGet);
    };

    auto expect_response_to_outcome = [](const std::shared_ptr<HttpRequest>&,
                                         const std::shared_ptr<HttpResponse>& response, TosError&,
                                         ListObjectsType2Output& result) {
        std::string body_text;
        if (response->Body() != nullptr) {
            std::stringstream ss;
            ss << response->Body()->rdbuf();
            body_text = ss.str();
        }
        if (!body_text.empty()) {
            result.fromJsonString(body_text);
        }

        RequestInfo request_info;
        request_info.setRequestId(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_REQUEST_ID));
        request_info.setId2(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_ID_2));
        request_info.setStatusCode(response->statusCode());
        request_info.setHeaders(response->Headers());
        result.setRequestInfo(request_info);
    };

    auto json_to_output = [](ListObjectsType2Output& result, json& j) {
        result.fromJsonString(j.dump());
    };

    pipeline->inputCheck(validate)
            .input2Json([](ListObjectsType2Input&) { return std::string(); })
            .httpRequestSender([tosClient](const std::shared_ptr<HttpRequest>& request,
                                           const OnDataReceiveWithEvent& dr, const OnDataSendWithEvent& sd,
                                           const std::function<void(std::shared_ptr<HttpResponse>)>& fi,
                                           const OnRequestStart& st, const OnHttpStatusSet& sts,
                                           const OnContentLengthSet& cls) {
                (void)tosClient->getAsyncTransport()->send(request, dr, sd, fi, st, sts, cls);
            })
            .onRequestDone(on_request_done)
            .expectStatus({200})
            .defaultUnexpectResponse2Outcome()
            .responseJson()
            .json2Output(json_to_output)
            .expectResponse2Outcome(expect_response_to_outcome)
            .userName(__func__)
            .input2HttpRequest(input_to_http_request);
    pipeline->asyncExecute();
}

void TosAsyncClient::listObjectVersionsAsync(
        const ListObjectVersionsAsyncInput& input,
        const std::function<void(Outcome<TosError, ListObjectVersionsAsyncOutput>&)>& on_request_done) const {
    listObjectVersions(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::createMultipartUploadAsync(
        const CreateMultipartUploadAsyncInput& input,
        const std::function<void(Outcome<TosError, CreateMultipartUploadAsyncOutput>&)>& on_request_done) const {
    createMultipartUpload(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::abortMultipartUploadAsync(
        const AbortMultipartUploadInput& input,
        const std::function<void(Outcome<TosError, AbortMultipartUploadOutput>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase>& tosClient = this->getTosClient();
    auto* pipeline = newBasePipeline<AbortMultipartUploadInput, AbortMultipartUploadOutput>(input, true);

    auto validate = [](AbortMultipartUploadInput& ipt) {
        if (ipt.getBucket().empty()) {
            return std::string("bucket is empty");
        }
        if (ipt.getKey().empty()) {
            return std::string("key is empty");
        }
        if (ipt.getUploadId().empty()) {
            return std::string("uploadId is empty");
        }
        return std::string();
    };

    auto input_to_http_request = [tosClient](AbortMultipartUploadInput& ipt) {
        const std::map<std::string, std::string> empty_headers;
        const std::map<std::string, std::string> empty_queries;
        auto rb = tosClient->newBuilder(ipt.getBucket(), ipt.getKey(), ipt.getRequestDate(), empty_headers,
                                        empty_queries, 0);
        rb.withQuery("uploadId", ipt.getUploadId());
        return rb.buildHttpRequest(http::MethodDelete);
    };

    auto expect_response_to_outcome = [](const std::shared_ptr<HttpRequest>&,
                                         const std::shared_ptr<HttpResponse>& response, TosError&,
                                         AbortMultipartUploadOutput& result) {
        RequestInfo request_info;
        request_info.setRequestId(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_REQUEST_ID));
        request_info.setId2(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_ID_2));
        request_info.setStatusCode(response->statusCode());
        request_info.setHeaders(response->Headers());
        result.setRequestInfo(request_info);
    };

    pipeline->inputCheck(validate)
            .input2Json([](AbortMultipartUploadInput&) { return std::string(); })
            .httpRequestSender([tosClient](const std::shared_ptr<HttpRequest>& request,
                                           const OnDataReceiveWithEvent& dr, const OnDataSendWithEvent& sd,
                                           const std::function<void(std::shared_ptr<HttpResponse>)>& fi,
                                           const OnRequestStart& st, const OnHttpStatusSet& sts,
                                           const OnContentLengthSet& cls) {
                (void)tosClient->getAsyncTransport()->send(request, dr, sd, fi, st, sts, cls);
            })
            .onRequestDone(on_request_done)
            .expectStatus({204})
            .defaultUnexpectResponse2Outcome()
            .expectResponse2Outcome(expect_response_to_outcome)
            .userName(__func__)
            .input2HttpRequest(input_to_http_request);
    pipeline->asyncExecute();
}

void TosAsyncClient::setObjectTimeAsync(
        const SetObjectTimeInput& input,
        const std::function<void(Outcome<TosError, SetObjectTimeOutput>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase>& tosClient = this->getTosClient();
    auto* pipeline = newBasePipeline<SetObjectTimeInput, SetObjectTimeOutput>(input, true);

    auto validate = [](SetObjectTimeInput& ipt) {
        if (ipt.getBucket().empty()) {
            return std::string("bucket is empty");
        }
        if (ipt.getKey().empty()) {
            return std::string("key is empty");
        }
        return std::string();
    };

    auto input_to_http_request = [tosClient](SetObjectTimeInput& ipt) {
        const std::map<std::string, std::string> empty_headers;
        const std::map<std::string, std::string> empty_queries;
        auto rb = tosClient->newBuilder(ipt.getBucket(), ipt.getKey(), 0, empty_headers, empty_queries, 0);
        rb.withQuery("time", "");
        rb.withHeader(HEADER_MODIFY_TIMESTAMP, std::to_string(ipt.getModifyTimestamp().tv_sec));
        rb.withHeader(HEADER_MODIFY_TIMESTAMP_NS, std::to_string(ipt.getModifyTimestamp().tv_nsec));
        return rb.buildHttpRequest(http::MethodPost);
    };

    auto expect_response_to_outcome = [](const std::shared_ptr<HttpRequest>&,
                                         const std::shared_ptr<HttpResponse>& response, TosError&,
                                         SetObjectTimeOutput& result) {
        RequestInfo request_info;
        request_info.setRequestId(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_REQUEST_ID));
        request_info.setId2(MapUtils::findValueByKeyIgnoreCase(response->Headers(), HEADER_ID_2));
        request_info.setStatusCode(response->statusCode());
        request_info.setHeaders(response->Headers());
        result.setRequestInfo(request_info);
    };

    pipeline->inputCheck(validate)
            .input2Json([](SetObjectTimeInput&) { return std::string(); })
            .httpRequestSender([tosClient](const std::shared_ptr<HttpRequest>& request,
                                           const OnDataReceiveWithEvent& dr, const OnDataSendWithEvent& sd,
                                           const std::function<void(std::shared_ptr<HttpResponse>)>& fi,
                                           const OnRequestStart& st, const OnHttpStatusSet& sts,
                                           const OnContentLengthSet& cls) {
                (void)tosClient->getAsyncTransport()->send(request, dr, sd, fi, st, sts, cls);
            })
            .onRequestDone(on_request_done)
            .expectStatus({200})
            .defaultUnexpectResponse2Outcome()
            .expectResponse2Outcome(expect_response_to_outcome)
            .userName(__func__)
            .input2HttpRequest(input_to_http_request);
    pipeline->asyncExecute();
}

void TosAsyncClient::uploadPartAsync(
        const UploadPartAsyncInput& input, const OnDataSendWithEvent& on_data_send,
        const std::function<void(Outcome<TosError, UploadPartAsyncOutput>&)>& on_request_done) const {
    uploadPart(this->getTosClient(), input, on_data_send, on_request_done)->asyncExecute();
}

void TosAsyncClient::uploadPartFromFdRangeAsync(
        const UploadPartAsyncInput& input, FileRange range, FileTransferOptions options,
        const std::function<void(Outcome<TosError, UploadPartAsyncOutput>&, FileTransferResult)>&
                on_request_done) const {
    const std::uint64_t expected_bytes = range.length;
    options = MergeClientFileTransferOptions(std::move(options), fileTransferOptions_);
    auto source = CreateFileRangeAsyncDataSource(std::move(range), std::move(options));
    UploadPartAsyncInput upload_input = input;
    upload_input.setContentLength(static_cast<int64_t>(expected_bytes));
    auto on_data_send = [source](char* data, size_t len, AsyncEvent* ev) {
        return source ? source->Read(data, len, ev) : static_cast<size_t>(0);
    };
    uploadPartAsync(upload_input, on_data_send,
                    [source, expected_bytes, on_request_done](
                            Outcome<TosError, UploadPartAsyncOutput>& outcome) mutable {
                        FileTransferStatus network_status = FileTransferStatus::Ok();
                        if (!outcome.isSuccess()) {
                            network_status = FileTransferStatus::Failed(outcome.error().String());
                        }
                        FileTransferResult transfer =
                                source ? source->Finish(std::move(network_status))
                                       : FileTransferResult{FileTransferStatus::Failed("upload part source missing")};
                        ApplyUploadFileTransferResult(outcome, transfer, expected_bytes);
                        on_request_done(outcome, transfer);
                    });
}

void TosAsyncClient::completeMultipartUploadAsync(
        const CompleteMultipartUploadAsyncInput& input,
        const std::function<void(Outcome<TosError, CompleteMultipartUploadAsyncOutput>&)>& on_request_done) const {
    completeMultipartUpload(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::uploadPartCopyUploadAsync(
        const UploadPartCopyAsyncInput& input,
        const std::function<void(Outcome<TosError, UploadPartCopyAsyncOutput>&)>& on_request_done) const {
    uploadPartCopyUpload(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::putSymlinkAsync(
        const PutSymlinkAsyncInput& input,
        const std::function<void(Outcome<TosError, PutSymlinkAsyncOutput>&)>& on_request_done) const {
    putSymlink(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::getSymlinkAsync(
        const GetSymlinkAsyncInput& input,
        const std::function<void(Outcome<TosError, GetSymlinkAsyncOutput>&)>& on_request_done) const {
    getSymlink(this->getTosClient(), input, on_request_done)->asyncExecute();
}

void TosAsyncClient::renameObjectAsync(
        const RenameObjectAsyncInput& input,
        const std::function<void(Outcome<TosError, RenameObjectAsyncOutput>&)>& on_request_done) const {
    renameObject(this->getTosClient(), input, on_request_done)->asyncExecute();
}

ProcessingPipline<GetFileStatusAsyncInput, GetFileStatusAsyncOutput>* getFileStatus(
        const std::shared_ptr<TosClientBase>& tosClient, const GetFileStatusAsyncInput& input,
        const std::function<void(Outcome<TosError, GetFileStatusAsyncOutput>&)>& on_request_done) {
    const std::string func_name = __func__;
    auto* pipeline = getBasePipline<GetFileStatusAsyncInput, GetFileStatusAsyncOutput>({200}, input, tosClient, nullptr,
                                                                                       nullptr, on_request_done);
    pipeline->decorateInput([&tosClient](GetFileStatusAsyncInput& ipt) {
                ipt.setIsCustomDomain(tosClient->getConfig().isCustomDomain());
            })
            .userName(func_name)
            .input2HttpRequest(getDefaultInput2HttpRequest<GetFileStatusAsyncInput>(
                    tosClient, http::MethodGet, input.getBucketName(), input.getKey(), func_name))
            .responseJson();
    return pipeline;
}

void TosAsyncClient::getFileStatusAsync(
        const GetFileStatusAsyncInput& input,
        const std::function<void(Outcome<TosError, GetFileStatusAsyncOutput>&)>& on_request_done) const {
    const std::shared_ptr<TosClientBase>& tosClient = this->getTosClient();
    const std::string& bucketName = input.getBucketName();
    const std::string& key = input.getKey();

    struct CacheProbe {
        bool hit;
        BucketType bucketType;
        CacheProbe() : hit(false), bucketType(BucketType::FNS) {
        }
    };

    LockFreeCache<std::string, BucketCache>& bucketCache = tosClient->getBucketCache();
    auto getCache = [bucketName, &bucketCache](const Notifier& notifier) {
        CacheProbe probe;
        BucketCache cache;
        const bool inCache = bucketCache.get(bucketName, cache) &&
                             std::chrono::duration_cast<std::chrono::seconds>(
                                     std::chrono::steady_clock::now() - cache.lastUpdateTimeNanos_) < cache.timeout_;
        if (inCache) {
            probe.hit = true;
            probe.bucketType = cache.bucketType_;
        }
        Outcome<TosError, CacheProbe> out;
        out.setR(probe);
        out.setSuccess(true);
        notifier.notify<CacheProbe>(out);
    };

    const auto fromCache = execStepFromValue<CacheProbe, HeadBucketAsyncOutput>(
            [](const CacheProbe& p, Outcome<TosError, HeadBucketAsyncOutput>& out) {
                HeadBucketAsyncOutput hb;
                hb.setBucketType(p.bucketType);
                out.setR(hb);
                out.setSuccess(true);
            });

    const auto headBucketStep =
            execStepFromPipelineGetter<CacheProbe, HeadBucketAsyncInput, HeadBucketAsyncOutput, HeadBucketAsyncOutput>(
                    [bucketName](const CacheProbe&) -> HeadBucketAsyncInput {
                        return HeadBucketAsyncInput(bucketName);
                    },
                    [tosClient](const HeadBucketAsyncInput& ipt,
                                const std::function<void(Outcome<TosError, HeadBucketAsyncOutput>&)>& cb) {
                        return headBucket(tosClient, ipt, cb);
                    },
                    [bucketName, &bucketCache](Outcome<TosError, HeadBucketAsyncOutput>& in,
                                               Outcome<TosError, HeadBucketAsyncOutput>& out) {
                        if (!in.isSuccess()) {
                            out.setE(in.error());
                            out.setSuccess(false);
                            return;
                        }
                        BucketCache newCache;
                        newCache.bucketType_ = in.result().getBucketType();
                        newCache.lastUpdateTimeNanos_ = std::chrono::steady_clock::now();
                        newCache.timeout_ = std::chrono::seconds(15 * 60);
                        if (!bucketCache.put(bucketName, newCache)) {
                            Logger::getInstance().error("put to cache failed: ", bucketName);
                        } else {
                            Logger::getInstance().warn("put to cache: ", bucketName);
                        }
                        out.setR(in.result());
                        out.setSuccess(true);
                    });

    const auto hnsBranch = execStepFromPipelineGetter<HeadBucketAsyncOutput, HeadObjectAsyncInput,
                                                      HeadObjectAsyncOutput, GetFileStatusAsyncOutput>(
            [bucketName, key](const HeadBucketAsyncOutput& /*prev*/) -> HeadObjectAsyncInput {
                return {bucketName, key};
            },
            [tosClient](const HeadObjectAsyncInput& ipt,
                        const std::function<void(Outcome<TosError, HeadObjectAsyncOutput>&)>& cb) {
                return headObject(tosClient, ipt, cb);
            },
            [key](Outcome<TosError, HeadObjectAsyncOutput>& in, Outcome<TosError, GetFileStatusAsyncOutput>& out) {
                if (!in.isSuccess()) {
                    out.setE(in.error());
                    out.setSuccess(false);
                    return;
                }
                const HeadObjectAsyncOutput& head = in.result();
                GetFileStatusAsyncOutput ok;
                ok.setStatusCode(head.getStatusCode());
                ok.setRequestId(head.getRequestId());
                ok.setId2(head.getId2());
                ok.setHeaders(head.getHeaders());
                ok.setKey(key);
                ok.setContentLength(head.getContentLength());
                ok.setSize(head.getContentLength());
                ok.setHashCrc64Ecma(head.getHashCrc64Ecma());
                ok.setHashCrc32Ecma(head.getHashCrc32Ecma());
                ok.setEtag(head.getEtag());
                ok.setLastModified(head.getLastModified());
                ok.setObjectType(head.getObjectType());
                ok.setIsDirectory(head.isDirectory());
                ok.setMeta(head.getMeta());
                out.setR(ok);
                out.setSuccess(true);
            });

    const auto fnsBranch =
            [this, bucketName, key, tosClient](const HeadBucketAsyncOutput& /*prev*/, const Notifier& notifier) {
                auto statInput = GetFileStatusAsyncInput(bucketName, key);
                ProcessingPipline<GetFileStatusAsyncInput, GetFileStatusAsyncOutput>* statPipeline =
                        getFileStatus(
                                tosClient, statInput,
                                [this, bucketName, key, tosClient, notifier](
                                        Outcome<TosError, GetFileStatusAsyncOutput>& statOut) mutable {
                                    if (statOut.isSuccess() || !tosClient->getConfig().tosStatFallback() ||
                                        statOut.error().getStatusCode() != 404) {
                                        notifier.notify<GetFileStatusAsyncOutput>(statOut);
                                        return;
                                    }

                                    std::string listPrefix = key;
                                    if (!listPrefix.empty() && listPrefix.back() != '/') {
                                        listPrefix += "/";
                                    }

                                    auto listInput = ListObjectsType2Input(bucketName);
                                    listInput.setPrefix(listPrefix);
                                    listInput.setMaxKeys(1);
                                    listInput.setListOnlyOnce(true);
                                    listInput.setFetchMeta(true);

                                    this->listObjectsType2Async(
                                            listInput,
                                            [notifier](Outcome<TosError, ListObjectsType2Output>& listOut) mutable {
                                                Outcome<TosError, GetFileStatusAsyncOutput> out;
                                                if (!listOut.isSuccess()) {
                                                    out.setE(listOut.error());
                                                    out.setSuccess(false);
                                                    notifier.notify<GetFileStatusAsyncOutput>(out);
                                                    return;
                                                }

                                                const auto& contents = listOut.result().getContents();
                                                if (contents.empty()) {
                                                    TosError error;
                                                    error.setStatusCode(404);
                                                    error.setMessage("object not found");
                                                    out.setE(error);
                                                    out.setSuccess(false);
                                                    notifier.notify<GetFileStatusAsyncOutput>(out);
                                                    return;
                                                }

                                                const auto& requestInfo = listOut.result().getRequestInfo();
                                                const auto& listedObject = contents.front();
                                                GetFileStatusAsyncOutput ok;
                                                ok.setStatusCode(requestInfo.getStatusCode());
                                                ok.setRequestId(requestInfo.getRequestId());
                                                ok.setId2(requestInfo.getId2());
                                                ok.setHeaders(requestInfo.getHeaders());
                                                ok.setKey(listedObject.getKey());
                                                ok.setSize(listedObject.getSize());
                                                ok.setHashCrc64Ecma(listedObject.getHashCrc64Ecma());
                                                ok.setEtag(listedObject.getETag());
                                                ok.setLastModified(listedObject.getLastModified());
                                                ok.setObjectType(listedObject.getObjectType());
                                                ok.setMeta(listedObject.getMeta());
                                                out.setR(ok);
                                                out.setSuccess(true);
                                                notifier.notify<GetFileStatusAsyncOutput>(out);
                                            });
                                });
                if (statPipeline) {
                    statPipeline->asyncExecute();
                }
            };

    const auto coordinator = PiplineCoordinator::withPiplineRelease();
    coordinator->begin<CacheProbe>(getCache)
            .thenIf<HeadBucketAsyncOutput>([](const CacheProbe& p) { return p.hit; }, fromCache)
            .else_(headBucketStep)
            .thenIf<GetFileStatusAsyncOutput>(
                    [](const HeadBucketAsyncOutput& out) -> bool { return out.getBucketType() == BucketType::HNS; },
                    hnsBranch)
            .else_(fnsBranch);
    coordinator->run<GetFileStatusAsyncOutput>(on_request_done);
}

static void setContentType(RequestBuilder& rb, const std::string& objectKey) {
    std::string contentType = rb.findHeader(http::HEADER_CONTENT_TYPE);
    if (contentType.empty()) {
        contentType = MapUtils::findValueByKeyIgnoreCase(rb.getRequestHeader(), http::HEADER_CONTENT_TYPE);
    }
    if (contentType.empty()) {
        if (rb.isAutoRecognizeContentType()) {
            contentType = MimeType::getMimetypeByObjectKey(objectKey);
            rb.withHeader(http::HEADER_CONTENT_TYPE, contentType);
        }
    }
}
