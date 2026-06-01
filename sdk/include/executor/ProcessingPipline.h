#pragma once
// 修改说明（Aime）：
// - 引入 executor/PipelineBase.h 的 AnyOutcome
// - 新增 onAnyRequestDone(std::function<void(AnyOutcome)>) 用于协调器的类型擦除回调桥接
// - 在 handleResponse 中于 typed 回调之后触发 AnyOutcome 回调，保持向后兼容
#include "Outcome.h"
#include "PipelineBase.h"
#include "TosError.h"
#include "TosRequest.h"
#include "TosClientTemplate.h"
#include "logger/logger.h"
#include "transport/http/HttpRequest.h"
#include "transport/http/HttpResponse.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace VolcengineTos {
using json = nlohmann::json;

namespace executor_detail {
inline void appendMessage(std::ostringstream&) {
}

template <typename T, typename... Args>
void appendMessage(std::ostringstream& stream, const T& value, const Args&... args) {
    stream << value;
    appendMessage(stream, args...);
}

template <typename... Args>
std::string makeMessage(const Args&... args) {
    std::ostringstream stream;
    appendMessage(stream, args...);
    return stream.str();
}
}  // namespace executor_detail

struct FileContext {
    std::string file_path;         // 文件路径
    int file_fd = -1;              // 文件描述符（默认无效）
    void* mmap_ptr = nullptr;      // mmap映射指针
    size_t mmap_size = 0;          // 映射大小（对齐后）
    size_t file_offset = 0;        // 当前偏移
    size_t total_file_length = 0;  // 总文件长度（已知时有效，0表示未知）
    int status = 0;                // 状态（0=正常，-1=错误）
    size_t last_sync_offset = 0;   // 上次同步偏移（用于批量msync）写文件时生效
    constexpr static size_t sync_threshold = 64 * 1024 * 1024;  // 批量同步阈值（64MB）
    bool is_released = false;                                   // 资源释放状态标记

    // 重置上下文（重复调用enableWrite2File时复用）
    void reset(const std::string& path, const size_t file_len = 0) {
        // 释放之前的资源（防止重复调用导致泄漏）
        if (mmap_ptr != MAP_FAILED && mmap_ptr != nullptr) {
            munmap(mmap_ptr, mmap_size);
            mmap_ptr = nullptr;
        }
        if (file_fd != -1) {
            close(file_fd);
            file_fd = -1;
        }
        // 重置状态
        file_path = path;
        file_offset = 0;
        mmap_size = 0;
        status = 0;
        last_sync_offset = 0;
        total_file_length = file_len;  // 已知长度则赋值，否则0
        is_released = false;
    }

    void release_resources() {
        if (is_released) return;  // 已释放，直接返回，避免重复

        // 1. 最终同步（仅主动释放时需要，析构兜底时也建议保留）
        if (mmap_ptr != MAP_FAILED && mmap_ptr != nullptr) {
            msync(mmap_ptr, mmap_size, MS_SYNC);
        }

        // 2. 释放mmap
        if (mmap_ptr != MAP_FAILED && mmap_ptr != nullptr) {
            munmap(mmap_ptr, mmap_size);
            mmap_ptr = nullptr;
            Logger::getInstance().debug("release_resources: munmap success");
        }

        // 3. 关闭文件
        if (file_fd != -1) {
            close(file_fd);
            file_fd = -1;
            Logger::getInstance().debug("release_resources: close file success");
        }

        // 4. 标记为已释放
        is_released = true;
        file_offset = 0;
        mmap_size = 0;
        status = 0;
        last_sync_offset = 0;
    }

    void clear() {
        release_resources();
        file_path.clear();
        mmap_ptr = nullptr;
        mmap_size = 0;
        file_offset = 0;
        total_file_length = 0;
        status = 0;
        last_sync_offset = 0;
        is_released = true;
    }

    // 析构：确保资源释放（防止异常时泄漏）
    ~FileContext() {
        if (!is_released) {  // 仅未释放时才兜底
            //            Logger::getInstance().warn("FileWriteContext destructor: released
            //            (afterPiplineFinish not called?)");
            release_resources();  // 调用统一释放函数
        }
    }
};

template <typename T, typename O>
class ProcessingPipline {
    using InputCheck = std::function<std::string(T&)>;

    using Input2HttpRequest = std::function<std::shared_ptr<HttpRequest>(T&)>;

    using TosRequest2HttpRequest = std::function<std::shared_ptr<HttpRequest>(std::shared_ptr<TosRequest>)>;

    using OnResponseFinish = std::function<void(std::shared_ptr<HttpResponse>)>;

    using HttpRequestSender = std::function<void(
        std::shared_ptr<HttpRequest> request, const OnDataReceiveWithEvent& on_data_receive,
        const OnDataSendWithEvent& on_data_send, const OnResponseFinish& on_request_finished,
        const OnRequestStart& on_request_start, const OnHttpStatusSet& on_http_status_set,
        const OnContentLengthSet& on_content_length_set)>;

    using HttpResponse2OutCome = std::function<void(
        std::shared_ptr<HttpRequest>, std::shared_ptr<HttpResponse>, TosError& tos_error, O& result)>;

    using OutComeDecorator = std::function<void(Outcome<TosError, O>&)>;

    using InputDecorator = std::function<void(T&)>;

    using HttpRequestDecorator = std::function<void(std::shared_ptr<HttpRequest>)>;

    using HttpResponse2Retry = std::function<bool(std::shared_ptr<HttpResponse>, int tryCount, int maxTry)>;

    using OutcomeCallback = std::function<void(Outcome<TosError, O>&)>;

    using AfterPiplineFinish = std::function<void()>;

    using DestroyCallback = std::function<void()>;

    using Json2Output = std::function<void(O&, json&)>;

    using Input2Json = std::function<std::string(T&)>;

 public:
    explicit ProcessingPipline(const T& base_input) : input_(base_input), start_time_ms_(0) {
        expect_status_ = {200};
    };

    ~ProcessingPipline() {
        if (on_destroy_) {
            on_destroy_();
        }
    }

    T& getInput() { return input_; }

    ProcessingPipline& input2Json(const Input2Json& function) {
        input2_json_ = function;
        return *this;
    }

    ProcessingPipline& json2Output(const Json2Output& function) {
        json2output_ = function;
        return *this;
    }

    ProcessingPipline& onDestroy(const DestroyCallback& function) {
        on_destroy_ = function;
        return *this;
    }

    ProcessingPipline& input2HttpRequest(const Input2HttpRequest& function) {
        input2_http_request_ = function;
        return *this;
    }

    ProcessingPipline& httpRequestSender(const HttpRequestSender& function) {
        sender_ = function;
        return *this;
    }

    ProcessingPipline& userName(const std::string& func_name) {
        user_name_ = func_name;
        return *this;
    }

    std::string& getUserName() { return user_name_; }

    ProcessingPipline& OnDataReceiveWithEvent(const OnDataReceiveWithEvent& function) {
        if (function != nullptr) {
            on_data_receive_ = [this, function](char* data, const size_t length, AsyncEvent* ev) {
                const size_t bytes = function(data, length, ev);
                flow_bytes_ += static_cast<int64_t>(bytes);
                return bytes;
            };
        }
        return *this;
    }

    ProcessingPipline& OnDataSendWithEvent(const OnDataSendWithEvent& function) {
        if (function != nullptr) {
            on_data_send_ = function;
        }
        return *this;
    }

    ProcessingPipline& decorateOutcome(const OutComeDecorator& function) {
        auto old_dec = out_come_decorator_;
        auto new_dec = [old_dec, function](Outcome<TosError, O>& outcome) {
            if (old_dec) {
                old_dec(outcome);
            }
            function(outcome);
        };
        out_come_decorator_ = new_dec;
        return *this;
    }

    ProcessingPipline& decorateInput(const InputDecorator& function) {
        auto old_dec = input_decorator_;
        auto new_dec = [old_dec, function](T& input) {
            if (old_dec) {
                old_dec(input);
            }
            function(input);
        };
        input_decorator_ = new_dec;
        return *this;
    }

    ProcessingPipline& decorateHttpRequest(const HttpRequestDecorator& function) {
        auto old_dec = http_request_decorator_;
        auto new_dec = [old_dec, function](const std::shared_ptr<HttpRequest>& req) {
            if (old_dec) {
                old_dec(req);
            }
            function(req);
        };
        http_request_decorator_ = new_dec;
        return *this;
    }

    // todo:在不期待的code情况下，作为json解析body
    ProcessingPipline& responseJson(int size = 64 * 1024) {
        if (StringUtils::startsWithIgnoreCase(user_name_, "list")) {
            size = 1 * 1024 *
                   1024;  // 默认值增加，实际还是根据contentLength处理，但是如果服务端没携带，默认值尽量给大
        }

        response_buffer_limit_ = static_cast<size_t>(size);
        response_buffer_truncated_ = false;

        on_content_length_set_ = [this, size](const int64_t length) {
            if (length > 0) {
                // 以 Content-Length 为准（避免默认 size 过小导致 JSON 截断）
                response_buffer_limit_ = static_cast<size_t>(length);
                this->response_buffer_.reserve(static_cast<size_t>(length) + 1);
            } else {
                response_buffer_limit_ = static_cast<size_t>(size);
                this->response_buffer_.reserve(size);
            }
        };

        on_data_receive_ = [this, size](const char* data, const size_t length, AsyncEvent* ev) {
            if (!data || length == 0) {
                return static_cast<size_t>(0);  // 无有效数据，返回0
            }

            const size_t old_size = this->response_buffer_.size();
            const size_t new_size = old_size + length;
            const size_t limit =
                response_buffer_limit_ > 0 ? response_buffer_limit_ : static_cast<size_t>(size);
            if (new_size > limit) {
                flow_bytes_ += static_cast<int64_t>(length);
                response_buffer_truncated_ = true;
                return length;  // 继续消费数据，避免影响 curl；但标记为截断，后续不再解析
            }

            // 若超过预分配容量，vector会自动扩容
            this->response_buffer_.resize(new_size);

            memcpy(&this->response_buffer_[old_size], data, length);
            flow_bytes_ += static_cast<int64_t>(length);

            return length;
        };

        auto function = [this](Outcome<TosError, O>& outcome) {
            O& result = outcome.result();
            TosError& error = outcome.error();

            if (response_buffer_.empty()) {
                return;
            }

            if (response_buffer_truncated_) {
                outcome.setSuccess(false);
                error.setIsClientError(true);
                error.setCode("ResponseBodyTooLarge");
                error.setMessage("response json truncated: response body exceeds buffer limit");
                return;
            }

            try {
                // allow_exceptions=false: 解析失败时返回 discarded json，避免抛异常
                json j = json::parse(response_buffer_.data(),
                                     response_buffer_.data() + response_buffer_.size(), nullptr, false);
                if (j.is_discarded()) {
                    outcome.setSuccess(false);
                    error.setIsClientError(true);
                    error.setCode("JsonParseError");
                    error.setMessage("invalid json in response body");
                    return;
                }

                if (json2output_) {
                    json2output_(result, j);
                }

                if (j.contains("Code")) {
                    const auto code = j.at("Code").get<std::string>();
                    error.setCode(code);
                }
                if (j.contains("Message")) {
                    error.setIsClientError(false);
                    const auto msg = j.at("Message").get<std::string>();
                    error.setMessage(msg);
                }
                if (j.contains("RequestId")) {
                    const auto request_id = j.at("RequestId").get<std::string>();
                    error.setRequestId(request_id);
                }
                if (j.contains("HostId")) {
                    const auto host_id = j.at("HostId").get<std::string>();
                    error.setHostId(host_id);
                }
                if (j.contains("Condition")) {
                    const auto cond = j.at("Condition").get<std::string>();
                    error.setCondition(cond);
                }
            } catch (const std::exception& e) {
                outcome.setSuccess(false);
                error.setIsClientError(true);
                error.setCode("JsonParseError");
                error.setMessage(e.what());
            } catch (...) {
                outcome.setSuccess(false);
                error.setIsClientError(true);
                error.setCode("JsonParseError");
                error.setMessage("unknown exception while parsing response json");
            }
        };

        decorateOutcome(function);
        return *this;
    }

    ProcessingPipline& requestJson() {
        std::string json_str;
        if (input2_json_) {
            json_str = input2_json_(input_);
        }

        const size_t json_str_len = json_str.size();
        std::shared_ptr<size_t> offset = std::make_shared<size_t>(0);

        on_data_send_ = [json_str, json_str_len, offset](char* data, const size_t length, AsyncEvent* ev) {
            if (!data || length == 0) {
                return static_cast<size_t>(0);
            }
            if (*offset >= json_str_len) {
                return static_cast<size_t>(0);
            }

            const size_t remaining = json_str_len - *offset;
            size_t copy_len = std::min(remaining, length);

            if (copy_len > 0) {
                memcpy(data, json_str.data() + *offset, copy_len);
                *offset += copy_len;
            }

            return copy_len;
        };

        auto json_len_set = [json_str_len](const std::shared_ptr<HttpRequest>& request) -> void {
            request->setContentLength(
                static_cast<int64_t>(json_str_len));  // 无需算签 todo:应该挪到rb构建时
        };

        decorateHttpRequest(json_len_set);
        auto old_request_start = on_request_start_;
        on_request_start_ = [old_request_start, offset]() {
            *offset = 0;
            if (old_request_start) {
                old_request_start();
            }
        };
        return *this;
    }

    ProcessingPipline& onRequestDone(const OutcomeCallback& function) {
        auto old_dec = on_request_done_;
        auto new_dec = [old_dec, function](Outcome<TosError, O>& outcome) {
            if (old_dec) {
                old_dec(outcome);
            }
            function(outcome);
        };
        on_request_done_ = new_dec;
        return *this;
    }

    ProcessingPipline& defaultTosRequest2HttpRequest() {
        tos_request2_http_request_ = [](const std::shared_ptr<TosRequest>& request) {
            auto httpReq = std::make_shared<HttpRequest>(request->getMethod(), nullptr);

            if (request->getFileContent() != nullptr) {
                httpReq->setResponseOutput(request->getFileContent());
            }

            httpReq->setUrl(request->toUrl());
            httpReq->setHeaders(request->getHeaders());
            httpReq->setMethod(request->getMethod());
            httpReq->setBody(request->getContent());
            httpReq->setContentLength(request->getContentLength());
            httpReq->setDataTransferListener(request->getDataTransferListener());
            httpReq->setRateLimiter(request->getRataLimiter());
            httpReq->setCheckHighLatency(request->isCheckHighLatency());
            return httpReq;
        };
        return *this;
    }

    ProcessingPipline& unexpectResponse2Outcome(const HttpResponse2OutCome& function) {
        unexpect_response2_out_come_ = function;
        return *this;
    }

    ProcessingPipline& defaultUnexpectResponse2Outcome() {
        unexpect_response2_out_come_ = [](const std::shared_ptr<HttpRequest>& request,
                                          const std::shared_ptr<HttpResponse>& response, TosError& se,
                                          O& result) {
            // check error
            const auto headers = response->Headers();

            const std::string requestUrl = request->url().toString();
            se.setRequestUrl(requestUrl);
            if (se.getRequestId().empty()) {
                se.setRequestId(getRequestID(headers));
            }
            se.setStatusCode(response->statusCode());
            se.setEc(getEcCode(headers));
            se.setCode("UnexpectedStatusCode error");
            se.setMessage(response->statusMsg());
            se.setCurlErrCode(response->getCurlErrCode());

            RequestInfo reqInfo;
            reqInfo.setId2(getRequestId2(headers));
            reqInfo.setHeaders(headers);
            reqInfo.setStatusCode(response->statusCode());
            se.setRequestInfo(reqInfo);

            if (response->statusMsg() == "operation timeout") {
                se.setIsClientError(true);
                se.setMessage("http request timeout");
                se.setCode("operation timeout");
            }

            if (response->statusCode() == 404) {
                se.setCode("not found");
            }

            if (response->status() != 0) {
                se.setMessage(response->statusMsg());
                if (response->status() == -2) {
                    se.setIsClientError(true);
                }
                se.setCurlErrCode(response->getCurlErrCode());
            }

            if (se.getRequestId().empty() && se.getMessage().empty() && se.getCode().empty() &&
                se.getHostId().empty()) {
                // 完全无信息认为是服务端错误
                se.setMessage("no error info in headers");
                se.setIsClientError(false);
            }
        };
        return *this;
    }

    ProcessingPipline& expectResponse2Outcome(const HttpResponse2OutCome& function) {
        expect_response2_out_come_ = function;
        return *this;
    }

    ProcessingPipline& afterPiplineFinish(const AfterPiplineFinish& function) {
        if (function != nullptr) {
            after_pipline_finishes_.push_back(function);
        }
        return *this;
    }

    void resetForReuse(const T& input) {
        input_ = input;
        resetRuntimeState();
        on_destroy_ = nullptr;
    }

    void recycle() { resetRuntimeState(); }

    ProcessingPipline& inputCheck(const InputCheck& function) {
        input_checks_.push_back(function);
        return *this;
    }

    ProcessingPipline& transferEncodingCheck() {
        auto function = [](const T& ipt) {
            const TransferEncoding transfer_encoding = ipt.getTransferEncoding();
            if (transfer_encoding == TransferEncoding::ContentLength) {
                if (ipt.getContentLength() < 0) {
                    return "invalid content length.";
                }
            } else if (transfer_encoding == TransferEncoding::Identity ||
                       transfer_encoding == TransferEncoding::None) {
                return "unsupported transfer encoding.";
            }

            return "";
        };
        input_checks_.push_back(function);
        return *this;
    }

    ProcessingPipline& tryEnableCrc64Check(const bool enable) {
        if (!enable) {
            return *this;
        }
        auto old_sender = sender_;
        auto crc_sender = [this, old_sender](
                              const std::shared_ptr<HttpRequest>& request,
                              const ::VolcengineTos::OnDataReceiveWithEvent& on_data_receive,
                              const ::VolcengineTos::OnDataSendWithEvent& on_data_send,
                              const std::function<void(std::shared_ptr<HttpResponse>)>& on_request_finished,
                              const OnRequestStart& on_request_start,
                              const OnHttpStatusSet& on_http_status_set,
                              const OnContentLengthSet& on_content_length_set) -> void {
            if (request->Headers().count("Range") == 0) {
                request->setCheckCrc64(true);
                auto function = [this](Outcome<TosError, O>& outcome) {
                    O& result = outcome.result();
                    TosError& error = outcome.error();
                    if (result.getHashCrc64Ecma() > 0 &&
                        result.getCalHashCrc64Ecma() != result.getHashCrc64Ecma()) {
                        outcome.setSuccess(false);
                        error.setIsClientError(true);
                        error.setMessage("Check CRC failed: CRC checksum of client is mismatch with tos");
                        Logger::getInstance().error(
                            "Check CRC failed: CRC checksum of client is mismatch with tos: ",
                            result.getHashCrc64Ecma(), " <->calculated: ", result.getCalHashCrc64Ecma());
                    } else {
                        Logger::getInstance().info("hash crc64 check success: ", result.getHashCrc64Ecma());
                    }
                };
                decorateOutcome(function);
            }
            request->setPreHashCrc64Ecma(input_.getPreHashCrc64Ecma());

            if (old_sender) {
                old_sender(request, on_data_receive, on_data_send, on_request_finished, on_request_start,
                           on_http_status_set, on_content_length_set);
            } else {
            }
        };
        sender_ = crc_sender;
        return *this;
    }

    ProcessingPipline& ignoreInputCheck() {
        ignore_input_check_ = true;
        return *this;
    }

    ProcessingPipline& expectStatus(const std::vector<int>& expectStatus) {
        expect_status_ = expectStatus;
        return *this;
    }

    // todo:打开文件和发送不在一个线程，callback error逻辑不太好，修改
    ProcessingPipline& enableReadFromFile(const std::string& filePath) {
        std::string init_err_msg;
        file_ctx_.reset(filePath);
        // 1. 打开文件（只读模式，O_RDONLY）
        file_ctx_.file_fd = open(filePath.c_str(), O_RDONLY);
        if (file_ctx_.file_fd == -1) {
            init_err_msg = executor_detail::makeMessage(
                    "enableReadFromFile: open file failed, path: ", filePath, ", err: ", strerror(errno));
            file_ctx_.status = -1;
            Logger::getInstance().error(init_err_msg);
            return *this;
        }

        // 2. 获取文件总长度（用户未传入时通过stat获取）
        struct stat file_stat {};
        // fstat通过fd获取文件状态（比stat更安全，避免路径被篡改）
        if (fstat(file_ctx_.file_fd, &file_stat) == -1) {
            init_err_msg = executor_detail::makeMessage(
                    "enableReadFromFile: fstat failed, path: ", filePath, ", err: ", strerror(errno));
            file_ctx_.status = -1;
            Logger::getInstance().error(init_err_msg);
            return *this;
        }

        auto fileLength = static_cast<size_t>(file_stat.st_size);
        file_ctx_.total_file_length = fileLength;

        auto file_len_set = [fileLength](const std::shared_ptr<HttpRequest>& request) -> void {
            request->setContentLength(static_cast<int64_t>(fileLength));  // 无需算签 todo:应该挪到rb构建时
        };

        decorateHttpRequest(file_len_set);

        on_request_start_ = [this, init_err_msg]() {
            auto& ctx = file_ctx_;

            if (ctx.status != 0) {
                callbackError(init_err_msg);
                ctx.release_resources();
                return;
            }

            // 3. 检查文件长度合法性
            if (ctx.total_file_length == 0) {
                const std::string err_msg =
                        executor_detail::makeMessage("enableReadFromFile: file is empty, path: ",
                                                     ctx.file_path);
                ctx.status = -1;
                Logger::getInstance().error(err_msg);
                callbackError(err_msg);
                ctx.release_resources();  // 释放fd
                return;
            }

            // 4. 计算对齐后的映射大小（mmap长度必须是页大小整数倍）
            ctx.mmap_size = align_to_page(ctx.total_file_length);
            Logger::getInstance().debug("enableReadFromFile: file length: ", ctx.total_file_length,
                                        ", aligned mmap size: ", ctx.mmap_size);

            // 5. mmap映射整个文件（读取权限：PROT_READ，共享映射：MAP_SHARED）
            ctx.mmap_ptr = mmap(nullptr,        // 内核自动分配地址
                                ctx.mmap_size,  // 对齐后的映射长度
                                PROT_READ,      // 只读权限（安全，避免误写文件）
                                MAP_SHARED,  // 共享映射（无需写，仅为兼容POSIX，也可改用MAP_PRIVATE）
                                ctx.file_fd,  // 文件描述符
                                0             // 偏移量（从文件起始位置映射）
            );

            // 6. 检查mmap是否成功
            if (ctx.mmap_ptr == MAP_FAILED) {
                const std::string err_msg = executor_detail::makeMessage(
                        "enableReadFromFile: mmap failed, path: ", ctx.file_path, ", err: ", strerror(errno));
                ctx.status = -1;
                Logger::getInstance().error(err_msg);
                callbackError(err_msg);
                ctx.release_resources();  // 释放fd
                return;
            }

            Logger::getInstance().debug("enableReadFromFile: init success, file: ", ctx.file_path);
        };

        on_data_send_ = [this](char* data, size_t len, AsyncEvent* ev) {
            auto& ctx = file_ctx_;

            // 检查初始化状态（异常则返回0，停止发送）
            if (ctx.status != 0 || ctx.file_fd == -1 || ctx.mmap_ptr == MAP_FAILED ||
                ctx.mmap_ptr == nullptr) {
                Logger::getInstance().warn("on_data_send: invalid read state, file: ", ctx.file_path);
                return len;
            }

            // 检查缓冲区合法性
            if (data == nullptr || len == 0) {
                Logger::getInstance().warn("on_data_send: empty buffer, skip");
                return len;
            }

            // 计算剩余可读取长度（读取完成则返回0）
            const size_t remaining_len = ctx.total_file_length - ctx.file_offset;
            if (remaining_len == 0) {
                Logger::getInstance().debug("on_data_send: read complete, total length: ",
                                            ctx.total_file_length);
                return len;  // 返回0表示读取完成，框架停止调用
            }

            // 实际读取长度 = 取缓冲区长度和剩余长度的较小值
            if (len > remaining_len) {
                const std::string err_msg = executor_detail::makeMessage(
                        "enableReadFromFile: data oversize remaining: ", len, ", remain: ", remaining_len);
                callbackError(err_msg);
                ctx.status = -1;
                ctx.release_resources();  // 释放资源
                return len;
            }

            size_t read_len = std::min(len, remaining_len);

            // 从mmap映射内存拷贝数据到框架提供的缓冲区
            // mmap_ptr是const，拷贝到data（非const）符合读取逻辑
            memcpy(data, static_cast<const char*>(ctx.mmap_ptr) + ctx.file_offset, read_len);

            // 更新读取偏移
            ctx.file_offset += read_len;

            Logger::getInstance().debug("on_data_send: read ", read_len,
                                        " bytes, offset: ", ctx.file_offset,
                                        ", remaining: ", ctx.total_file_length - ctx.file_offset);

            return len;  // 返回实际读取长度，框架会继续调用直到返回0
        };

        afterPiplineFinish([this]() {
            auto& ctx = file_ctx_;
            ctx.release_resources();  // 调用统一释放函数
        });

        return *this;
    }

    ProcessingPipline& enableWrite2File(const std::string& filePath) {
        on_content_length_set_ = [this, filePath](const int64_t contentLength) {
            file_ctx_.reset(filePath, contentLength);
            auto& ctx = file_ctx_;

            // 打开文件（O_TRUNC：覆盖写入，需追加则改为O_APPEND并调整offset）
            ctx.file_fd = open(ctx.file_path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
            if (ctx.file_fd == -1) {
                const std::string err_msg = executor_detail::makeMessage(
                        "enableWrite2File: open file failed, path: ", ctx.file_path, ", err: ",
                        strerror(errno));
                ctx.status = -1;
                Logger::getInstance().error(err_msg);
                callbackError(err_msg);
                ctx.release_resources();  // 统一释放资源（即使fd打开失败也兼容）
                return;
            }

            // 分两种模式初始化mmap
            if (ctx.total_file_length > 0) {
                // 模式1：已知文件长度 → 一次映射到位（无需extendMmap）
                ctx.mmap_size = align_to_page(ctx.total_file_length);
                Logger::getInstance().debug("known file length: ", ctx.total_file_length,
                                            ", aligned mmap size: ", ctx.mmap_size);

                // 扩展文件到总长度（确保文件大小足够）
                if (ftruncate(ctx.file_fd, static_cast<off_t>(ctx.mmap_size)) == -1) {
                    const std::string err_msg = executor_detail::makeMessage(
                            "enableWrite2File: ftruncate file failed, path: ", ctx.file_path,
                            ", err: ", strerror(errno));
                    ctx.status = -1;
                    Logger::getInstance().error(err_msg);
                    callbackError(err_msg);
                    ctx.release_resources();  // 统一释放（fd+可能的部分资源）
                    return;
                }

                // 一次映射整个文件
                ctx.mmap_ptr = mmap(nullptr, ctx.mmap_size, PROT_WRITE, MAP_SHARED, ctx.file_fd, 0);
                if (ctx.mmap_ptr == MAP_FAILED) {
                    const std::string err_msg = executor_detail::makeMessage(
                            "enableWrite2File: mmap failed (known length), path: ", ctx.file_path,
                            ", err: ", strerror(errno));
                    ctx.status = -1;
                    Logger::getInstance().error(err_msg);
                    callbackError(err_msg);
                    ctx.release_resources();  // 释放fd+清空映射状态
                    return;
                }
            } else {
                // 模式2：未知文件长度 → 初始映射64MB（减少扩展次数）
                const size_t init_mmap_size = align_to_page(64 * 1024 * 1024);  // 64MB初始大小
                if (extendMmap(ctx.file_fd, ctx.mmap_ptr, ctx.mmap_size, ctx.file_offset, init_mmap_size) !=
                    0) {
                    const std::string err_msg = executor_detail::makeMessage(
                            "enableWrite2File: mmap init failed (unknown length), path: ", ctx.file_path);
                    ctx.status = -1;
                    Logger::getInstance().error(err_msg);
                    callbackError(err_msg);
                    ctx.release_resources();  // 释放fd+清空状态
                    return;
                }
            }

            Logger::getInstance().debug("mmap init success, file: ", ctx.file_path,
                                        ", mmap size: ", ctx.mmap_size);
        };

        on_data_receive_ = [this](char* data, size_t len, AsyncEvent* ev) {
            auto& ctx = file_ctx_;

            // 检查初始化状态
            if (ctx.status != 0 || ctx.file_fd == -1 || ctx.mmap_ptr == MAP_FAILED ||
                ctx.mmap_ptr == nullptr) {
                Logger::getInstance().error("writeCallback: invalid state, file: ", ctx.file_path);
                return len;
            }

            // 检查数据合法性
            if (data == nullptr || len == 0) {
                Logger::getInstance().warn("writeCallback: empty data, skip");
                return len;
            }

            // 已知文件长度：检查是否超出总长度（防止写入越界）
            if (ctx.total_file_length > 0) {
                if (ctx.file_offset + len > ctx.total_file_length) {
                    const std::string err_msg = executor_detail::makeMessage(
                            "on_data_receive: exceed total file length, path: ", ctx.file_path,
                            ", current offset: ", ctx.file_offset, ", len: ", len,
                            ", total: ", ctx.total_file_length);
                    Logger::getInstance().error(err_msg);
                    callbackError(err_msg);
                    ctx.status = -1;
                    ctx.release_resources();  // 释放资源
                    return len;
                }
            } else {
                // 未知文件长度：不足则扩展
                if (ctx.file_offset + len > ctx.mmap_size) {
                    if (extendMmap(ctx.file_fd, ctx.mmap_ptr, ctx.mmap_size, ctx.file_offset, len) != 0) {
                        const std::string err_msg = executor_detail::makeMessage(
                                "on_data_receive: mmap extend failed, path: ", ctx.file_path,
                                ", current offset: ", ctx.file_offset, ", len: ", len);
                        Logger::getInstance().error(err_msg);
                        callbackError(err_msg);
                        ctx.status = -1;
                        ctx.release_resources();  // 释放资源
                        return len;
                    }
                }
            }

            // 写入mmap映射区域（直接内存拷贝，无中间缓存）
            memcpy(static_cast<char*>(ctx.mmap_ptr) + ctx.file_offset, data, len);
            ctx.file_offset += len;

            // 批量同步：每写入64MB触发一次msync（平衡性能和安全性）
            if (ctx.file_offset - ctx.last_sync_offset >= ctx.sync_threshold) {
                if (msync(ctx.mmap_ptr, ctx.mmap_size, MS_SYNC) == -1) {
                    Logger::getInstance().error("writeCallback: msync failed, err: ", strerror(errno));
                } else {
                    ctx.last_sync_offset = ctx.file_offset;
                    Logger::getInstance().debug("writeCallback: msync success, total written: ",
                                                ctx.file_offset);
                }
            }

            Logger::getInstance().debug("writeCallback: wrote ", len, " bytes, total: ", ctx.file_offset);
            return len;
        };

        afterPiplineFinish([this]() {
            auto& ctx = file_ctx_;
            if (ctx.mmap_ptr != MAP_FAILED && ctx.mmap_ptr != nullptr && ctx.file_fd != -1) {
                if (msync(ctx.mmap_ptr, ctx.mmap_size, MS_SYNC) == -1) {
                    Logger::getInstance().error("afterPiplineFinish: final msync failed, path: {}, err: {}",
                                                ctx.file_path, strerror(errno));
                }
            }
            // 直接调用统一释放函数（会自动判断是否已释放）
            ctx.release_resources();
        });

        return *this;
    }

    void callbackError(const std::string& error_string) {
        if (is_callback_) {
            return;
        }
        Outcome<TosError, O> outcome;
        TosError error;
        error.setIsClientError(true);
        error.setMessage(error_string);
        outcome.setE(error);
        outcome.setSuccess(false);
        on_request_done_(outcome);
        is_callback_ = true;
    }

    ProcessingPipline& enableRetry(int max_retry_count, long retry_wait_scale) {
        if (max_retry_count <= 0) {
            return *this;
        }
        auto function = [this, max_retry_count, retry_wait_scale](Outcome<TosError, O>& outcome) {
            // 1. 成功则直接返回，不重试
            if (outcome.isSuccess()) {
                return;
            }

            const int64_t retry_after = outcome.result().getRetryAfter();
            const TosError& error = outcome.error();
            const auto statusCode = error.getStatusCode();

            // 2. 校验重试前提
            if (!checkShouldRetry(user_name_, statusCode, error.getCurlErrCode(), flow_bytes_)) {
                return;
            }

            // 3. 核心重试条件：次数未超限
            const int retry_count = run_count - 1;
            if (retry_count >= max_retry_count) {
                return;
            }

            // 3.1 计算基础重试间隔：指数退避（和原逻辑一致：scale * 2^retry_count_）
            int64_t retry_wait_time = retry_wait_scale * (1LL << retry_count);  // 1LL避免溢出

            // 3.2 兼容HTTP协议：429/503时优先遵守Retry-After头
            if (statusCode == 429 || statusCode == 503) {
                if (retry_after > 0) {
                    const int64_t retry_after_ms = retry_after * 1000;  // 转为毫秒
                    // 取「Retry-After计算值」和「指数退避值」的较大者
                    retry_wait_time = std::max(retry_wait_time, retry_after_ms);
                }
            }

            // 3.3 设置「最早可发送时间」：当前时间 + 重试间隔（替代sleep）
            const int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();  // 需实现：获取当前毫秒时间戳
            not_send_util_ = now_ms + retry_wait_time;

            // 3.4 标记异步重试；当前响应回调收尾后再重新进入asyncExecute。
            retry_scheduled_ = true;
        };

        retry_out_come_decorator_ = function;

        auto retry_header_set = [this,
                                 max_retry_count](const std::shared_ptr<HttpRequest>& request) -> void {
            if (run_count <= 1) {
                return;
            }
            // 3.4 标记重试信息到请求头（和原逻辑一致，让服务端感知重试）
            const std::string retry_info =
                "attempt=" + std::to_string(run_count - 1) + "; max=" + std::to_string(max_retry_count);
            request->setHeader(http::HEADER_SDK_RETRY_COUNT, retry_info);  // 无需算签
        };

        decorateHttpRequest(retry_header_set);

        return *this;
    }

    void asyncExecute() {
        run_count++;
        const int64_t start_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                          std::chrono::system_clock::now().time_since_epoch())
                                          .count();

        if (start_time_ms_ == 0) {
            start_time_ms_ = start_time_ms;
        }

        if (!input_checks_.empty() && !ignore_input_check_) {
            for (const auto& check : input_checks_) {      // 遍历所有检查函数
                std::string error_string = check(input_);  // 执行当前检查
                if (!error_string.empty()) {
                    callbackError(error_string);
                    finish();
                    return;
                }
            }
        }

        if (!input2_http_request_) {
            callbackError("http request builder is not configured");
            finish();
            return;
        }

        const std::shared_ptr<HttpRequest> http_request = input2_http_request_(input_);
        if (http_request == nullptr) {
            callbackError("http request can not be create");
            finish();
            return;
        }

        //        if (!NetUtils::isNotIP(http_request->url().host())) {
        //            callbackError("please do not use ip:port to access");
        //            after_pipline_finish_();
        //            return;
        //        }

        if (NetUtils::isS3Endpoint(http_request->url().host())) {
            callbackError("please do not use s3 endpoint to access");
            finish();
            return;
        }

        if (!sender_) {
            callbackError("http sender is not configured");
            finish();
            return;
        }

        // 让pipline随request一起释放
        if (not_send_util_ > 0) {
            http_request->setNotSendUtilMs(not_send_util_);
        }
        http_request->setUserName(user_name_);
        http_request->setRealStartTimeMs(start_time_ms);
        if (http_request_decorator_) {
            http_request_decorator_(http_request);
        }

        std::function<void(std::shared_ptr<HttpResponse>)> handleResponse;
        handleResponse = [this, http_request](std::shared_ptr<HttpResponse> http_response) {
            if (is_callback_) {
                return;
            }

            try {
                const bool expect = std::binary_search(expect_status_.begin(), expect_status_.end(),
                                                       http_response->statusCode());
                Outcome<TosError, O> outcome;
                TosError error;
                O result;
                if (expect && http_response->status() == 0) {
                    outcome.setSuccess(true);
                    expect_response2_out_come_(http_request, http_response, error, result);
                } else {
                    outcome.setSuccess(false);
                    unexpect_response2_out_come_(http_request, http_response, error, result);
                }

                outcome.setE(error);
                outcome.setR(result);

                if (out_come_decorator_) {
                    out_come_decorator_(outcome);
                }

                retry_scheduled_ = false;
                if (retry_out_come_decorator_) {
                    retry_out_come_decorator_(outcome);
                }

                if (retry_scheduled_) {
                    asyncExecute();
                    return;
                }

                on_request_done_(outcome);
                is_callback_ = true;
            } catch (const std::exception& e) {
                Outcome<TosError, O> outcome;
                TosError error;
                error.setIsClientError(true);
                error.setCode("UnhandledException");
                error.setMessage(e.what());
                if (http_response) {
                    error.setStatusCode(http_response->statusCode());
                }
                error.setRequestUrl(http_request->url().toString());
                outcome.setE(error);
                outcome.setSuccess(false);
                on_request_done_(outcome);
                is_callback_ = true;
            } catch (...) {
                Outcome<TosError, O> outcome;
                TosError error;
                error.setIsClientError(true);
                error.setCode("UnhandledException");
                error.setMessage("unknown exception in async pipeline");
                if (http_response) {
                    error.setStatusCode(http_response->statusCode());
                }
                error.setRequestUrl(http_request->url().toString());
                outcome.setE(error);
                outcome.setSuccess(false);
                on_request_done_(outcome);
                is_callback_ = true;
            }
        };

        sender_(http_request, on_data_receive_, on_data_send_,
                [handleResponse, this](const std::shared_ptr<HttpResponse>& response) {
                    handleResponse(response);  // 第一次尝试
                    if (is_callback_) {
                        finish();
                    }
                },
                on_request_start_, nullptr, on_content_length_set_);
    }

 private:
    static constexpr size_t kReusableResponseBufferCapacity = 64 * 1024;

    void resetRuntimeState() {
        run_count = 0;
        need_callback_ = true;
        is_callback_ = false;
        retry_scheduled_ = false;
        finish_called_ = false;
        ignore_input_check_ = false;
        start_time_ms_ = 0;
        not_send_util_ = 0;
        flow_bytes_ = 0;
        response_buffer_limit_ = 0;
        response_buffer_truncated_ = false;

        input_checks_.clear();
        input2_http_request_ = nullptr;
        tos_request2_http_request_ = nullptr;
        sender_ = nullptr;
        unexpect_response2_out_come_ = nullptr;
        expect_response2_out_come_ = nullptr;
        on_data_receive_ = nullptr;
        on_data_send_ = nullptr;
        on_request_done_ = nullptr;
        response2_retry_ = nullptr;
        after_pipline_finishes_.clear();
        expect_status_ = {200};
        input_decorator_ = nullptr;
        http_request_decorator_ = nullptr;
        out_come_decorator_ = nullptr;
        retry_out_come_decorator_ = nullptr;
        user_name_.clear();
        json2output_ = nullptr;
        input2_json_ = nullptr;
        on_request_start_ = nullptr;
        on_content_length_set_ = nullptr;
        file_ctx_.clear();

        if (response_buffer_.capacity() > kReusableResponseBufferCapacity) {
            std::vector<char>().swap(response_buffer_);
        } else {
            response_buffer_.clear();
        }
    }

    static void runFinalizers(std::vector<AfterPiplineFinish> finalizers) {
        for (auto it = finalizers.rbegin(); it != finalizers.rend(); ++it) {
            if (*it) {
                (*it)();
            }
        }
    }

    void finish() {
        if (finish_called_) {
            return;
        }

        finish_called_ = true;
        auto finalizers = std::move(after_pipline_finishes_);
        after_pipline_finishes_.clear();
        runFinalizers(std::move(finalizers));
    }

    T input_;
    int run_count = 0;  // asyncExecute运行次数
    bool need_callback_ = true;
    bool is_callback_ = false;
    bool retry_scheduled_ = false;
    bool finish_called_ = false;

    bool ignore_input_check_ = false;
    std::vector<InputCheck> input_checks_;
    Input2HttpRequest input2_http_request_;
    TosRequest2HttpRequest tos_request2_http_request_;
    HttpRequestSender sender_;
    HttpResponse2OutCome unexpect_response2_out_come_;
    HttpResponse2OutCome expect_response2_out_come_;

    ::VolcengineTos::OnDataReceiveWithEvent on_data_receive_;
    ::VolcengineTos::OnDataSendWithEvent on_data_send_;
    OutcomeCallback on_request_done_;
    //    AnyOutcomeCallback on_any_request_done_;

    HttpResponse2Retry response2_retry_;

    std::vector<AfterPiplineFinish> after_pipline_finishes_;

    std::vector<int> expect_status_;

    std::vector<char> response_buffer_;
    size_t response_buffer_limit_ = 0;
    bool response_buffer_truncated_ = false;
    InputDecorator input_decorator_;
    HttpRequestDecorator http_request_decorator_;
    OutComeDecorator out_come_decorator_;
    OutComeDecorator retry_out_come_decorator_;
    std::string user_name_;
    int64_t start_time_ms_;
    DestroyCallback on_destroy_;

    Json2Output json2output_;
    Input2Json input2_json_;

    OnRequestStart
        on_request_start_;  // 这个阶段callback error的，不代表后续不读取数据，数据发送/读取完后，正常释放
    OnContentLengthSet on_content_length_set_;

    FileContext file_ctx_;

    int64_t not_send_util_ = 0;
    int64_t flow_bytes_ = 0;
};

}  // namespace VolcengineTos
