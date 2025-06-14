#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

namespace network
{

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace asio = boost::asio;
using namespace std::chrono_literals;

class WebSocketServer
{
  public:
    using MessageCallback = std::function<void(std::vector<std::byte>)>;
    using Connection = websocket::stream<tcp::socket>;
    using ConnectionId = uint64_t;

    explicit WebSocketServer(asio::io_context &ioContext, uint16_t port = 8080, const std::string &address = "127.0.0.1");

    void setMessageCallback(MessageCallback cb);

    void start();

    void stop();

    std::future<void> sendToAllClients(std::vector<std::byte> data, std::chrono::milliseconds timeout);

  private:
    void acceptLoop();

    asio::awaitable<void> handleSession(Connection ws);

    asio::awaitable<void> sendMessage(ConnectionId connectionId, Connection &ws, std::vector<std::byte> &data,
                                      std::chrono::milliseconds timeout);

    asio::io_context &ioContext_;
    tcp::acceptor acceptor_;
    ConnectionId nextConnectionId_;
    std::unordered_map<ConnectionId, Connection &> connections_; // from id to connection
    std::mutex mutex_;
    std::optional<MessageCallback> messageCallback_;
};

} // namespace network