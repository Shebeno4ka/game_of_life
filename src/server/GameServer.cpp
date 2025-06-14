#include "server/GameServer.h"
#include "core/BitField.h"
#include <chrono>

namespace LifeGame {

GameServer::GameServer(std::unique_ptr<network::WebSocketServer> ws_server, std::chrono::milliseconds sendTimeoutMs,
                       std::chrono::milliseconds stepIntervalMs)
    : webSocketServer_(std::move(ws_server)),
      stepIntervalMs_(stepIntervalMs),
      sendTimeoutMs_(sendTimeoutMs),
      running_(false) {
    webSocketServer_->setMessageCallback([this](std::vector<std::byte> data) { onClientMessage(std::move(data)); });
}

GameServer::~GameServer() {
    stop();
}

void GameServer::start() {
    if (running_) {
        return;
    }
    running_ = true;
    webSocketServer_->start();
    gameThread_ = std::thread(&GameServer::gameLoop, this);
}

void GameServer::stop() {
    if (!running_) {
        return;
    }
    running_.store(false);
    events_.close();
    webSocketServer_->stop();

    if (gameThread_.joinable()) {
        gameThread_.join();
    }
}

void GameServer::setInitialPattern(BitField pattern) {
    simulator_.setInitialPattern(std::move(pattern));
}

void GameServer::gameLoop() {
    while (running_) {
        auto stepStartTime = std::chrono::steady_clock::now();
        simulator_.step();

        auto currentState = simulator_.getStateData();
        auto sendFuture = webSocketServer_->sendToAllClients(std::move(currentState), sendTimeoutMs_);

        auto stepEndTime = stepStartTime + std::chrono::milliseconds(stepIntervalMs_);
        while (std::chrono::steady_clock::now() < stepEndTime) {
            GameEvent event;
            if (events_.tryPop(event)) {
                event.Run(simulator_);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        sendFuture.get();
    }
}

void GameServer::onClientMessage(std::vector<std::byte> data) {
    auto changes = parseClientMessage(std::move(data));
    GameEvent event(std::move(changes));
    events_.push(std::move(event));
}

std::vector<CellChange> GameServer::parseClientMessage(std::vector<std::byte> message) {
    if (message.size() % MESSAGE_BYTES_SIZE != 0 || message.empty()) {
        return {};
    }

    std::vector<CellChange> changes;
    changes.reserve(message.size() / MESSAGE_BYTES_SIZE);

    for (size_t i = 0; i < message.size(); i += MESSAGE_BYTES_SIZE) {
        uint32_t x = static_cast<uint32_t>(message[i]);
        uint32_t y = static_cast<uint32_t>(message[i + 1]);

        changes.emplace_back(x, y, true);
    }

    return changes;
}

} // namespace LifeGame
