#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <functional>
#include <type_traits>
#include <utility>

namespace VolcengineTos {
// 无锁缓存的节点结构体（存储键值对 + 链表指针 + 版本号 + 删除标记）
template <typename K, typename V>
struct LockFreeCacheNode {
    K key;                                       // 键（支持哈希和比较）
    V value;                                     // 值（结构体需支持拷贝构造）
    std::atomic<bool> deleted;                   // 逻辑删除标记（true = 已删除）
    std::atomic<uint64_t> version;               // 版本号（每次修改递增，解决 ABA 问题）
    std::atomic<LockFreeCacheNode<K, V>*> next;  // 下一个节点的原子指针

    // 构造函数（初始化节点）
    LockFreeCacheNode(K&& k, V&& v)
            : key(std::move(k)), value(std::move(v)), deleted(false), version(0), next(nullptr) {
    }
};

// 无锁并发缓存类（C++11 兼容）
template <typename K, typename V,
          size_t BucketCount = 1024,            // 哈希桶数量（默认 1024，可调整）
          typename Hash = std::hash<K>,         // 键的哈希函数（默认 std::hash）
          typename KeyEqual = std::equal_to<K>  // 键的比较函数（默认 ==）
          >
class LockFreeCache {
    static_assert(std::is_copy_constructible<V>::value, "Value type must be copy-constructible");
    static_assert(std::is_default_constructible<V>::value, "Value type must be default-constructible");
    using Node = LockFreeCacheNode<K, V>;

public:
    // 构造函数（初始化哈希桶）
    LockFreeCache() : buckets_(BucketCount) {
        for (auto& bucket : buckets_) {
            bucket.store(nullptr, std::memory_order_release);
        }
    }

    // 析构函数（清理所有节点，避免内存泄漏）
    ~LockFreeCache() {
        for (auto& bucket : buckets_) {
            Node* node = bucket.load(std::memory_order_acquire);
            while (node != nullptr) {
                Node* next = node->next.load(std::memory_order_acquire);
                delete node;
                node = next;
            }
            bucket.store(nullptr, std::memory_order_release);
        }
    }

    // 禁止拷贝和移动（无锁结构拷贝复杂，避免误用）
    LockFreeCache(const LockFreeCache&) = delete;
    LockFreeCache& operator=(const LockFreeCache&) = delete;
    LockFreeCache(LockFreeCache&&) = delete;
    LockFreeCache& operator=(LockFreeCache&&) = delete;

    /**
     * @brief 插入/更新缓存项
     * @param key 缓存键
     * @param value 缓存值（结构体）
     * @return 是否成功（失败仅可能是内存分配失败）
     */
    bool put(K key, V value) {
        const size_t bucket_idx = hash_(key) % BucketCount;
        auto& bucket = buckets_[bucket_idx];

        // 循环尝试插入（CAS 可能失败，需要重试）
        while (true) {
            Node* head = bucket.load(std::memory_order_acquire);
            Node* curr = head;
            Node* prev = nullptr;

            // 1. 遍历链表，查找是否已存在该键（未删除）
            while (curr != nullptr) {
                // 跳过已删除的节点
                if (!curr->deleted.load(std::memory_order_acquire)) {
                    if (key_equal_(curr->key, key)) {
                        // 2. 已存在：更新值（通过版本号保证原子性）
                        uint64_t old_version = curr->version.load(std::memory_order_acquire);
                        // 拷贝新值（避免修改时影响读取）
                        V new_value = value;
                        // CAS 更新：版本号一致才允许更新，更新后版本号+1
                        if (curr->version.compare_exchange_weak(old_version, old_version + 1, std::memory_order_release,
                                                                std::memory_order_relaxed)) {
                            curr->value = std::move(new_value);
                            return true;
                        }
                        // 版本号不一致，说明有并发更新，重试
                        break;
                    }
                }
                prev = curr;
                curr = curr->next.load(std::memory_order_acquire);
            }

            // 3. 未存在：创建新节点（内存分配可能失败，需处理）
            Node* new_node = new (std::nothrow) Node(std::move(key), std::move(value));
            if (new_node == nullptr) {
                return false;
            }

            // 4. 插入新节点到链表头部（CAS 原子操作）
            new_node->next.store(head, std::memory_order_release);
            if (bucket.compare_exchange_weak(head, new_node, std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }

            // CAS 失败（有并发插入），释放新节点并重试
            delete new_node;
        }
    }

    /**
     * @brief 查询缓存项
     * @param key 缓存键
     * @param out_value 输出参数：存储查询到的值（结构体）
     * @return 是否查询成功（true = 找到未删除的项）
     */
    bool get(const K& key, V& out_value) {
        const size_t bucket_idx = hash_(key) % BucketCount;
        auto& bucket = buckets_[bucket_idx];

        // 循环尝试读取（避免并发修改导致的脏读）
        while (true) {
            Node* curr = bucket.load(std::memory_order_acquire);
            while (curr != nullptr) {
                // 跳过已删除的节点
                if (curr->deleted.load(std::memory_order_acquire)) {
                    curr = curr->next.load(std::memory_order_acquire);
                    continue;
                }

                // 找到目标键：通过版本号双重检查，避免读取过程中节点被修改
                if (key_equal_(curr->key, key)) {
                    uint64_t old_version = curr->version.load(std::memory_order_acquire);
                    // 拷贝值（结构体拷贝，需保证线程安全）
                    out_value = curr->value;
                    // 再次检查版本号：若一致，说明拷贝过程中无修改；否则重试
                    if (curr->version.load(std::memory_order_acquire) == old_version) {
                        return true;
                    }
                    // 版本号不一致，重试整个查询流程
                    break;
                }

                curr = curr->next.load(std::memory_order_acquire);
            }

            // 遍历完链表仍未找到，返回失败
            return false;
        }
    }

    /**
     * @brief 删除缓存项（逻辑删除，避免内存释放冲突）
     * @param key 缓存键
     * @return 是否删除成功（true = 找到并标记删除）
     */
    bool erase(const K& key) {
        const size_t bucket_idx = hash_(key) % BucketCount;
        auto& bucket = buckets_[bucket_idx];

        Node* curr = bucket.load(std::memory_order_acquire);
        while (curr != nullptr) {
            // 跳过已删除的节点
            if (curr->deleted.load(std::memory_order_acquire)) {
                curr = curr->next.load(std::memory_order_acquire);
                continue;
            }

            // 找到目标键：原子标记为删除（逻辑删除）
            if (key_equal_(curr->key, key)) {
                bool expected = false;
                if (curr->deleted.compare_exchange_strong(expected, true, std::memory_order_release,
                                                          std::memory_order_relaxed)) {
                    // 版本号递增，通知其他线程节点状态变化
                    curr->version.fetch_add(1, std::memory_order_release);
                    return true;
                }
                // CAS 失败（并发删除），返回成功（已被其他线程删除）
                return true;
            }

            curr = curr->next.load(std::memory_order_acquire);
        }

        // 未找到目标键
        return false;
    }

    /**
     * @brief 清空缓存（仅在无并发访问时调用，如程序退出前）
     */
    void clear() {
        for (auto& bucket : buckets_) {
            Node* node = bucket.exchange(nullptr, std::memory_order_acquire);
            while (node != nullptr) {
                Node* next = node->next.load(std::memory_order_acquire);
                delete node;
                node = next;
            }
        }
    }

private:
    std::vector<std::atomic<Node*>> buckets_;  // 哈希桶（每个桶是原子指针）
    Hash hash_;                                // 哈希函数实例
    KeyEqual key_equal_;                       // 键比较函数实例
};
}  // namespace VolcengineTos