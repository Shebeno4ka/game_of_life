#include "server/GameServer.h"
#include "network/WebSocketServer.h"
#include "utils/LoggerSetup.h"
#include "core/GameSimulator.h"
#include "server/FixedStepStrategy.h"
#include "../tests/utils/Client.h"
#include "../tests/utils/SimulationPatterns.h"
#include "utils/DebugUtils.h"

#include <chrono>
#include <iostream>
#include <boost/asio.hpp>
#include <atomic>

int main(int argc, char* argv[]) {
    utils::setupLogging();

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

    boost::asio::io_context ioContext;

    // Create and configure the GameServer
    auto ws_server_ptr = std::make_unique<network::WebSocketServer>(ip, port);
    auto gameSimulator = std::make_unique<LifeGame::GameSimulator>();
    auto stepStrategy = std::make_unique<LifeGame::FixedStepStrategy<1000>>();
    LifeGame::BitField startField(10, 10);
    LifeGame::GameServer gameServer(std::move(ws_server_ptr), std::move(gameSimulator), std::move(stepStrategy));
    LifeGame::utils::applyPattern(startField, LifeGame::utils::Patterns::block(0, 0));
    LifeGame::utils::applyPattern(startField, LifeGame::utils::Patterns::blinker(5, 5));
    gameServer.setInitialPattern(std::move(startField));

    gameServer.start();

    // Create and configure the client
    Client client(ioContext);
    std::atomic<int> counter{0};
    client.setOnServerMessageCallback([&counter](Client::Loger logger, std::vector<std::byte> data) {
        counter.fetch_add(1);
        if (counter.load() % 15 == 0) {
            logger->info("Received {} response: \n{}", counter.load(), stringField(data));
        }
    });

    try {
        client.connect(ip + ":" + std::to_string(port));
    } catch (const std::exception& e) {
        std::cerr << "Failed to connect to server: " << e.what() << "\n";
        return 1;
    }

    // Run the io_context in the main thread
    std::thread thread([&ioContext]() {ioContext.run();});

    gameServer.waitUntilStopped();

    gameServer.stop();

    return 0;
}
