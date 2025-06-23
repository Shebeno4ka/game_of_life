#pragma once

#include "clients/IClient.h"

#include <memory>
#include <boost/asio/io_context.hpp>

template <typename T>
concept IClientGenerator = requires(T a, boost::asio::io_context& ioContext) {
    { a.next(ioContext) } -> std::same_as<std::unique_ptr<IClient>>;
};