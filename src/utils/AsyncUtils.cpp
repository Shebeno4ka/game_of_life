#include "AsyncUtils.h"

namespace asio = boost::asio;

std::future<void> runAll(asio::io_context &ioc, std::vector<asio::awaitable<void>> tasks) {
    struct Control {
        std::promise<void> promise;
        std::atomic<int> working;
    };

    auto control = new Control;
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

