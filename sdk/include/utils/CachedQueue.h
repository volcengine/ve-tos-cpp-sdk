#pragma once
#include <stdexcept>
#include <vector>
#include <memory>

namespace VolcengineTos {

template <typename T>
struct QueueNode {
    T* data;
    QueueNode<T>* next;  // 普通指针替代原子指针

    QueueNode() : data(nullptr), next(nullptr) {
    }

    // 重置节点状态，用于复用
    void reset() {
        data = nullptr;
        next = nullptr;
    }
};

template <typename T>
class NodeCache {
private:
    using Node = QueueNode<T>;
    std::vector<Node*> cache;  // 单线程访问，无需锁保护
    const size_t max_cache_size;

public:
    explicit NodeCache(size_t max_size = 4096) : max_cache_size(max_size) {
    }

    ~NodeCache() {
        // 释放所有缓存的节点
        for (Node* node : cache) {
            delete node;
        }
        cache.clear();
    }

    // 获取节点（从缓存或新建）
    Node* acquire() {
        if (!cache.empty()) {
            Node* node = cache.back();
            cache.pop_back();
            node->reset();  // 重置节点状态
            return node;
        }
        return new Node();  // 缓存为空时新建
    }

    // 回收节点（放入缓存或销毁）
    void release(Node* node) {
        if (!node)
            return;

        node->reset();
        if (cache.size() < max_cache_size) {
            cache.push_back(node);  // 缓存未满则复用
        } else {
            delete node;  // 缓存已满则销毁
        }
    }
};

// 单线程队列（保留节点缓存）
template <typename T>
class CachedQueue {
private:
    using Node = QueueNode<T>;
    Node* head;               // 普通指针（单线程无需原子）
    Node* tail;               // 普通指针
    size_t size_;             // 普通变量记录大小
    NodeCache<T> node_cache;  // 节点缓存池

public:
    explicit CachedQueue(size_t max_cache_size = 4096) : node_cache(max_cache_size) {
        Node* sentinel = node_cache.acquire();  // 哨兵节点（简化边界条件）
        head = sentinel;
        tail = sentinel;
        size_ = 0;
    }

    ~CachedQueue() {
        // 清空队列，回收所有节点到缓存
        Node* current = head;
        while (current) {
            Node* next = current->next;
            node_cache.release(current);
            current = next;
        }
    }

    // 入队操作（单线程直接修改指针）
    void push(T* data) {
        if (!data) {
            throw std::invalid_argument("Cannot push null pointer");
        }

        Node* new_node = node_cache.acquire();
        new_node->data = data;

        // 直接将新节点挂到尾部
        tail->next = new_node;
        tail = new_node;  // 更新尾指针
        size_++;
    }

    // 出队操作（单线程直接修改指针）
    T* pop() {
        if (size_ == 0) {
            return nullptr;  // 队列为空
        }

        // 哨兵节点的next是第一个有效节点
        Node* old_head = head;
        Node* first_node = head->next;

        T* data = first_node->data;  // 取出数据

        // 更新头指针到下一个节点（哨兵节点前移）
        head = first_node;
        node_cache.release(old_head);  // 回收旧哨兵节点

        size_--;
        return data;
    }

    std::vector<T*> traverse() const {
        std::vector<T*> result;
        Node* current = head->next;  // 从第一个有效节点开始

        while (current) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }
};
}  // namespace VolcengineTos