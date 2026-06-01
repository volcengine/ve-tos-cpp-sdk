#pragma once
#include <atomic>
#include <cstddef>
#include <stdexcept>

namespace VolcengineTos {

// 无锁队列节点
template <typename T>
struct LockFreeQueueNode {
    T* data;
    std::atomic<LockFreeQueueNode<T>*> next;

    // 直接初始化数据，无需重置（无节点复用）
    explicit LockFreeQueueNode(T* data_ptr) : data(data_ptr), next(nullptr) {
    }
};

// 简化版无锁队列（删除节点缓存、traverse、snapshot）
template <typename T>
class LockFreeQueue {
private:
    using Node = LockFreeQueueNode<T>;
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    std::atomic<size_t> size_{};

public:
    LockFreeQueue() {
        // 哨兵节点（直接新建，无需缓存）
        Node* sentinel = new Node(nullptr);
        head.store(sentinel, std::memory_order_relaxed);
        tail.store(sentinel, std::memory_order_relaxed);
        size_.store(0, std::memory_order_relaxed);
    }

    ~LockFreeQueue() {
        // 清空队列，直接删除所有节点（无缓存回收）
        Node* current = head.load(std::memory_order_relaxed);
        while (current) {
            Node* next = current->next.load(std::memory_order_relaxed);
            delete current;
            current = next;
        }
    }

    // 入队操作（直接新建节点，无缓存获取）
    void push(T* data) {
        if (!data) {
            throw std::invalid_argument("Cannot push null pointer");
        }

        Node* new_node = new Node(data);  // 直接创建节点

        while (true) {
            Node* current_tail = tail.load(std::memory_order_acquire);
            Node* tail_next = current_tail->next.load(std::memory_order_acquire);

            if (current_tail != tail.load(std::memory_order_acquire))
                continue;
            if (tail_next != nullptr) {
                tail.compare_exchange_strong(current_tail, tail_next, std::memory_order_release,
                                             std::memory_order_relaxed);
                continue;
            }

            // 插入新节点并更新尾指针
            if (current_tail->next.compare_exchange_strong(tail_next, new_node, std::memory_order_release,
                                                           std::memory_order_relaxed)) {
                tail.compare_exchange_strong(current_tail, new_node, std::memory_order_release,
                                             std::memory_order_relaxed);
                size_.fetch_add(1, std::memory_order_relaxed);
                return;
            }
        }
    }

    // 出队操作（直接删除节点，无缓存回收）
    T* pop() {
        while (true) {
            Node* current_head = head.load(std::memory_order_acquire);
            Node* current_tail = tail.load(std::memory_order_acquire);
            Node* head_next = current_head->next.load(std::memory_order_acquire);

            if (current_head != head.load(std::memory_order_acquire))
                continue;
            if (current_head == current_tail) {
                if (head_next == nullptr)
                    return nullptr;  // 队列为空
                tail.compare_exchange_strong(current_tail, head_next, std::memory_order_release,
                                             std::memory_order_relaxed);
            } else {
                // 弹出节点并删除旧头节点
                if (head.compare_exchange_strong(current_head, head_next, std::memory_order_release,
                                                 std::memory_order_relaxed)) {
                    T* data = head_next->data;
                    delete current_head;  // 直接删除哨兵节点（无缓存）
                    size_.fetch_sub(1, std::memory_order_relaxed);
                    return data;
                }
            }
        }
    }

    // 保留原有的不安全大小查询接口
    size_t unsafeSize() const {
        return size_.load(std::memory_order_relaxed);
    }

    bool unsafeEmpty() const {
        return unsafeSize() == 0;
    }
};
}  // namespace VolcengineTos