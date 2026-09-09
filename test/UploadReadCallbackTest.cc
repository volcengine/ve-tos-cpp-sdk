#include <gtest/gtest.h>
#include <stdexcept>

// Exercise the actual private curl callback without exposing a public test API.
#include "transport/http/HttpClient.cc"

using namespace VolcengineTos;

namespace {
class FaultBuffer : public std::streambuf {
public:
    enum Mode { Negative, Excessive, Throw };
    explicit FaultBuffer(Mode mode) : mode_(mode) {
    }

protected:
    std::streamsize xsgetn(char*, std::streamsize n) override {
        if (mode_ == Throw) {
            throw std::runtime_error("source read failed");
        }
        return mode_ == Negative ? -1 : n + 1;
    }

private:
    Mode mode_;
};

class UploadReadCallbackTest : public ::testing::Test {
protected:
    HttpRequest request;
    ResourceManager state{};
    char buffer[16]{};

    void SetUp() override {
        state.httpReq = &request;
        state.total = 6;
        state.enableCrc64 = true;
        state.dataTransferType = 1;
        request.setBody(std::make_shared<std::stringstream>("abcdef"));
    }

    size_t read(size_t n = 16) {
        return sendBody(buffer, 1, n, &state);
    }

    void expectAbort() {
        EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read());
        EXPECT_EQ(0, state.send);
        EXPECT_EQ(0u, state.sendCrc64Value);
    }
};

TEST_F(UploadReadCallbackTest, NormalReadsRespectLengthAndCrc) {
    state.total = 5;
    EXPECT_EQ(3u, read(3));
    EXPECT_EQ(2u, read());
    EXPECT_EQ(0u, read());
    EXPECT_EQ(5, state.send);
    EXPECT_EQ(CRC64::CalcCRC(0, const_cast<char*>("abcde"), 5), state.sendCrc64Value);
    EXPECT_EQ('f', request.Body()->peek());
}

TEST_F(UploadReadCallbackTest, UnknownLengthAcceptsPartialFinalRead) {
    state.total = -1;
    EXPECT_EQ(6u, read());
    EXPECT_EQ(0u, read());
    EXPECT_EQ(6, state.send);
}

TEST_F(UploadReadCallbackTest, PrematureEofAborts) {
    state.total = 7;
    expectAbort();
}

TEST_F(UploadReadCallbackTest, NegativeAndExcessiveCountsAbortBeforeCrc) {
    for (auto mode : {FaultBuffer::Negative, FaultBuffer::Excessive}) {
        FaultBuffer source(mode);
        request.setBody(std::make_shared<std::iostream>(&source));
        expectAbort();
    }
}

TEST_F(UploadReadCallbackTest, StreamExceptionsAndBadStateAbort) {
    FaultBuffer source(FaultBuffer::Throw);
    request.setBody(std::make_shared<std::iostream>(&source));
    expectAbort();
    request.setBody(std::make_shared<std::iostream>(&source));
    request.Body()->exceptions(std::ios::badbit);
    expectAbort();
}

TEST_F(UploadReadCallbackTest, EofExceptionDoesNotEscapeCallback) {
    request.Body()->exceptions(std::ios::failbit);
    state.total = 7;
    expectAbort();
}

TEST_F(UploadReadCallbackTest, ProgressExceptionDoesNotEscapeCallback) {
    state.progress = [](const std::shared_ptr<DataTransferStatus>&) { throw std::runtime_error("progress failed"); };
    expectAbort();
}

TEST_F(UploadReadCallbackTest, CompletionProgressExceptionDoesNotEscapeCallback) {
    state.dataTransferType = 2;
    state.progress = [](const std::shared_ptr<DataTransferStatus>&) { throw std::runtime_error("progress failed"); };
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read());
    EXPECT_EQ(0u, state.sendCrc64Value);
}

TEST_F(UploadReadCallbackTest, EmptyUnknownStreamIsNormalEof) {
    request.setBody(std::make_shared<std::stringstream>());
    state.total = -1;
    EXPECT_EQ(0u, read());
    EXPECT_EQ(0, state.send);
}

TEST_F(UploadReadCallbackTest, InvalidLengthsAbortBeforeReading) {
    state.send = -1;
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read());
    state.send = 7;
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read());
    state.send = 0;
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT),
              sendBody(buffer, 2, (std::numeric_limits<size_t>::max)(), &state));
    state.total = -1;
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read((std::numeric_limits<size_t>::max)()));
    state.send = (std::numeric_limits<int64_t>::max)();
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), read());
    EXPECT_EQ(0u, state.sendCrc64Value);
}

TEST_F(UploadReadCallbackTest, NullBodyAndEmptyRequests) {
    request.setBody(nullptr);
    expectAbort();
    state.total = 0;
    EXPECT_EQ(0u, read());
    EXPECT_EQ(0u, read(0));
    EXPECT_EQ(static_cast<size_t>(CURL_READFUNC_ABORT), sendBody(buffer, 1, 1, nullptr));
}
}  // namespace
