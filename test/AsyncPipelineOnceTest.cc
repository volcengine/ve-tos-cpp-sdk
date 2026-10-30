#include "executor/ProcessingPipline.h"

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

using namespace VolcengineTos;

#define CHECK(expression)                                                                                 \
    do {                                                                                                  \
        if (!(expression)) {                                                                              \
            throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) +               \
                                     " " #expression);                                                    \
        }                                                                                                 \
    } while (false)

namespace {
struct Output {
    int value = 0;
    int64_t getRetryAfter() const { return 0; }
};
using Pipeline = ProcessingPipline<int, Output>;
using Reply = Outcome<TosError, Output>;
using Done = std::function<void(std::shared_ptr<HttpResponse>)>;

std::shared_ptr<HttpResponse> Response(int code = 200) {
    auto response = std::make_shared<HttpResponse>();
    response->setStatus(0);
    response->setStatusCode(code);
    return response;
}

// The external sender is the only fake boundary. Parsing, delivery, retry,
// cleanup and reset all execute the actual SDK ProcessingPipline template.
struct Transport {
    size_t sends = 0;
    std::vector<Done> completions;
    std::function<void(const Done&)> on_send;

    void Attach(Pipeline& pipeline) {
        pipeline.httpRequestSender(
            [this](std::shared_ptr<HttpRequest>, const OnDataReceiveWithEvent&, const OnDataSendWithEvent&,
                   const Done& done, const OnRequestStart&, const OnHttpStatusSet&, const OnContentLengthSet&) {
                ++sends;
                completions.push_back(done);
                if (on_send) on_send(done);
            });
    }
};

struct Observed {
    size_t callbacks = 0;
    size_t parses = 0;
    size_t finishes = 0;
    Reply result;
    std::function<void(Reply&)> on_done;
};

void Configure(Pipeline& pipeline, Transport& transport, Observed& observed) {
    pipeline.input2HttpRequest([](int&) {
        auto request = std::make_shared<HttpRequest>();
        request->setUrl(Url("http://127.0.0.1:1/offline-contract"));
        return request;
    });
    pipeline.expectResponse2Outcome(
        [&observed](std::shared_ptr<HttpRequest>, std::shared_ptr<HttpResponse>, TosError&, Output& output) {
            ++observed.parses;
            output.value = 73;
        });
    pipeline.unexpectResponse2Outcome(
        [&observed](std::shared_ptr<HttpRequest>, std::shared_ptr<HttpResponse> response, TosError& error,
                    Output&) {
            ++observed.parses;
            error.setStatusCode(response->statusCode());
            error.setCode("RemoteStatus");
        });
    pipeline.onRequestDone([&observed](Reply& outcome) {
        ++observed.callbacks;
        observed.result = outcome;
        if (observed.on_done) observed.on_done(outcome);
    });
    pipeline.afterPiplineFinish([&observed] { ++observed.finishes; });
    transport.Attach(pipeline);
}

struct Fixture {
    Pipeline pipeline{1};
    Transport transport;
    Observed observed;
    Fixture() { Configure(pipeline, transport, observed); }
};

void NormalResponseAndSequentialDuplicateCompleteOnce() {
    Fixture f;
    f.pipeline.asyncExecute();
    CHECK(f.transport.sends == 1 && f.observed.callbacks == 0 && f.observed.finishes == 0);
    f.transport.completions[0](Response());
    CHECK(f.observed.callbacks == 1 && f.observed.parses == 1 && f.observed.finishes == 1);
    CHECK(f.observed.result.isSuccess() && f.observed.result.result().value == 73);
    f.transport.completions[0](Response(500));
    f.pipeline.callbackError("late local error");
    f.pipeline.asyncExecute();
    CHECK(f.transport.sends == 1 && f.observed.callbacks == 1 && f.observed.parses == 1 &&
          f.observed.finishes == 1);
}

void ThrowingCompletionDoesNotBecomeSecondFailure() {
    for (bool standard : {false, true}) {
        Fixture f;
        f.observed.on_done = [standard](Reply&) {
            if (standard) throw std::runtime_error("user completion failed");
            throw 17;
        };
        f.pipeline.asyncExecute();
        f.transport.completions[0](Response());
        CHECK(f.observed.callbacks == 1 && f.observed.parses == 1 && f.observed.finishes == 1);
        CHECK(f.observed.result.isSuccess() && f.observed.result.result().value == 73);
        f.transport.completions[0](Response());
        CHECK(f.observed.callbacks == 1 && f.observed.finishes == 1);
    }
}

void CallbackErrorPublishesBeforeThrowOrReentry() {
    Fixture f;
    f.pipeline.inputCheck([](int&) { return "invalid offline input"; });
    f.observed.on_done = [&f](Reply&) {
        f.pipeline.callbackError("reentered error");
        f.pipeline.asyncExecute();
        throw std::runtime_error("error completion failed");
    };
    f.pipeline.asyncExecute();
    CHECK(f.transport.sends == 0 && f.observed.callbacks == 1 && f.observed.parses == 0 &&
          f.observed.finishes == 1);
    CHECK(!f.observed.result.isSuccess() && f.observed.result.error().isClientError());
    CHECK(f.observed.result.error().getMessage() == "invalid offline input");
}

void EarlyDataErrorWaitsForTransportCompletionToFinish() {
    Fixture f;
    f.observed.on_done = [](Reply&) { throw std::runtime_error("data-error callback failed"); };
    f.pipeline.asyncExecute();
    f.pipeline.callbackError("data source failed");
    CHECK(f.observed.callbacks == 1 && f.observed.finishes == 0 && f.observed.parses == 0);
    CHECK(f.observed.result.error().getMessage() == "data source failed");
    f.transport.completions[0](Response());
    f.transport.completions[0](Response());
    CHECK(f.observed.callbacks == 1 && f.observed.finishes == 1 && f.observed.parses == 0);
}

void ParserAndDecoratorExceptionsProduceOneClientError() {
    for (int kind = 0; kind < 5; ++kind) {
        Fixture f;
        if (kind < 2) {
            f.pipeline.expectResponse2Outcome(
                [kind](std::shared_ptr<HttpRequest>, std::shared_ptr<HttpResponse>, TosError&, Output&) {
                    if (kind == 0) throw std::runtime_error("response parser failed");
                    throw 23;
                });
        } else if (kind == 2) {
            f.pipeline.unexpectResponse2Outcome(
                [](std::shared_ptr<HttpRequest>, std::shared_ptr<HttpResponse>, TosError&, Output&) {
                    throw std::runtime_error("error parser failed");
                });
        } else if (kind == 3) {
            f.pipeline.decorateOutcome([](Reply&) { throw std::runtime_error("decorator failed"); });
        }
        f.observed.on_done = [](Reply&) { throw std::runtime_error("failure observer failed"); };
        f.pipeline.asyncExecute();
        f.transport.completions[0](kind == 4 ? nullptr : Response(kind == 2 ? 503 : 200));
        CHECK(f.observed.callbacks == 1 && f.observed.finishes == 1);
        CHECK(!f.observed.result.isSuccess() && f.observed.result.error().isClientError());
        CHECK(f.observed.result.error().getCode() == "UnhandledException");
        CHECK(f.observed.result.error().getRequestUrl() == "http://127.0.0.1:1/offline-contract");
        if (kind != 4) CHECK(f.observed.result.error().getStatusCode() == (kind == 2 ? 503 : 200));
    }
}

void SynchronousSenderAndBuilderExceptionsFinishOnce() {
    for (int kind = 0; kind < 4; ++kind) {
        Fixture f;
        if (kind < 2) {
            f.transport.on_send = [kind](const Done&) {
                if (kind == 0) throw std::runtime_error("sender failed before completion");
                throw 31;
            };
        } else {
            f.pipeline.input2HttpRequest([kind](int&) -> std::shared_ptr<HttpRequest> {
                if (kind == 2) throw std::runtime_error("request builder failed");
                throw 37;
            });
        }
        f.observed.on_done = [](Reply&) { throw std::runtime_error("error observer failed"); };
        f.pipeline.asyncExecute();
        CHECK(f.observed.callbacks == 1 && f.observed.finishes == 1 && f.observed.parses == 0);
        CHECK(!f.observed.result.isSuccess() && f.observed.result.error().getCode() == "UnhandledException");
        if (!f.transport.completions.empty()) f.transport.completions[0](Response());
        CHECK(f.observed.callbacks == 1 && f.observed.finishes == 1);
    }
}

void InlineCompletionAndSenderThrowCannotUseDeletedPipeline() {
    for (int throw_kind = 0; throw_kind < 3; ++throw_kind) {
        Transport transport;
        Observed observed;
        size_t destroyed = 0;
        bool alive_after_inline_completion = false;
        auto* pipeline = new Pipeline(1);
        Configure(*pipeline, transport, observed);
        pipeline->onDestroy([&] { ++destroyed; });
        pipeline->afterPiplineFinish([pipeline] { delete pipeline; });
        transport.on_send = [&](const Done& done) {
            done(Response());
            alive_after_inline_completion = destroyed == 0;
            if (throw_kind == 1) throw std::runtime_error("sender threw after completing");
            if (throw_kind == 2) throw 41;
        };
        observed.on_done = [](Reply&) { throw std::runtime_error("user observer failed"); };
        pipeline->asyncExecute();
        CHECK(alive_after_inline_completion && destroyed == 1);
        CHECK(observed.callbacks == 1 && observed.finishes == 1 && observed.result.isSuccess());
        // Only the external callback/token remains. It must not touch the raw
        // pipeline pointer after its finalizer has deleted that pipeline.
        transport.completions[0](Response());
        CHECK(destroyed == 1 && observed.callbacks == 1 && observed.finishes == 1);
    }
}

void CompletionAndFinalizerReentryDoNotFinishTwice() {
    Fixture f;
    std::vector<int> finalizer_order;
    f.pipeline.afterPiplineFinish([&] {
        finalizer_order.push_back(1);
        f.pipeline.asyncExecute();
        f.pipeline.callbackError("finalizer reentry");
    });
    f.pipeline.afterPiplineFinish([&] {
        finalizer_order.push_back(2);
        throw std::runtime_error("cleanup hook failed");
    });
    f.observed.on_done = [&](Reply&) {
        f.pipeline.callbackError("callback reentry");
        f.pipeline.asyncExecute();
        f.transport.completions[0](Response());
        throw std::runtime_error("completion failed after reentry");
    };
    f.pipeline.asyncExecute();
    f.transport.completions[0](Response());
    CHECK(f.transport.sends == 1 && f.observed.callbacks == 1 && f.observed.parses == 1 &&
          f.observed.finishes == 1);
    CHECK(finalizer_order == std::vector<int>({2, 1}));
}

void MissingCompletionStillRunsCleanup() {
    Pipeline pipeline(1);
    size_t finishes = 0;
    pipeline.afterPiplineFinish([&] { ++finishes; });
    // Missing request builder uses callbackError, with no user callback set.
    pipeline.asyncExecute();
    CHECK(finishes == 1);
    pipeline.asyncExecute();
    CHECK(finishes == 1);
}

void PoolResetSeparatesOldAndNewCompletionGenerations() {
    Pipeline pipeline(1);
    Transport transport;
    Observed first, second, third;
    size_t recycled = 0;
    Configure(pipeline, transport, first);
    pipeline.afterPiplineFinish([&] {
        pipeline.recycle();
        ++recycled;
    });
    first.on_done = [](Reply&) { throw std::runtime_error("first observer failed"); };
    pipeline.asyncExecute();
    Done old_completion = transport.completions[0];
    old_completion(Response());
    CHECK(first.callbacks == 1 && first.finishes == 1 && recycled == 1);
    old_completion(Response());
    pipeline.resetForReuse(2);
    CHECK(pipeline.getInput() == 2);
    Configure(pipeline, transport, second);
    old_completion(Response());
    CHECK(second.callbacks == 0 && second.finishes == 0);
    pipeline.asyncExecute();
    old_completion(Response());
    CHECK(transport.sends == 2 && second.callbacks == 0);
    transport.completions[1](Response());
    CHECK(second.callbacks == 1 && second.finishes == 1 && second.result.isSuccess());
    CHECK(first.callbacks == 1 && first.finishes == 1 && recycled == 1);
    pipeline.resetForReuse(3);
    Configure(pipeline, transport, third);
    pipeline.inputCheck([](int&) { return "third input invalid"; });
    pipeline.asyncExecute();
    old_completion(Response());
    transport.completions[1](Response());
    CHECK(third.callbacks == 1 && third.finishes == 1 && !third.result.isSuccess());
    CHECK(third.result.error().getMessage() == "third input invalid" && transport.sends == 2);
}

void InlineRetryFinishesOnlyAfterNestedSendersReturn() {
    Fixture f;
    // Exercise the pipeline retry state machine with an allowlisted name.
    // The real getObject helper currently uses "getObject", not this name;
    // its retry-policy mismatch remains a separate production coverage gap.
    f.pipeline.userName("getObjectAsync").enableRetry(1, 0);
    bool premature_finish = false;
    f.transport.on_send = [&](const Done& done) {
        done(Response(f.transport.sends == 1 ? 503 : 200));
        premature_finish = premature_finish || f.observed.finishes != 0;
    };
    f.pipeline.asyncExecute();
    CHECK(!premature_finish && f.transport.sends == 2);
    CHECK(f.observed.callbacks == 1 && f.observed.parses == 2 && f.observed.finishes == 1);
    CHECK(f.observed.result.isSuccess());
}

void WorkerCompletionOverlapsSubmittingSenderWithoutEarlyDeletion() {
    // Both orderings are synchronized explicitly. No sleeps, real HTTP, or
    // unsupported concurrent duplicate parser invocations are involved.
    for (bool finish_before_sender_returns : {false, true}) {
        Transport transport;
        Observed observed;
        size_t destroyed = 0;
        std::mutex mutex;
        std::condition_variable changed;
        bool allow_completion = finish_before_sender_returns;
        bool worker_finished = false;
        bool alive_at_sender_return = false;
        std::thread worker;
        auto* pipeline = new Pipeline(1);
        Configure(*pipeline, transport, observed);
        pipeline->onDestroy([&] { ++destroyed; });
        pipeline->afterPiplineFinish([pipeline] { delete pipeline; });
        transport.on_send = [&](const Done& done) {
            worker = std::thread([&, done] {
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    changed.wait(lock, [&] { return allow_completion; });
                }
                done(Response());
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    worker_finished = true;
                }
                changed.notify_one();
            });
            if (finish_before_sender_returns) {
                std::unique_lock<std::mutex> lock(mutex);
                changed.wait(lock, [&] { return worker_finished; });
            }
            alive_at_sender_return = destroyed == 0;
        };
        observed.on_done = [](Reply&) { throw std::runtime_error("worker observer failed"); };
        pipeline->asyncExecute();
        if (!finish_before_sender_returns) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                allow_completion = true;
            }
            changed.notify_one();
        }
        worker.join();
        CHECK(alive_at_sender_return && destroyed == 1 && transport.sends == 1);
        CHECK(observed.callbacks == 1 && observed.parses == 1 && observed.finishes == 1);
        CHECK(observed.result.isSuccess());
        transport.completions[0](Response());
        CHECK(destroyed == 1 && observed.callbacks == 1 && observed.finishes == 1);
    }
}
}  // namespace

int main() {
    try {
        NormalResponseAndSequentialDuplicateCompleteOnce();
        ThrowingCompletionDoesNotBecomeSecondFailure();
        CallbackErrorPublishesBeforeThrowOrReentry();
        EarlyDataErrorWaitsForTransportCompletionToFinish();
        ParserAndDecoratorExceptionsProduceOneClientError();
        SynchronousSenderAndBuilderExceptionsFinishOnce();
        InlineCompletionAndSenderThrowCannotUseDeletedPipeline();
        CompletionAndFinalizerReentryDoNotFinishTwice();
        MissingCompletionStillRunsCleanup();
        PoolResetSeparatesOldAndNewCompletionGenerations();
        InlineRetryFinishesOnlyAfterNestedSendersReturn();
        WorkerCompletionOverlapsSubmittingSenderWithoutEarlyDeletion();
        std::cout << "AsyncPipelineOnceTest: 12 groups passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
