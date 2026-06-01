#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include <cstring>

namespace VolcengineTos {

// 单线程使用的字节链表缓冲区，用于在上游更强场景下缓存未被回调消费完的数据。
// 典型使用：
//  - append() 将 libcurl 传入但未被用户回调消费完的尾部数据复制到链表中；
//  - drainTo() 在恢复阶段重放缓存数据到用户回调；
class LinkedBufferQueue {
public:
    struct Node {
        std::unique_ptr<char[]> buf;
        std::size_t len{0};      // 有效数据长度
        std::size_t consumed{0}; // 已消费字节数
        Node* next{nullptr};

        explicit Node(std::size_t capacity) : buf(new char[capacity]), len(0), consumed(0), next(nullptr) {
        }
    };

    LinkedBufferQueue() : head_(nullptr), tail_(nullptr), total_bytes_(0) {
    }

    ~LinkedBufferQueue() {
        clear();
    }

    LinkedBufferQueue(const LinkedBufferQueue&) = delete;
    LinkedBufferQueue& operator=(const LinkedBufferQueue&) = delete;
    LinkedBufferQueue(LinkedBufferQueue&&) = delete;
    LinkedBufferQueue& operator=(LinkedBufferQueue&&) = delete;

    bool empty() const {
        return total_bytes_ == 0;
    }

    std::size_t totalSize() const {
        return total_bytes_;
    }

    // 追加一段数据（内部复制）
    void append(const char* data, std::size_t len) {
        if (data == nullptr || len == 0) {
            return;
        }

        Node* node = new Node(len);
        std::memcpy(node->buf.get(), data, len);
        node->len = len;
        node->consumed = 0;
        node->next = nullptr;

        if (tail_ == nullptr) {
            head_ = node;
            tail_ = node;
        } else {
            tail_->next = node;
            tail_ = node;
        }
        total_bytes_ += len;
    }

    // 返回当前队头未消费数据的视图
    std::pair<char*, std::size_t> frontBytes() {
        if (head_ == nullptr) {
            return {nullptr, 0};
        }
        if (head_->consumed >= head_->len) {
            return {nullptr, 0};
        }
        return {head_->buf.get() + head_->consumed, head_->len - head_->consumed};
    }

    // 将缓冲区数据重放给回调；回调返回本次消费的字节数。
    // 规则：
    //  - 若回调返回 0，则立即停止重放；
    //  - 若返回值小于本次提供的字节数，则保留剩余数据并停止重放；
    //  - 若等于本次提供的字节数，则继续处理后续节点。
    template <typename Callback>
    std::size_t drainTo(Callback&& cb) {
        std::size_t total_used = 0;
        while (head_ != nullptr) {
            if (head_->consumed >= head_->len) {
                // 当前节点已完全消费，直接删除
                Node* old = head_;
                head_ = head_->next;
                if (head_ == nullptr) {
                    tail_ = nullptr;
                }
                delete old;
                continue;
            }

            char* data = head_->buf.get() + head_->consumed;
            std::size_t avail = head_->len - head_->consumed;
            if (avail == 0) {
                break;
            }

            std::size_t used = cb(data, avail);
            if (used == 0 || used > avail) {
                break;
            }

            head_->consumed += used;
            total_bytes_ -= used;
            total_used += used;

            if (head_->consumed < head_->len) {
                // 本节点仍有剩余数据，暂不继续向后遍历
                break;
            }

            // 当前节点消费完毕，释放节点
            Node* old = head_;
            head_ = head_->next;
            if (head_ == nullptr) {
                tail_ = nullptr;
            }
            delete old;
        }

        return total_used;
    }

    void clear() {
        Node* cur = head_;
        while (cur != nullptr) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
        total_bytes_ = 0;
    }

private:
    Node* head_;
    Node* tail_;
    std::size_t total_bytes_;
};

}  // namespace VolcengineTos
