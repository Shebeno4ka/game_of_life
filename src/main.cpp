#include "server/GameServer.h"
#include "network/WebSocketServer.h"
#include "config/LoggerSetup.h"
#include "core/GameSimulator.h"
#include "server/FixedStepStrategy.h"
#include "client/Client.h"
// #include "../tests/utils/SimulationPatterns.h"
#include "utils/DebugUtils.h"

#include <chrono>
#include <iostream>
#include <boost/asio.hpp>
#include <atomic>


void startStandardServer(uint32_t fieldSize) {
    std::string ip = "0.0.0.0";
    int port = 8080;
    auto ws_server_ptr = std::make_unique<network::WebSocketServer>(ip, port);
    auto gameSimulator = std::make_unique<LifeGame::GameSimulator>(fieldSize, fieldSize);
    auto stepStrategy = std::make_unique<LifeGame::FixedStepStrategy<500>>();
    LifeGame::GameServer gameServer(std::move(ws_server_ptr), std::move(gameSimulator), std::move(stepStrategy));

    gameServer.start();
    gameServer.waitUntilStopped();

}

int main(int argc, char* argv[]) {
    // startStandardServer(10);
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
    // LifeGame::utils::applyPattern(startField, LifeGame::utils::Patterns::block(0, 0));
    // LifeGame::utils::applyPattern(startField, LifeGame::utils::Patterns::blinker(5, 5));
    // gameServer.setInitialPattern(std::move(startField));

    gameServer.start();

    auto logger = spdlog::default_logger();


    // Create and configure the client
    Client client(ioContext);
    std::atomic<int> counter{0};
    client.setOnServerMessageCallback([&counter, &logger](std::vector<std::byte> data) {
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

    std::thread thread([&ioContext]() { ioContext.run(); });

    sleep(3);
    client.send({{0, 0}, {1, 1}, {1, 0}, {0, 1}});

    // Run the io_context in the main thread


    gameServer.waitUntilStopped();

    gameServer.stop();

    return 0;
}
