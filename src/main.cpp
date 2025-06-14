#include "server/GameServer.h"
#include "network/WebSocketServer.h"
#include <chrono>

#include <iostream>

int main(int argc, char* argv[]) {
    boost::asio::ip::address ip = boost::asio::ip::address::from_string("0.0.0.0");
    network::WebSocketServer ws(ip, 8080);
    auto callback = [](std::vector<std::byte> data) { std::cout << "I am get message"; };
    ws.setMessageCallback(std::move(callback));
    std::string str;
    ws.start();
    while (str != "q") {
        std::cout << "Enter text\n";
        std::cin >> str;
        std::vector<std::byte> bytes2(str.size());
        std::transform(str.begin(), str.end(), bytes2.begin(), [](char c) { return static_cast<std::byte>(c); });
        ws.sendToAllClients(std::move(bytes2), std::chrono::milliseconds(1000));
    }
    ws.stop();

    return 0;
}
