#include "WebSocketServer.h"
#include <utils/AsyncUtils.h>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include "server/GameServer.h"

using namespace network;

WebSocketServer::WebSocketServer(const std::string& ip, uint16_t port)
    : acceptor_(ioContext_, tcp::endpoint(boost::asio::ip::make_address(ip), port)), nextConnectionId_(0) {
    logger_ = spdlog::get("WebSocketServer");
    if (!logger_) {
        logger_ = spdlog::default_logger();
    }
    logger_->info("WebSocketServer created on {}:{}", ip, port);
}

void WebSocketServer::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void WebSocketServer::start() {
    logger_->info("Starting WebSocket server");
    acceptLoop();
    ioThread_ = std::thread([this]() { ioContext_.run(); });
    logger_->info("WebSocket server started");
}

void WebSocketServer::stop() {
    logger_->info("Stopping WebSocket server");
    ioContext_.stop();
    std::scoped_lock lock(mutex_);
    size_t connectionCount = connections_.size();
    for (auto& [id, ws] : connections_) {
        boost::system::error_code ignored;
        ws.next_layer().cancel(ignored);
    }
    connections_.clear();
    ioThread_.join();
    logger_->info("WebSocket server stopped, closed {} connections", connectionCount);
}

asio::awaitable<void> WebSocketServer::sendMessage(ConnectionId connectionId, Connection& ws,
                                                   std::vector<std::byte>& data, std::chrono::milliseconds timeout) {
    auto timer = asio::steady_timer(co_await asio::this_coro::executor);
    boost::system::error_code ecWrite, ecTimer;
    std::atomic<bool> writeDone = false;

    timer.expires_after(timeout);

    asio::co_spawn(
        ioContext_,
        [&ws, &data, &writeDone, &timer, &ecWrite]() -> asio::awaitable<void> {
            co_await ws.async_write(asio::buffer(data), asio::redirect_error(asio::use_awaitable, ecWrite));
            writeDone.store(true);
            timer.cancel();
            co_return;
        },
        asio::detached);

    co_await timer.async_wait(asio::redirect_error(asio::use_awaitable, ecTimer));

    if (!writeDone.load()) {
        logger_->warn("Send timeout for connection {}", connectionId);
        boost::system::error_code ignored;
        ws.next_layer().cancel(ignored);
        std::scoped_lock lock(mutex_);
        connections_.erase(connectionId);
        co_return;
    }

    if (ecWrite) {
        logger_->debug("Send failed for connection {}: {}", connectionId, ecWrite.message());
        std::scoped_lock lock(mutex_);
        connections_.erase(connectionId);
    }
}

std::future<void> WebSocketServer::sendToAllClients(std::vector<std::byte> data, std::chrono::milliseconds timeout) {
    size_t clientCount;
    {
        std::scoped_lock lock(mutex_);
        clientCount = connections_.size();
    }
    
    if (clientCount > 0) {
        logger_->debug("Broadcasting {} bytes to {} clients", data.size(), clientCount);
    }

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
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto endpoint = socket.remote_endpoint();
            logger_->info("New connection from {}:{}", endpoint.address().to_string(), endpoint.port());
            auto ws = Connection(std::move(socket));
            asio::co_spawn(ioContext_, handleSession(std::move(ws)), asio::detached);
        } else {
            logger_->error("Accept failed: {}", ec.message());
        }
        acceptLoop();
    });
}

asio::awaitable<void> WebSocketServer::handleSession(Connection ws) {
    ws.binary(true);
    boost::system::error_code acceptEc;
    co_await ws.async_accept(asio::redirect_error(asio::use_awaitable, acceptEc));
    
    if (acceptEc) {
        logger_->warn("WebSocket handshake failed: {}", acceptEc.message());
        co_return;
    }

    ConnectionId connectionId;
    {
        std::scoped_lock lock(mutex_);
        connectionId = nextConnectionId_++;
        connections_.emplace(connectionId, ws);
        logger_->info("Client {} connected, total connections: {}", connectionId, connections_.size());
    }

    for (;;) {
        boost::beast::flat_buffer buffer;
        boost::system::error_code ec;
        co_await ws.async_read(buffer, asio::redirect_error(asio::use_awaitable, ec));
        if (ec) {
            if (ec != boost::beast::websocket::error::closed) {
                logger_->debug("Read error from client {}: {}", connectionId, ec.message());
            }
            break;
        }

        auto ptr = static_cast<std::byte*>(buffer.data().data());
        std::vector<std::byte> data(ptr, ptr + buffer.size());
        logger_->debug("Received {} bytes from client {}", data.size(), connectionId);

        std::vector<CellChange> parsed_events = parseClientMessage_(std::move(data));

        if (messageCallback_.has_value()) {
            (*messageCallback_)(std::move(parsed_events));
        }
    }

    {
        std::scoped_lock lock(mutex_);
        connections_.erase(connectionId);
        logger_->info("Client {} disconnected, remaining connections: {}", connectionId, connections_.size());
    }
}

std::vector<CellChange> WebSocketServer::parseClientMessage_(std::vector<std::byte> message) {
    if (message.size() % LifeGame::MESSAGE_BYTES_SIZE != 0 || message.empty()) {
        return {};
    }

    std::vector<CellChange> changes;
    changes.reserve(message.size() / LifeGame::MESSAGE_BYTES_SIZE);

    for (size_t i = 0; i < message.size(); i += LifeGame::MESSAGE_BYTES_SIZE) {
        uint32_t x = static_cast<uint32_t>(message[i]);
        uint32_t y = static_cast<uint32_t>(message[i + 1]);

        changes.emplace_back(x, y, true);
    }

    return changes;
}