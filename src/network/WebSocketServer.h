#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/awaitable.hpp>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <optional>

namespace network {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace asio = boost::asio;
using namespace std::chrono_literals;

class WebSocketServer {
   public:
    using MessageCallback = std::function<void(std::vector<std::byte>)>;
    using Connection = websocket::stream<tcp::socket>;
    using ConnectionId = uint64_t;

    WebSocketServer(asio::ip::address ip, uint16_t port);

    void setMessageCallback(MessageCallback cb);

    void start();

    void stop();

    std::future<void> sendToAllClients(std::vector<std::byte> data, std::chrono::milliseconds timeout);

   private:
    void acceptLoop();

    asio::awaitable<void> handleSession(Connection ws);

    asio::awaitable<void> sendMessage(ConnectionId connectionId, Connection& ws, std::vector<std::byte>& data,
                                      std::chrono::milliseconds timeout);

    asio::io_context ioContext_;
    std::thread ioThread_;
    tcp::acceptor acceptor_;
    ConnectionId nextConnectionId_;
    std::unordered_map<ConnectionId, Connection&> connections_; // from id to connection
    std::mutex mutex_;
    std::optional<MessageCallback> messageCallback_;
};

} // namespace network