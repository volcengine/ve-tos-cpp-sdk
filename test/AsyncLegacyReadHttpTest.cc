// Real native asynchronous SDK and deterministic server scripts; no account credentials.
// This is a wire/lifetime contract test, not a TosFuse driver or real TOS proof.
#include "LoopbackHttpFixture.h"
#include "TosAsyncClient.h"
#include "logger/logger.h"
#include <curl/curl.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

using namespace VolcengineTos;
using Server = tos_test::LoopbackHttpFixture;

// Internal diagnostics used only by this standalone single-client test. A
// balanced context counter is not a production per-request drain receipt.
namespace VolcengineTos {
extern std::atomic<uint64_t> g_req_ctx_create_cnt;
extern std::atomic<uint64_t> g_req_ctx_destroy_cnt;
}

namespace {
#define CHECK(expression)                                                                     \
    do {                                                                                      \
        if (!(expression))                                                                    \
            throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " + \
                                     #expression);                                            \
    } while (false)

template <class Output>
struct Completion {
    mutable std::mutex mutex;
    std::condition_variable cv;
    unsigned calls{0};
    Outcome<TosError, Output> outcome;

    void Record(Outcome<TosError, Output>& result) {
        std::lock_guard<std::mutex> lock(mutex);
        ++calls;
        if (calls == 1) outcome = result;
        cv.notify_all();
    }
    Outcome<TosError, Output> Wait() {
        std::unique_lock<std::mutex> lock(mutex);
        CHECK(cv.wait_for(lock, std::chrono::seconds(5), [&] { return calls != 0; }));
        return outcome;
    }
    unsigned Calls() const {
        std::lock_guard<std::mutex> lock(mutex);
        return calls;
    }
};

// Never hold SDK transport bytes or AsyncEvent beyond the receive call.
// The test sink deliberately rejects a body exceeding its charged capacity.
struct Sink {
    explicit Sink(size_t capacity) : capacity(capacity) { bytes.reserve(capacity); }
    size_t Receive(char* data, size_t size, AsyncEvent*) {
        std::lock_guard<std::mutex> lock(mutex);
        if (size > capacity - bytes.size()) {
            overflow = true;
            return 0;
        }
        bytes.append(data, size);
        return size;
    }
    std::string Bytes() const {
        std::lock_guard<std::mutex> lock(mutex);
        return bytes;
    }
    mutable std::mutex mutex;
    const size_t capacity;
    std::string bytes;
    bool overflow{false};
};

ClientConfig Config(const Server& server, int timeout_ms = 2000) {
    ClientConfig config;
    config.endPoint = server.Endpoint();
    // This routes /key on loopback; it is not bucket-in-path addressing.
    config.isCustomDomain = true;
    config.event_thread_count_ = 1;
    config.async_transport_mode_ = AsyncTransportMode::Isolated;
    config.maxConnections = 2;
    config.max_request_queue_ = 8;
    config.maxRetryCount = 0;
    config.requestTimeout = timeout_ms;
    config.connectionTimeout = 1000;
    config.enableCRC = false;
    config.enableDebug = false;
    config.detail_log_ = false;
    config.connection_reuse_ = false;
    return config;
}

Server::Response Reply(int status, std::string body = {}) {
    Server::Response response;
    response.status = status;
    response.reason = status == 206 ? "Partial Content" : (status == 200 ? "OK" : "Error");
    response.body = std::move(body);
    response.headers = {{"ETag", "\"opaque-etag\""},
                        {"x-tos-version-id", "version-1"},
                        {"x-tos-request-id", "local-contract-only"},
                        {"Last-Modified", "Thu, 01 Jan 1970 00:00:01 GMT"}};
    return response;
}

void Read(TosAsyncClient& client, GetObjectAsyncInput input, Sink& sink,
          Completion<GetObjectAsyncOutput>& completion, bool throw_on_done = false) {
    client.getObjectAsync(
            input, [&sink](char* data, size_t size, AsyncEvent* event) {
                return sink.Receive(data, size, event);
            },
            [&completion, throw_on_done](Outcome<TosError, GetObjectAsyncOutput>& result) {
                completion.Record(result);
                if (throw_on_done) throw std::runtime_error("intentional consumer failure");
            });
}

void ListType2RequiresBooleanTruncationFlag() {
    ListObjectsType2Output final_page;
    final_page.fromJsonString(R"({"IsTruncated":false,"KeyCount":0,"Contents":[]})");
    CHECK(!final_page.isTruncated() && final_page.getContents().empty());
    ListObjectsType2Output more_pages;
    more_pages.fromJsonString(
            R"({"IsTruncated":true,"NextContinuationToken":"next+%2F","Contents":[{"Key":"raw%2Fkey"}]})");  // @opensource-lint-ignore: synthetic pagination cursor, not an authentication credential.
    CHECK(more_pages.isTruncated());
    CHECK(more_pages.getNextContinuationToken() == "next+%2F");
    CHECK(more_pages.getContents().size() == 1 && more_pages.getContents()[0].getKey() == "raw%2Fkey");
    for (const std::string malformed : {
                 R"({"Name":"fixture-bucket","KeyCount":0,"Contents":[]})",
                 R"({"IsTruncated":null})", R"({"IsTruncated":0})", R"({"IsTruncated":1})",
                 R"({"IsTruncated":"false"})", R"({"IsTruncated":[]})", R"({"IsTruncated":{}})",
                 "[]", "null"}) {
        bool rejected = false;
        try {
            ListObjectsType2Output output;
            output.fromJsonString(malformed);
        } catch (const std::exception&) {
            rejected = true;
        }
        CHECK(rejected);
    }
}

void SymlinkBodyFramingAndServiceErrors() {
    struct Expected {
        bool success;
        int status;
        std::string code;
    };
    std::vector<Server::Step> steps;
    std::vector<Expected> expected;
    auto add = [&](Server::BodyFraming framing, int status, std::string body, bool success,
                   std::string code) {
        auto response = Reply(status, std::move(body));
        response.framing = framing;
        response.headers["x-tos-symlink-target"] = "../raw%2F+target";
        response.headers["x-tos-symlink-bucket"] = "fixture-bucket";
        steps.push_back({"GET", "/native-link?symlink=&versionId=version-1", std::move(response)});
        expected.push_back({success, status, std::move(code)});
    };
    for (const auto framing : {Server::BodyFraming::ContentLength, Server::BodyFraming::CloseDelimited,
                               Server::BodyFraming::Chunked}) {
        add(framing, 200, "", true, "");
        // Valid JSON used to be silently discarded, producing a false success.
        add(framing, 200, "{}", false, "UnexpectedResponseBody");
    }
    add(Server::BodyFraming::CloseDelimited, 200, "not-json", false, "UnexpectedResponseBody");
    add(Server::BodyFraming::ContentLength, 200, std::string(65537, 'x'), false, "UnexpectedResponseBody");
    add(Server::BodyFraming::CloseDelimited, 403, R"({"Code":"AccessDenied","Message":"denied"})",
        false, "AccessDenied");
    add(Server::BodyFraming::Chunked, 404, R"({"Code":"NoSuchKey","Message":"missing"})", false,
        "NoSuchKey");
    add(Server::BodyFraming::ContentLength, 412, R"({"Code":"PreconditionFailed","Message":"changed"})",
        false, "PreconditionFailed");
    add(Server::BodyFraming::ContentLength, 403,
        std::string("{\"Code\":\"AccessDenied\",\"Message\":\"") + std::string(65537, 'x') + "\"}",
        false, "ResponseBodyTooLarge");
    Server server(std::move(steps));
    std::vector<std::shared_ptr<Completion<GetSymlinkAsyncOutput>>> completions;
    completions.reserve(expected.size());
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    for (const auto& expect : expected) {
        auto completion = std::make_shared<Completion<GetSymlinkAsyncOutput>>();
        completions.push_back(completion);
        GetSymlinkAsyncInput input("fixture-bucket", "native-link");
        input.setIfMatch("\"opaque-etag\"");
        input.setVersionId("version-1");
        client.getSymlinkAsync(input, [completion](auto& result) { completion->Record(result); });
        const auto outcome = completion->Wait();
        CHECK(completion->Calls() == 1 && outcome.isSuccess() == expect.success);
        if (expect.status == 200) {
            CHECK(outcome.result().getStatusCode() == 200);
            CHECK(outcome.result().getSymlinkTargetKey() == "../raw%2F+target");
            CHECK(outcome.result().getSymlinkTargetBucket() == "fixture-bucket");
            if (!expect.success) CHECK(outcome.error().isClientError());
        } else {
            CHECK(outcome.error().getStatusCode() == expect.status);
            CHECK(outcome.error().isClientError() == (expect.code == "ResponseBodyTooLarge"));
        }
        if (!expect.success) CHECK(outcome.error().getCode() == expect.code);
    }
    client.close();
    server.AssertDone();
    for (const auto& completion : completions) CHECK(completion->Calls() == 1);
    const auto requests = server.Requests();
    CHECK(requests.size() == expected.size());
    for (const auto& request : requests) {
        CHECK(request.headers.at("if-match") == "\"opaque-etag\"");
        CHECK(request.headers.count("range") == 0);
        CHECK(request.headers.at("authorization") == "<redacted>");
    }
}

void HeadAndPinnedOneByte() {
    auto head = Reply(200);
    head.declared_content_length = 10;
    head.headers.insert({{"Content-Type", "application/octet-stream"},
                         {"x-tos-meta-mode", "33188"},
                         {"x-tos-meta-uid", "123"},
                         {"x-tos-meta-comment", "one%252Ftwo"}});
    auto range = Reply(206, "a");
    range.headers["Content-Range"] = "bytes 0-0/10";
    Server server({{"HEAD", "/legacy/file%20name", head},
                   {"GET", "/legacy/file%20name?versionId=version-1", range}});
    Completion<HeadObjectAsyncOutput> headed;
    Completion<GetObjectAsyncOutput> read;
    Sink sink(1);
    // The fixture needs no identity. Empty credentials still exercise signing.
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    client.headObjectAsync(HeadObjectAsyncInput("fixture-bucket", "legacy/file name"),
                           [&](auto& result) { headed.Record(result); });
    const auto metadata = headed.Wait();
    CHECK(metadata.isSuccess());
    CHECK(metadata.result().getContentLength() == 10);
    CHECK(metadata.result().getEtag() == "\"opaque-etag\"");
    CHECK(metadata.result().getVersionId() == "version-1");
    CHECK(metadata.result().getLastModified() == 1);
    CHECK(metadata.result().getMeta().at("mode") == "33188");
    CHECK(metadata.result().getMeta().at("uid") == "123");
    CHECK(metadata.result().getMeta().at("comment") == "one%2Ftwo");

    GetObjectAsyncInput input("fixture-bucket", "legacy/file name");
    input.setRange("bytes=0-0");
    input.setIfMatch(metadata.result().getEtag());
    input.setVersionId(metadata.result().getVersionId());
    Read(client, input, sink, read);
    const auto result = read.Wait();
    client.close();  // Main thread, isolated transport, after admitted requests.
    server.AssertDone();
    CHECK(headed.Calls() == 1 && read.Calls() == 1);
    CHECK(result.isSuccess() && result.result().getStatusCode() == 206);
    CHECK(result.result().getContentRange() == "bytes 0-0/10");
    CHECK(result.result().getContentLength() == 1 && sink.Bytes() == "a");
    const auto requests = server.Requests();
    CHECK(requests.size() == 2);
    CHECK(requests[1].headers.count("range") == 1);
    CHECK(requests[1].headers.at("range") == "bytes=0-0");
    CHECK(requests[1].headers.at("if-match") == "\"opaque-etag\"");
}

void NumericAndReservedHeaders() {
    auto partial = Reply(206, "cdef");
    partial.headers["Content-Range"] = "bytes 2-5/10";
    Server server({{"GET", "/numeric", partial}, {"GET", "/whole", Reply(200, "abcdefghij")}});
    Completion<GetObjectAsyncOutput> ranged, whole;
    Sink first(4), second(10);
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    GetObjectAsyncInput input("fixture-bucket", "numeric");
    input.setRangeStart(2);
    input.setRangeEnd(5);
    input.setHeaders({{"Range", "bytes=9-9"}, {"Host", "not-a-real-host.invalid"},
                      {"Content-Length", "1234"}, {"Authorization", "not-a-real-signature"}});
    Read(client, input, first, ranged);
    CHECK(ranged.Wait().isSuccess());
    GetObjectAsyncInput plain("fixture-bucket", "whole");
    plain.setHeaders({{"Range", "bytes=9-9"}});
    Read(client, plain, second, whole);
    CHECK(whole.Wait().isSuccess());
    client.close();
    server.AssertDone();
    const auto requests = server.Requests();
    CHECK(requests[0].headers.count("range") == 1);
    CHECK(requests[0].headers.at("range") == "bytes=2-5");
    CHECK(requests[0].headers.at("host").find("127.0.0.1:") == 0);
    CHECK(requests[0].headers.count("content-length") == 0);
    CHECK(requests[1].headers.count("range") == 0);  // Generic reserved header stays blocked.
    CHECK(first.Bytes() == "cdef" && second.Bytes() == "abcdefghij");
    CHECK(ranged.Calls() == 1 && whole.Calls() == 1);
}

void PreconditionAndShortBody() {
    auto failed = Reply(412, "{\"Code\":\"PreconditionFailed\",\"Message\":\"changed\"}");
    failed.headers["Content-Type"] = "application/json";
    auto short_body = Reply(206, "ab");
    short_body.headers["Content-Range"] = "bytes 0-3/10";
    short_body.declared_content_length = 4;
    Server server({{"GET", "/changed", failed}, {"GET", "/short", short_body}});
    Completion<GetObjectAsyncOutput> changed, short_read;
    Sink first(128), second(4);
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    GetObjectAsyncInput input("fixture-bucket", "changed");
    input.setIfMatch("\"old\"");
    input.setRange("bytes=0-3");
    Read(client, input, first, changed);
    const auto error = changed.Wait();
    CHECK(!error.isSuccess() && error.error().getStatusCode() == 412);
    CHECK(error.error().getCurlErrCode() == CURLE_OK);
    GetObjectAsyncInput short_input("fixture-bucket", "short");
    short_input.setRange("bytes=0-3");
    Read(client, short_input, second, short_read);
    const auto short_result = short_read.Wait();
    CHECK(!short_result.isSuccess());
    CHECK(short_result.error().getCurlErrCode() == CURLE_PARTIAL_FILE);
    client.close();
    server.AssertDone();
    CHECK(changed.Calls() == 1 && short_read.Calls() == 1);
    // The generic SDK receive callback also receives error bodies. An empty
    // small sink would hide this behind CURLE_WRITE_ERROR. A filesystem driver
    // must own these bytes privately until status/headers are validated.
    CHECK(first.Bytes() == failed.body);
    CHECK(second.Bytes() == "ab");  // Driver must discard these partial bytes on failure.
}

void FullBodyAndBoundedSink() {
    // SDK's generic GET intentionally accepts 200. A filesystem Range driver
    // must reject it (including a small 200 body) and validate Content-Range.
    Server server({{"GET", "/ignores-range", Reply(200, "ab")},
                   {"GET", "/oversized", Reply(200, std::string(8192, 'x'))}});
    Completion<GetObjectAsyncOutput> full, rejected;
    Sink first(4), second(4);
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    GetObjectAsyncInput input("fixture-bucket", "ignores-range");
    input.setRange("bytes=0-1");
    Read(client, input, first, full);
    const auto generic = full.Wait();
    CHECK(generic.isSuccess() && generic.result().getStatusCode() == 200);
    CHECK(generic.result().getContentRange().empty());
    GetObjectAsyncInput oversized("fixture-bucket", "oversized");
    oversized.setRange("bytes=0-3");
    Read(client, oversized, second, rejected);
    const auto rejected_result = rejected.Wait();
    CHECK(!rejected_result.isSuccess());
    CHECK(rejected_result.error().getCurlErrCode() == CURLE_WRITE_ERROR);
    client.close();
    server.AssertDone();
    CHECK(full.Calls() == 1 && rejected.Calls() == 1);
    CHECK(first.Bytes() == "ab" && second.Bytes().size() <= 4 && second.overflow);
}

void DeadlineBeforeServerRelease() {
    auto gate = std::make_shared<Server::Gate>();
    auto held = Reply(206, "a");
    held.headers["Content-Range"] = "bytes 0-0/10";
    held.gate = gate;
    held.allow_peer_disconnect = true;
    Server server({{"GET", "/held", held}});
    Completion<GetObjectAsyncOutput> completion;
    Sink sink(1);
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server, 150));
    GetObjectAsyncInput input("fixture-bucket", "held");
    input.setRange("bytes=0-0");
    Read(client, input, sink, completion);
    CHECK(gate->WaitUntilReached());
    const auto result = completion.Wait();  // Gate stays closed until SDK times out.
    gate->Release();
    client.close();
    CHECK(!result.isSuccess());
    CHECK(result.error().getCurlErrCode() == CURLE_OPERATION_TIMEDOUT);
    CHECK(completion.Calls() == 1 && sink.Bytes().empty());
    server.AssertDone();
    CHECK(server.Requests().size() == 1);
}

void ThrowingCompletionAndNextRequest() {
    Server server({{"GET", "/throws", Reply(200, "a")}, {"GET", "/next", Reply(200, "b")}});
    Completion<GetObjectAsyncOutput> first, next, invalid;
    Sink a(1), b(1), unused(1);
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    Read(client, GetObjectAsyncInput("fixture-bucket", "throws"), a, first, true);
    CHECK(first.Wait().isSuccess());
    Read(client, GetObjectAsyncInput("fixture-bucket", "next"), b, next);
    CHECK(next.Wait().isSuccess());
    // Validation is inline; an exception in its completion must not escape or
    // produce a second callback, and must not strand the pipeline finalizer.
    Read(client, GetObjectAsyncInput("fixture-bucket", ""), unused, invalid, true);
    CHECK(invalid.Calls() == 1 && !invalid.Wait().isSuccess());
    client.close();
    server.AssertDone();
    CHECK(first.Calls() == 1 && next.Calls() == 1 && invalid.Calls() == 1);
    CHECK(a.Bytes() == "a" && b.Bytes() == "b" && unused.Bytes().empty());
}

void ListSchemaFailureAndServiceErrorsCompleteOnce() {
    const std::string target = "/?fetch-owner=true&list-type=2&max-keys=3&prefix=old%2F";
    const std::string valid =
            R"({"Name":"fixture-bucket","Prefix":"old/","MaxKeys":3,"KeyCount":1,"IsTruncated":false,"Contents":[{"Key":"old/%2Fkey"}]})";
    const std::vector<Server::Response> responses{
            Reply(200, R"({"Name":"fixture-bucket","KeyCount":0,"Contents":[]})"),
            Reply(200, R"({"IsTruncated":"false"})"),
            Reply(403, R"({"Code":"AccessDenied","Message":"contract"})"),
            Reply(404, R"({"Code":"NoSuchBucket","Message":"contract"})"),
            Reply(412, R"({"Code":"PreconditionFailed","Message":"contract"})"),
            Reply(200, valid)};
    std::vector<Server::Step> steps;
    for (const auto& response : responses) steps.push_back({"GET", target, response});
    Server server(std::move(steps));
    std::vector<std::shared_ptr<Completion<ListObjectsType2Output>>> completions;
    TosAsyncClient client("cn-beijing", std::make_shared<Credentials>(), Config(server));
    ListObjectsType2Input input("fixture-bucket");
    input.setPrefix("old/");
    input.setMaxKeys(3);
    input.setDelimiter("");
    for (size_t index = 0; index < responses.size(); ++index) {
        auto completion = std::make_shared<Completion<ListObjectsType2Output>>();
        completions.push_back(completion);
        client.listObjectsType2Async(input, [completion](auto& result) { completion->Record(result); });
        const auto result = completion->Wait();
        if (index < 2) {
            CHECK(!result.isSuccess() && result.error().isClientError());
            // The typed output is not initialized after a parser failure.
            // Do not read its RequestInfo/status in this branch.
        } else if (index < 5) {
            CHECK(!result.isSuccess() && !result.error().isClientError());
            CHECK(result.error().getStatusCode() == responses[index].status);
            CHECK(result.error().getCurlErrCode() == 0);
        } else {
            CHECK(result.isSuccess() && result.result().getRequestInfo().getStatusCode() == 200);
            CHECK(!result.result().isTruncated() && result.result().getContents().size() == 1);
            CHECK(result.result().getContents()[0].getKey() == "old/%2Fkey");
        }
    }
    client.close();
    server.AssertDone();
    CHECK(server.Requests().size() == responses.size());
    for (const auto& completion : completions) CHECK(completion->Calls() == 1);
}
}  // namespace

int main() {
    try {
        Logger::getInstance().setAsyncLogLevel(ERROR);
        const auto outstanding = g_req_ctx_create_cnt.load() - g_req_ctx_destroy_cnt.load();
        ListType2RequiresBooleanTruncationFlag();
        SymlinkBodyFramingAndServiceErrors();
        HeadAndPinnedOneByte();
        NumericAndReservedHeaders();
        PreconditionAndShortBody();
        FullBodyAndBoundedSink();
        DeadlineBeforeServerRelease();
        ThrowingCompletionAndNextRequest();
        ListSchemaFailureAndServiceErrorsCompleteOnce();
        CHECK(g_req_ctx_create_cnt.load() - g_req_ctx_destroy_cnt.load() == outstanding);
        std::cout << "async SDK HTTP: 8 loopback + 1 LIST parser contract groups passed (one worker, HTTP only)\n";
        return 0;
    } catch (const std::exception& error) {
        // Never include request headers, dummy Authorization or environment.
        std::cerr << "async SDK HTTP contract failed: " << error.what() << '\n';
        return 1;
    }
}
