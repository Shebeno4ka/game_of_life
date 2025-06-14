#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>

namespace LifeGame {

/**
 * Многопоточная очередь MPSC (Multi Producer Single Consumer)
 */
template <typename T>
class MPSCQueue {
   public:
    MPSCQueue() : closed_(false) {}

    ~MPSCQueue() {
        close();
    }

    void push(T item) {
        std::lock_guard lock(mutex_);
        if (!closed_.load()) {
            queue_.push(std::move(item));
            condition_.notify_one();
        }
    }

    bool tryPop(T& item) {
        std::unique_lock lock(mutex_);

        if (queue_.empty()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool popBlocking(T& item) {
        std::unique_lock lock(mutex_);

        while (queue_.empty() && !closed_.load()) {
            condition_.wait(lock);
        }

        if (queue_.empty() && closed_.load()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void close() {
        std::lock_guard lock(mutex_);
        closed_.store(true);
        condition_.notify_all();
    }

    bool isClosed() const {
        return closed_.load();
    }

   private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<T> queue_;
    std::atomic<bool> closed_;
};

} // namespace LifeGame
