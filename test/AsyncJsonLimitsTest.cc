#include "executor/ProcessingPipline.h"
#include "model/object/ListObjectsType2Output.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>

namespace {
thread_local bool fail_next_allocation = false;
}
void* operator new(std::size_t size) {
    if (fail_next_allocation) {
        fail_next_allocation = false;
        throw std::bad_alloc();
    }
    if (auto* memory = std::malloc(size ? size : 1)) return memory;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* value) noexcept { std::free(value); }
void operator delete[](void* value) noexcept { std::free(value); }
void operator delete(void* value, std::size_t) noexcept { std::free(value); }
void operator delete[](void* value, std::size_t) noexcept { std::free(value); }

using namespace VolcengineTos;
#define CHECK(value)                                                                                 \
    do {                                                                                             \
        if (!(value))                                                                                \
            throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) + ": " #value); \
    } while (false)

namespace {
struct Output {
    int value{0};
    int64_t getRetryAfter() const { return 0; }
};
using Pipeline = ProcessingPipline<int, Output>;
using OutcomeType = Outcome<TosError, Output>;

// Only the physical sender is replaced. Buffering, JSON parser, completion,
// finalization and typed LIST conversion are the real SDK implementations.
struct Fixture {
    Pipeline pipeline{1};
    OnDataReceiveWithEvent receive;
    OnContentLengthSet length;
    std::function<void(std::shared_ptr<HttpResponse>)> complete;
    unsigned sends{0}, parses{0}, replies{0}, finishes{0};
    std::optional<OutcomeType> result;
    explicit Fixture(JsonResponseLimits limits) {
        pipeline
            .input2HttpRequest([](int&) {
                auto request = std::make_shared<HttpRequest>();
                request->setUrl(Url("http://127.0.0.1:1/json-contract"));
                return request;
            })
            .httpRequestSender([this](auto, const auto& recv, const auto&, const auto& done, const auto&,
                                      const auto&, const auto& size) {
                ++sends;
                receive = recv;
                length = size;
                complete = done;
            })
            .expectResponse2Outcome([](auto, auto, TosError&, Output&) {})
            .defaultUnexpectResponse2Outcome()
            .json2Output([this](Output& output, json& document) {
                ++parses;
                if (document.contains("value")) document.at("value").get_to(output.value);
            })
            .responseJsonBounded(limits)
            .onRequestDone([this](auto& outcome) {
                ++replies;
                result = outcome;
            })
            .afterPiplineFinish([this] { ++finishes; });
        pipeline.asyncExecute();
        CHECK(sends == 1 && replies == 0 && finishes == 0);
    }
    size_t Feed(const std::string& bytes) {
        return receive(const_cast<char*>(bytes.data()), bytes.size(), nullptr);
    }
    void Finish(int status = 200) {
        auto response = std::make_shared<HttpResponse>();
        response->setStatus(0);
        response->setStatusCode(status);
        complete(response);
        complete(response);
        CHECK(replies == 1 && finishes == 1 && result);
    }
    void Failure(const std::string& code) {
        CHECK(result && !result->isSuccess() && result->error().isClientError());
        CHECK(result->error().getCode() == code && parses == 0 && result->result().value == 0);
    }
};

void ExactBodyAndShapeLimits() {
    const std::string body = R"({"value":7})";
    for (int64_t declared : std::array<int64_t, 3>{-1, 0, static_cast<int64_t>(body.size())}) {
        Fixture fixture({body.size(), 1, 3});
        fixture.length(declared);
        CHECK(fixture.Feed(body.substr(0, 4)) == 4);
        CHECK(fixture.Feed(body.substr(4)) == body.size() - 4);
        fixture.Finish();
        CHECK(fixture.result->isSuccess() && fixture.parses == 1 && fixture.result->result().value == 7);
    }
    Fixture no_length({body.size(), 1, 3});
    CHECK(no_length.Feed(body) == body.size());
    no_length.Finish();
    CHECK(no_length.result->isSuccess() && no_length.result->result().value == 7);
}

void OversizeHeadersAndChunksNeverPublishPrefix() {
    const std::string body = R"({"value":7})";
    for (bool header : {false, true}) {
        Fixture fixture({body.size(), 4, 100});
        if (header) {
            // No allocation is allowed solely because a server claims a huge body.
            fail_next_allocation = true;
            fixture.length(std::numeric_limits<int64_t>::max());
            CHECK(fail_next_allocation);
            fail_next_allocation = false;
        } else {
            CHECK(fixture.Feed(body) == body.size());
        }
        CHECK(fixture.Feed(" ") == 0);
        CHECK(fixture.Feed(body) == 0);  // Cannot recover after overflow with a valid suffix.
        fixture.Finish();
        fixture.Failure("ResponseBodyTooLarge");
    }
    Fixture huge_chunk({16, 4, 100});
    char byte = 'x';
    CHECK(huge_chunk.receive(&byte, std::numeric_limits<size_t>::max(), nullptr) == 0);
    huge_chunk.Finish();
    huge_chunk.Failure("ResponseBodyTooLarge");
    Fixture header_only({16, 4, 100});
    header_only.length(17);
    header_only.Finish();
    header_only.Failure("ResponseBodyTooLarge");
}

void AllocationFailureDoesNotEscapeReceive() {
    Fixture fixture({128, 4, 100});
    const std::string body = R"({"value":7})";
    fixture.length(128);
    AsyncEvent event;
    fail_next_allocation = true;
    CHECK(fixture.receive(const_cast<char*>(body.data()), body.size(), &event) == 0);
    CHECK(!fail_next_allocation);
    fixture.Finish();
    fixture.Failure("ResponseBodyAllocationFailed");
}

void DepthAndEventsStopBeforeTypedOutput() {
    for (const auto& item : {std::pair<std::string, JsonResponseLimits>{R"({"value":7})", {128, 4, 2}},
                             {R"({"nested":{"value":7}})", {128, 1, 100}},
                             {R"([0,0,0,0,0])", {128, 4, 3}},
                             {R"({"a":1,"a":2,"a":3})", {128, 4, 5}}}) {
        Fixture fixture(item.second);
        CHECK(fixture.Feed(item.first) == item.first.size());
        fixture.Finish();
        fixture.Failure("JsonResponseLimitExceeded");
    }
    Fixture exact({128, 2, 5});
    const std::string nested = R"({"nested":{"value":7}})";
    CHECK(exact.Feed(nested) == nested.size());
    exact.Finish();
    CHECK(exact.result->isSuccess() && exact.parses == 1);
}

void MalformedAndServiceErrorsRemainErrors() {
    for (const std::string& bytes : {std::string{}, std::string("{")}) {
        Fixture fixture({128, 4, 100});
        if (!bytes.empty()) CHECK(fixture.Feed(bytes) == bytes.size());
        fixture.Finish();
        fixture.Failure("JsonParseError");
    }
    const std::string bytes = R"({"Code":"NoSuchKey","Message":"missing"})";
    Fixture service({128, 4, 100});
    CHECK(service.Feed(bytes) == bytes.size());
    service.Finish(404);
    CHECK(!service.result->isSuccess() && service.parses == 0);
    CHECK(service.result->error().getCode() == "NoSuchKey" && service.result->error().getStatusCode() == 404);
    Fixture empty_service({128, 4, 100});
    empty_service.Finish(403);
    CHECK(!empty_service.result->isSuccess() && empty_service.result->error().getStatusCode() == 403);
    CHECK(empty_service.parses == 0);
}

void InvalidLocalLimitsFailBeforeSubmission() {
    for (auto limits : {JsonResponseLimits{0, 1, 1}, {16, 0, 1}, {16, 1, 0}, {16, 257, 1}}) {
        Pipeline pipeline(1);
        bool threw = false;
        try {
            pipeline.responseJsonBounded(limits);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw);
    }
}

void ListDirectDecodeAndAtomicPublication() {
    ListObjectsType2Output output;
    CHECK(output.getRequestInfo().getStatusCode() == 0);
    RequestInfo info;
    info.setStatusCode(200);
    output.setRequestInfo(info);
    const auto first =
        json::parse(R"({"IsTruncated":false,"Name":"bucket","Contents":[{"Key":"old%2F+key"}]})");
    output.fromJson(first, 1);
    CHECK(output.getContents().size() == 1 && output.getContents()[0].getKey() == "old%2F+key");
    CHECK(output.getRequestInfo().getStatusCode() == 200);
    for (const auto& bad :
         {R"({"IsTruncated":false,"Name":"changed","Contents":[{"Key":"new"},{"Key":1}]})",
          R"({"IsTruncated":false,"Contents":[{"Key":"a"},{"Key":"b"}]})",
          R"({"IsTruncated":false,"Contents":[{"Key":"a"}],"CommonPrefixes":[{"Prefix":"d/"}]})",
          R"({"IsTruncated":false,"Contents":{}})", R"({"IsTruncated":false,"Contents":[1]})",
          R"({"Contents":[]})"}) {
        bool threw = false;
        try {
            output.fromJson(json::parse(bad), 1);
        } catch (const std::exception&) {
            threw = true;
        }
        CHECK(threw && output.getName() == "bucket" && output.getContents().size() == 1);
        CHECK(output.getContents()[0].getKey() == "old%2F+key" &&
              output.getRequestInfo().getStatusCode() == 200);
    }
    const auto map_size = StringtoStorageClassType.size();
    output.fromJsonString(
        R"({"IsTruncated":false,"Contents":[{"Key":"new","StorageClass":"unknown-contract-value"}]})");
    CHECK(output.getContents().size() == 1 && output.getContents()[0].getKey() == "new");
    CHECK(output.getName().empty() && StringtoStorageClassType.size() == map_size);
    output.fromJsonString(R"({"IsTruncated":false,"Contents":[]})");
    CHECK(output.getContents().empty());
}

void ListIntegerSchemaAndAtomicPublication() {
    ListObjectsType2Output output;
    RequestInfo info;
    info.setStatusCode(200);
    info.setHeaders({{"x-fixture", "retained"}});
    output.setRequestInfo(info);
    const auto original = json::parse(
        R"({"IsTruncated":false,"Name":"original","MaxKeys":3,"KeyCount":1,"Contents":[{"Key":"old/pinned","Size":7}]})");
    output.fromJson(original, 2);
    auto rejected = [&](json input) {
        input["Name"] = "must-not-publish";
        bool threw = false;
        try {
            output.fromJson(input, 2);
        } catch (const std::exception&) {
            threw = true;
        }
        CHECK(threw && output.getName() == "original");
        CHECK(output.getMaxKeys() == 3 && output.getKeyCount() == 1 && !output.isTruncated());
        CHECK(output.getContents().size() == 1 && output.getContents()[0].getKey() == "old/pinned");
        CHECK(output.getContents()[0].getSize() == 7 && output.getRequestInfo().getStatusCode() == 200);
        CHECK(output.getRequestInfo().getHeaders().at("x-fixture") == "retained");
    };
    for (const auto* field : {"MaxKeys", "KeyCount"}) {
        for (const auto* number : {"-1", "-4294967293", "1.5", "1.0", "2147483648", "4294967297",
                                   "4294967299", "18446744073709551615", "18446744073709551616",
                                   "1e100", "true", "null", "\"1\"", "[]", "{}"}) {
            auto input = original;
            input[field] = json::parse(number);
            rejected(std::move(input));
        }
    }
    for (const auto* number : {"-1", "0.5", "0.0", "9223372036854775808", "18446744073709551615",
                               "18446744073709551616", "1e100", "true", "null", "\"0\"", "[]", "{}"}) {
        auto input = original;
        input["KeyCount"] = 2;
        // The first item is valid and already converted before the second
        // item's invalid Size throws; neither may replace the previous page.
        input["Contents"].push_back({{"Key", "old/new"}, {"Size", json::parse(number)}});
        rejected(std::move(input));
    }

    const auto max_count = std::numeric_limits<int>::max();
    const auto max_size = std::numeric_limits<int64_t>::max();
    for (const bool unsigned_json : {false, true}) {
        for (const bool maximum : {false, true}) {
            auto input = original;
            const auto count = maximum ? max_count : 0;
            const auto size = maximum ? max_size : int64_t{0};
            input["MaxKeys"] = unsigned_json ? json(static_cast<uint64_t>(count)) : json(int64_t{count});
            input["KeyCount"] = unsigned_json ? json(static_cast<uint64_t>(count)) : json(int64_t{count});
            input["Contents"][0]["Size"] = unsigned_json ? json(static_cast<uint64_t>(size)) : json(size);
            output.fromJson(input, 2);
            CHECK(output.getMaxKeys() == count && output.getKeyCount() == count);
            CHECK(output.getContents().size() == 1 && output.getContents()[0].getSize() == size);
            CHECK(output.getRequestInfo().getHeaders().at("x-fixture") == "retained");
        }
    }
    // Count/entry consistency is the caller's protocol contract; these tests
    // isolate the typed model's numeric range without allocating count items.
    output.fromJsonString(R"({"IsTruncated":false,"Contents":[{"Key":"old/default"}]})");
    CHECK(output.getMaxKeys() == 0 && output.getKeyCount() == 0 && output.getContents()[0].getSize() == 0);
}
}  // namespace

int main() {
    try {
        ExactBodyAndShapeLimits();
        OversizeHeadersAndChunksNeverPublishPrefix();
        AllocationFailureDoesNotEscapeReceive();
        DepthAndEventsStopBeforeTypedOutput();
        MalformedAndServiceErrorsRemainErrors();
        InvalidLocalLimitsFailBeforeSubmission();
        ListDirectDecodeAndAtomicPublication();
        ListIntegerSchemaAndAtomicPublication();
        std::cout << "bounded JSON: 8 real-pipeline/parser contract groups passed\n";
        return 0;
    } catch (const std::exception& error) {
        fail_next_allocation = false;
        std::cerr << "bounded JSON contract failed: " << error.what() << '\n';
        return 1;
    }
}
