#pragma once
#include <future>
#include <boost/asio.hpp>

// Это костыль пиздец
std::future<void> runAll(
    boost::asio::io_context& ioc,
    std::vector<boost::asio::awaitable<void>> tasks);
