#pragma once
#include <future>
#include <boost/asio.hpp>
#include <linux/futex.h>

// это костыль пиздец
// UPD: даже хуже чем я думал
template <typename T>
std::future<void> runAll(boost::asio::io_context& ioc, std::vector<boost::asio::awaitable<void>> tasks,
                         std::unique_ptr<T> resources) {
    if (tasks.empty()) {
        return std::async(std::launch::deferred, [] {});
    }

    namespace asio = boost::asio;

    struct Control {
        std::unique_ptr<T> resources;
        std::promise<void> promise;
        std::atomic<int> working;
    };

    auto control = new Control{std::move(resources)};
    control->working.store(tasks.size());
    std::future<void> future = control->promise.get_future();

    for (auto& task : tasks) {
        asio::co_spawn(
            ioc,
            [t = std::move(task), control]() mutable -> asio::awaitable<void> {
                co_await std::move(t);
                if (control->working.fetch_add(-1) == 1) {
                    control->promise.set_value();
                    delete control;
                }
                co_return;
            },
            asio::detached);
    }

    return future;
}

inline int futexWait(std::atomic<uint32_t>& value, int expected, const struct timespec* timeout = nullptr) {
    return syscall(SYS_futex, reinterpret_cast<int*>(&value), FUTEX_WAIT, expected, timeout, nullptr, 0);
}

inline int futexWakeOne(std::atomic<uint32_t>& value) {
    return syscall(SYS_futex, reinterpret_cast<int*>(&value), FUTEX_WAKE, 1, nullptr, nullptr, 0);
}

inline int futexWakeAll(std::atomic<uint32_t>& value) {
    return syscall(SYS_futex, reinterpret_cast<int*>(&value), FUTEX_WAKE, INT_MAX, nullptr, nullptr, 0);
}