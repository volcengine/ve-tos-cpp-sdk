#include "utils/ConcurrentQueue.h"
#include "utils/LockFreeQueue.h"
#include "logger/logger.h"
#include "AsyncEvent.h"
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace VolcengineTos;
#define CHECK(x)                                \
    do {                                        \
        if (!(x)) throw std::runtime_error(#x); \
    } while (false)

int main() {
    try {
        constexpr size_t producers = 8, each = 20000, count = producers * each;
        ConcurrentQueue<size_t> queue;
        std::vector<size_t> values(count);
        std::vector<std::atomic<unsigned>> seen(count);
        for (size_t i = 0; i != count; ++i) {
            values[i] = i;
            seen[i] = 0;
        }
        std::atomic<size_t> consumed{0};
        std::atomic<bool> start{false};
        std::vector<std::thread> threads;
        for (size_t p = 0; p != producers; ++p)
            threads.emplace_back([&, p] {
                while (!start) std::this_thread::yield();
                for (size_t i = p * each; i != (p + 1) * each; ++i) queue.push(&values[i]);
            });
        for (unsigned c = 0; c != 4; ++c)
            threads.emplace_back([&] {
                while (!start) std::this_thread::yield();
                while (consumed < count) {
                    if (auto* item = queue.pop()) {
                        ++seen[*item];
                        ++consumed;
                    } else
                        std::this_thread::yield();
                }
            });
        start = true;
        for (auto& thread : threads) thread.join();
        CHECK(queue.unsafeEmpty());
        for (const auto& value : seen) CHECK(value == 1);

        // Capacity is checked atomically with publication, including equality.
        threads.clear();
        std::atomic<unsigned> accepted{0};
        for (size_t p = 0; p != producers; ++p)
            threads.emplace_back([&, p] {
                for (size_t i = p * each; i != (p + 1) * each; ++i)
                    if (queue.tryPush(&values[i], 17)) ++accepted;
            });
        for (auto& thread : threads) thread.join();
        CHECK(accepted == 17 && queue.unsafeSize() == 17);
        CHECK(queue.popIf([](size_t*) { return false; }) == nullptr);
        CHECK(queue.unsafeSize() == 17);
        while (queue.pop()) {
        }
        LockFreeQueue<size_t> legacy_name;
        legacy_name.push(&values[0]);
        CHECK(legacy_name.pop() == &values[0]);

        auto& logger = Logger::getInstance();
        logger.setAsyncLogLevel(ERROR);
        bool evaluated = false;
        logger.debugLazy([&](Logger&) { evaluated = true; });
        logger.infoLazy([&](Logger&) { evaluated = true; });
        CHECK(!evaluated);
        logger.setMaxQueueSize(0);
        const auto drops = logger.droppedMessages();
        // Old logger busy-waited forever at zero capacity. No output is emitted.
        for (unsigned i = 0; i != 10000; ++i) logger.error("saturated");
        CHECK(logger.droppedMessages() == drops + 10000);

        logger.setAsyncLogLevel(INFO);
        const auto before_throw = logger.droppedMessages();
        logger.infoLazy([](Logger&) { throw std::runtime_error("formatting failure"); });
        CHECK(logger.droppedMessages() == before_throw + 1);
        // Exercise actual formatting with INFO enabled, not just its disabled
        // path. Different timestamps must not share libc's static tm buffer.
        std::atomic<bool> bad_time{false};
        std::atomic<unsigned> formatted{0};
        threads.clear();
        for (unsigned t = 0; t < 8; ++t) {
            AsyncEvent reference;
            const int64_t timestamp = 1700000000000LL + t * 86400000LL;
            reference.setResumeAt(timestamp);
            const auto expected = reference.toString();
            threads.emplace_back([&, timestamp, expected] {
                AsyncEvent event;
                event.setResumeAt(timestamp);
                for (unsigned i = 0; i < 2000; ++i) {
                    logger.infoLazy([&](Logger& active_logger) {
                        const auto message = event.toString();
                        if (message != expected) bad_time = true;
                        ++formatted;
                        active_logger.info(message);
                    });
                }
            });
        }
        for (auto& thread : threads) thread.join();
        CHECK(!bad_time && formatted == 16000);
        logger.setAsyncLogLevel(ERROR);
        logger.setMaxQueueSize(10000);
        std::cout << "PASS: MPMC 160000 items, strict capacity, deferred entries, lazy/drop logging\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
