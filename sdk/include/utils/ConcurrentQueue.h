#pragma once

#include <atomic>
#include <cstddef>
#include <deque>
#include <mutex>
#include <stdexcept>

namespace VolcengineTos {

// Payload ownership passes to the consumer after a successful push. The queue
// never deletes payloads. Node storage and reclamation share a short lock.
template <typename T>
class ConcurrentQueue {
   public:
    void push(T* data) { (void)tryPush(data, static_cast<size_t>(-1)); }
    bool tryPush(T* data, size_t limit) {
        if (!data) throw std::invalid_argument("Cannot push null pointer");
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= limit) return false;
        queue_.push_back(data);  // Allocation failure leaves ownership with caller.
        size_.store(queue_.size(), std::memory_order_relaxed);
        return true;
    }
    T* pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return nullptr;
        T* value = queue_.front();
        queue_.pop_front();
        size_.store(queue_.size(), std::memory_order_relaxed);
        return value;
    }
    // Skipped delayed requests stay queued with their admission slots. Unlike
    // pop/push rotation this never allocates. Predicate must not invoke user code.
    template <typename Predicate>
    T* popIf(Predicate ready) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = queue_.begin(); it != queue_.end(); ++it) {
            if (!ready(*it)) continue;
            T* value = *it;
            queue_.erase(it);
            size_.store(queue_.size(), std::memory_order_relaxed);
            return value;
        }
        return nullptr;
    }
    // Inspect queued work without consuming it or allocating a snapshot. The
    // visitor runs under the queue lock and must not invoke external code.
    template <typename Visitor>
    void inspect(Visitor visitor) const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto* value : queue_) visitor(value);
    }
    // Hints, not admission checks; use tryPush or an external capacity lease.
    size_t unsafeSize() const { return size_.load(std::memory_order_relaxed); }
    bool unsafeEmpty() const { return unsafeSize() == 0; }

   private:
    mutable std::mutex mutex_;
    std::deque<T*> queue_;
    std::atomic<size_t> size_{0};
};
}  // namespace VolcengineTos
