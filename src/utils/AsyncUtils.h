#pragma once
#include <future>
#include <boost/asio.hpp>


// это костыль пиздец
// UPD: даже хуже чем я думал
template <typename T>
std::future<void> runAll(
        boost::asio::io_context &ioc,
        std::vector<boost::asio::awaitable<void>> tasks,
        std::unique_ptr<T> resources) {
    namespace asio = boost::asio;

    struct Control {
        std::unique_ptr<T> resources;
        std::promise<void> promise;
        std::atomic<int> working;
    };

    auto control = new Control{std::move(resources)};
    control->working.store(tasks.size());
    std::future<void> future = control->promise.get_future();

    for (auto& task: tasks) {
        asio::co_spawn(ioc, [t = std::move(task), control]() mutable -> asio::awaitable<void> {
            co_await std::move(t);
            if (control->working.fetch_add(-1) == 1) {
                control->promise.set_value();
                delete control;
            }
            co_return;
        }, asio::detached);
    }

    return future;
}
