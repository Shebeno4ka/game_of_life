#include "WebSocketServer.h"
#include <utils/AsyncUtils.h>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

using namespace network;

WebSocketServer::WebSocketServer(boost::asio::ip::address ip, uint16_t port)
        : acceptor_(ioContext_, tcp::endpoint(ip, port))
        , nextConnectionId_(0)  {

}

void WebSocketServer::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void WebSocketServer::start() {
    acceptLoop();
    ioThread_ = std::thread([this](){ioContext_.run();});
}

void WebSocketServer::stop() {
    ioContext_.stop();
    std::scoped_lock lock(mutex_);
    for (auto& [id, ws] : connections_) {
        boost::system::error_code ignored;
        ws.next_layer().cancel(ignored);
    }
    connections_.clear();
    ioThread_.join();
}

asio::awaitable<void> WebSocketServer::sendMessage(
        ConnectionId connectionId
        , Connection &ws
        , std::vector<std::byte> &data
        , std::chrono::milliseconds timeout) {
    auto timer = asio::steady_timer(co_await asio::this_coro::executor);
    boost::system::error_code ecWrite, ecTimer;
    std::atomic<bool> writeDone = false;

    timer.expires_after(timeout);

    asio::co_spawn(ioContext_, [&ws, &data, &writeDone, &timer, &ecWrite]() -> asio::awaitable<void> {
        co_await ws.async_write(
            asio::buffer(data),
            asio::redirect_error(asio::use_awaitable, ecWrite));
        writeDone.store(true);
        timer.cancel();
        co_return;
    }, asio::detached);

    co_await timer.async_wait(asio::redirect_error(asio::use_awaitable, ecTimer));

    if (!writeDone.load()) {
        boost::system::error_code ignored;
        ws.next_layer().cancel(ignored);
        std::scoped_lock lock(mutex_);
        connections_.erase(connectionId);
        co_return;
    }

    if (ecWrite) {
        std::scoped_lock lock(mutex_);
        connections_.erase(connectionId);
    }
}


std::future<void> WebSocketServer::sendToAllClients(
        std::vector<std::byte> data
        , std::chrono::milliseconds timeout) {
    std::vector<asio::awaitable<void>> tasks;

    auto sharedResources = std::make_unique<std::vector<std::byte>>(std::move(data));
    {
        std::scoped_lock lock(mutex_);
        for (auto& [id, ws] : connections_) {
            tasks.emplace_back(sendMessage(id, ws, *sharedResources, timeout));
        }
    }

    return runAll(ioContext_, std::move(tasks), std::move(sharedResources));
}

void WebSocketServer::acceptLoop() {
    acceptor_.async_accept(
    [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto ws = Connection(std::move(socket));
            asio::co_spawn(ioContext_, handleSession(std::move(ws)), asio::detached);
        }
        acceptLoop();
    });
}

asio::awaitable<void> WebSocketServer::handleSession(Connection ws) {
    ws.binary(true);
    co_await ws.async_accept(asio::use_awaitable);

    ConnectionId connectionId;
    {
        std::scoped_lock lock(mutex_);
        connectionId = nextConnectionId_++;
        connections_.emplace(connectionId, ws);
    }

    for (;;) {
        boost::beast::flat_buffer buffer;
        boost::system::error_code ec;
        co_await ws.async_read(buffer, asio::redirect_error(asio::use_awaitable, ec));
        if (ec) {
            break;
        }

        auto ptr = static_cast<std::byte*>(buffer.data().data());
        std::vector<std::byte> data(ptr, ptr + buffer.size());

        if (messageCallback_.has_value()) {
            (*messageCallback_)(std::move(data));
        }
    }

    std::scoped_lock lock(mutex_);
    connections_.erase(connectionId);
}
