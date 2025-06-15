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
    webSocketServer_->setMessageCallback([this](std::vector<CellChange> data) { onClientMessage(std::move(data)); });
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
    gameMainThread_ = std::thread(&GameServer::gameLoop, this);
}

void GameServer::stop() {
    if (!running_) {
        return;
    }
    running_.store(false);
    events_.close();
    webSocketServer_->stop();

    if (gameMainThread_.joinable()) {
        gameMainThread_.join();
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

void GameServer::onClientMessage(std::vector<CellChange>&& changes) {
    GameEvent event(std::move(changes));
    events_.push(std::move(event));
}

} // namespace LifeGame
