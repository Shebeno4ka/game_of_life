#pragma once

#include "server/GameServer.h"
#include "core/GameEvent.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/awaitable.hpp>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <optional>
#include <spdlog/spdlog.h>

namespace network {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace asio = boost::asio;
using namespace std::chrono_literals;

/**
 * Класс WebSocketServer реализует многопользовательский WebSocket-сервер.
 *
 * Методы:
 * - setMessageCallback: устанавливает обработчик входящих сообщений
 * - start / stop: запускает и останавливает сервер
 * - sendToAllClients: отправляет сообщение с таймаутом всем активным клиентам.
 *    Возвращает std::future<void>, на котором можно дождаться, когда каждое сообщение
 *    будет либо успешно доставлено, либо по нему сработает таймаут.
*/
class WebSocketServer {
    using MessageCallback = std::function<void(std::vector<CellChange>)>;
    using Connection = websocket::stream<tcp::socket>;
    using ConnectionId = uint64_t;

    asio::io_context ioContext_;
    std::thread ioThread_;  // Thread running ioContext_.run()
    tcp::acceptor acceptor_;
    ConnectionId nextConnectionId_;
    std::unordered_map<ConnectionId, Connection&> connections_; // from id to connection
    std::mutex mutex_;
    std::optional<MessageCallback> messageCallback_;
    std::shared_ptr<spdlog::logger> logger_;

   public:
    WebSocketServer(const std::string& ip, uint16_t port);

    // NetworkDriver concept
    void setMessageCallback(MessageCallback cb);
    void start();
    void stop();
    std::future<void> sendToAllClients(std::vector<std::byte> data, std::chrono::milliseconds timeout);

   private:
    void acceptLoop();

    asio::awaitable<void> handleSession(Connection ws);

    asio::awaitable<void> sendMessage(ConnectionId connectionId, Connection& ws, std::vector<std::byte>& data,
                                      std::chrono::milliseconds timeout);

    std::vector<CellChange> parseClientMessage_(std::vector<std::byte> message);
};

} // namespace network