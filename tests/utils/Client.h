#pragma once
#include <functional>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <memory>
#include <spdlog/spdlog.h>

class Client {
public:
    using Loger = std::shared_ptr<spdlog::logger>;
    using OnMessageCallback = std::function<void(Loger, std::vector<std::byte>)>;

    explicit Client(boost::asio::io_context& ioContext);
    ~Client();

    void connect(std::string address);
    void disconnect();
    void send(std::vector<std::pair<uint32_t, uint32_t>> changes);
    void setCallback(OnMessageCallback cb);

private:
    void doRead();

    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    Loger logger_;
    OnMessageCallback callback_;
    boost::beast::flat_buffer buffer_;
};