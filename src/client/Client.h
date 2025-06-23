#pragma once
#include <functional>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <spdlog/spdlog.h>

/*
 Класс Client предназначен для тестирования:
 позволяет моделировать реальные подключения к серверу через WebSocket.

 Перед уничтожением должен быть вызван disconnect, и завершён ioContext.run(), иначе UB
*/
class Client {
public:
    using Loger = std::shared_ptr<spdlog::logger>;
    using OnMessageCallback = std::function<void(std::vector<std::byte>)>;

    explicit Client(boost::asio::io_context& ioContext);
    ~Client();

    void connect(std::string address);
    void disconnect();
    void send(std::vector<std::pair<uint32_t, uint32_t>> changes);
    void setOnServerMessageCallback(OnMessageCallback cb);

private:
    using ErrorCode = boost::system::error_code;
    using ResolveResults = boost::asio::ip::tcp::resolver::results_type;

    /*
     * Последовательность инициализации подключения к серверу:
     * Каждый метод запускает асинхронную операцию и в её коллбэке вызывает следующий шаг:
     * 1. handleResolve — асинхронный DNS-резолвинг адреса, вызывает handleConnect
     * 2. handleConnect — устанавливает TCP-соединение, вызывает handleHandshake
     * 3. handleHandshake — выполняет WebSocket-handshake, вызывает doRead
     * 4. doRead — запускает цикл чтения входящих сообщений от сервера
     */
    void handleResolve(std::string address);
    void handleConnect(std::string address, ResolveResults results);
    void handleHandshake(std::string address);
    void doRead();

    boost::asio::io_context& ioContext_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    Loger logger_;
    OnMessageCallback callback_;
    boost::beast::flat_buffer buffer_;
    std::atomic<bool> isClosed_{true};
};