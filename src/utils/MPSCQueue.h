#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>
#include <optional>

namespace LifeGame {

// сейчас в реализации это MPMCQueue, но в коде этого не требуется, поэтому в дальнейшем можно заменить на более эфективную реализацию
/**
 * Многопоточная очередь MPSC (Multi Producer Single Consumer)
 */
template <typename T>
class MPSCQueue {
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<T> queue_;
    std::atomic<bool> closed_;

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

    void waitWhileEmpty() {
        std::unique_lock lock(mutex_);
        while (queue_.empty() && !closed_.load()) {
            condition_.wait(lock);
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

    std::optional<T> tryPop() {
        std::unique_lock lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop();
        return std::optional<T>(std::move(item));
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
};

} // namespace LifeGame
