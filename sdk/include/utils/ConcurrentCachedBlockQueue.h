#pragma once
#include <stdexcept>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace VolcengineTos {

template <typename T>
struct ConcurrentQueueNode {
    T* data;
    ConcurrentQueueNode<T>* next;  // 保持普通指针（锁保护下无需原子指针）

    ConcurrentQueueNode() : data(nullptr), next(nullptr) {
    }

    // 重置节点状态，用于复用
    void reset() {
        data = nullptr;
        next = nullptr;
    }
};

template <typename T>
class ConcurrentNodeCache {
private:
    using Node = ConcurrentQueueNode<T>;
    std::vector<Node*> cache;
    const size_t max_cache_size;
    mutable std::mutex cache_mutex_;  // 新增：保护缓存池的线程安全

public:
    explicit ConcurrentNodeCache(size_t max_size = 4096) : max_cache_size(max_size) {
    }

    ~ConcurrentNodeCache() {
        // 释放所有缓存的节点（析构时需确保无其他线程访问）
        std::lock_guard<std::mutex> lock(cache_mutex_);
        for (Node* node : cache) {
            delete node;
        }
        cache.clear();
    }

    // 获取节点（从缓存或新建）- 线程安全
    Node* acquire() {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (!cache.empty()) {
            Node* node = cache.back();
            cache.pop_back();
            node->reset();  // 重置节点状态
            return node;
        }
        return new Node();  // 缓存为空时新建
    }

    // 回收节点（放入缓存或销毁）- 线程安全
    void release(Node* node) {
        if (!node)
            return;

        node->reset();
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (cache.size() < max_cache_size) {
            cache.push_back(node);  // 缓存未满则复用
        } else {
            delete node;  // 缓存已满则销毁
        }
    }
};

// 线程安全的缓存队列（支持阻塞超时pop）
template <typename T>
class ConcurrentCachedBlockQueue {
private:
    using Node = ConcurrentQueueNode<T>;
    Node* head;                         // 哨兵节点的头指针
    Node* tail;                         // 队列尾指针
    size_t size_;                       // 队列元素个数
    ConcurrentNodeCache<T> node_cache;            // 节点缓存池
    mutable std::mutex queue_mutex_;    // 新增：保护队列操作的互斥锁
    std::condition_variable queue_cv_;  // 新增：阻塞/唤醒条件变量

    // 核心出队逻辑（必须在已加锁的情况下调用）
    // 提取为独立函数，避免代码重复
    T* pop_impl() {
        Node* old_sentinel = head;      // 旧哨兵节点（即将回收）
        Node* first_node = head->next;  // 第一个有效节点

        T* data = first_node->data;  // 取出数据（用户需自行管理T的内存）

        // 哨兵节点前移（新哨兵 = 原第一个有效节点）
        head = first_node;
        node_cache.release(old_sentinel);  // 回收旧哨兵节点到缓存池

        size_--;
        return data;
    }

public:
    explicit ConcurrentCachedBlockQueue(size_t max_cache_size = 4096) : node_cache(max_cache_size) {
        Node* sentinel = node_cache.acquire();  // 初始化哨兵节点（简化边界条件）
        head = sentinel;
        tail = sentinel;
        size_ = 0;
    }

    ~ConcurrentCachedBlockQueue() {
        // 清空队列，回收所有节点到缓存池（析构时需确保无其他线程访问）
        std::lock_guard<std::mutex> lock(queue_mutex_);
        Node* current = head;
        while (current) {
            Node* next = current->next;
            node_cache.release(current);
            current = next;
        }
        head = nullptr;
        tail = nullptr;
        size_ = 0;
    }

    // 禁用拷贝/移动（多线程场景下避免浅拷贝导致的线程安全问题）
    ConcurrentCachedBlockQueue(const ConcurrentCachedBlockQueue&) = delete;
    ConcurrentCachedBlockQueue& operator=(const ConcurrentCachedBlockQueue&) = delete;
    ConcurrentCachedBlockQueue(ConcurrentCachedBlockQueue&&) = delete;
    ConcurrentCachedBlockQueue& operator=(ConcurrentCachedBlockQueue&&) = delete;

    /**
     * @brief 入队操作（线程安全，无阻塞）
     * @param data 入队的指针（非空，用户需确保data指向的内存有效）
     * @throw std::invalid_argument 当data为nullptr时抛出
     */
    void push(T* data) {
        if (!data) {
            throw std::invalid_argument("Cannot push null pointer to CachedQueue");
        }

        std::lock_guard<std::mutex> lock(queue_mutex_);  // 自动加锁/解锁

        // 从缓存池获取节点，复用内存
        Node* new_node = node_cache.acquire();
        new_node->data = data;

        // 挂载新节点到队尾
        tail->next = new_node;
        tail = new_node;  // 更新尾指针
        size_++;

        // 唤醒一个等待的出队线程（避免惊群效应，比notify_all更高效）
        queue_cv_.notify_one();
    }

    /**
     * @brief 非阻塞出队（线程安全）
     * @return 成功出队返回元素指针，队列为空返回nullptr
     */
    T* pop() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (empty()) {
            return nullptr;
        }
        return pop_impl();
    }

    /**
     * @brief 阻塞式出队（支持超时，线程安全）
     * @tparam Rep 时间单位的数值类型（如int、long）
     * @tparam Period 时间单位（如std::chrono::milliseconds、std::chrono::seconds）
     * @param timeout 阻塞超时时间（例如：std::chrono::milliseconds(500) 表示阻塞500ms）
     * @return 成功出队返回元素指针，超时/队列为空返回nullptr
     */
    template <typename Rep, typename Period>
    T* pop(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(queue_mutex_);  // 支持手动解锁的锁

        // 阻塞等待：直到队列非空 或 超时（自动处理操作系统虚假唤醒）
        bool has_data = queue_cv_.wait_for(lock, timeout, [this]() { return !empty(); }  // 谓词：队列非空时停止等待
        );

        if (!has_data) {
            return nullptr;  // 超时或队列仍为空
        }

        return pop_impl();  // 队列有数据，执行出队
    }

    /**
     * @brief 遍历队列所有元素（线程安全，返回元素指针副本）
     * @return 包含所有元素指针的vector（遍历期间队列被锁定，避免数据不一致）
     */
    std::vector<T*> traverse() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        std::vector<T*> result;
        Node* current = head->next;  // 从第一个有效节点开始遍历
        while (current) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }

    /**
     * @brief 获取队列大小（线程安全）
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return size_;
    }

    /**
     * @brief 判断队列是否为空（线程安全）
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return size_ == 0;
    }
};

}  // namespace VolcengineTos