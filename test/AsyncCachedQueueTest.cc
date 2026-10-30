#include "utils/CachedQueue.h"
#include <cstdlib>
#include <iostream>
#include <new>

namespace {
bool fail_allocation = false;
size_t refused = 0;
}
void* operator new(std::size_t size) {
    if (fail_allocation) { ++refused; throw std::bad_alloc(); }
    if (void* p = std::malloc(size ? size : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }

int main() {
    int a = 1, b = 2, c = 3;
    VolcengineTos::CachedQueue<int> queue;
    queue.push(&a); queue.push(&b); queue.push(&c);
    fail_allocation = true;
    const bool removed_middle = queue.erase(&b);
    const bool removed_tail = queue.erase(&c);
    const bool missing = !queue.erase(&b);
    const bool popped = queue.pop() == &a && queue.pop() == nullptr && queue.empty() && queue.size() == 0;
    fail_allocation = false;
    queue.push(&a); queue.push(&b); // tail remains valid after erase/pop emptied the queue
    const bool order = queue.pop() == &a && queue.pop() == &b;
    auto* destroying = new VolcengineTos::CachedQueue<int>();
    destroying->push(&a); destroying->push(&b);
    fail_allocation = true;
    delete destroying;
    fail_allocation = false;
    if (!(removed_middle && removed_tail && missing && popped && order && refused >= 3)) {
        std::cerr << "cached queue allocation-failure invariant failed\n"; return 1;
    }
    std::cout << "PASS allocation-failing cached queue erase/pop/destruction\n";
}
