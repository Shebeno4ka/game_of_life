#include "server/GameServer.h"
#include "network/WebSocketServer.h"
#include <chrono>

#include <iostream>

int main(int argc, char* argv[]) {
    std::string ip = "0.0.0.0";
    int port = 8080;

    if (argc != 3 && argc != 1) {
        std::cerr << "Usage: " << argv[0] << " [ip] [port]\n";
        std::cerr << "Default: " << ip << ":" << port << "\n";
        return 1;
    }

    if (argc == 3) {
        ip = argv[1];
        port = std::stoi(argv[2]);
    }

    std::cout << "Starting game server..." << std::endl;
    auto ws_server_ptr = std::make_unique<network::WebSocketServer>(ip, port);
    auto gameSimulator = std::make_unique<LifeGame::GameSimulator>();
    LifeGame::GameServer game_server(std::move(ws_server_ptr), std::move(gameSimulator));

    game_server.start();

    std::cout << "Game server is up!" << std::endl;
    while (true) {
    }

    return 0;
}
