#include "server/GameServer.h"
#include "core/BitField.h"
#include "core/GameEvent.h"
#include <chrono>

namespace LifeGame {

GameServer::GameServer(std::unique_ptr<network::WebSocketServer> ws_server, std::unique_ptr<GameSimulator> simulator,
                       std::chrono::milliseconds sendTimeoutMs, std::chrono::milliseconds stepIntervalMs)
    : webSocketServer_(std::move(ws_server)),
      simulator_(std::move(simulator)),
      stepIntervalMs_(stepIntervalMs),
      sendTimeoutMs_(sendTimeoutMs),
      running_(false) {
    logger_ = spdlog::get("GameServer");
    if (!logger_) {
        logger_ = spdlog::default_logger();
    }

    webSocketServer_->setMessageCallback([this](std::vector<CellChange> data) { onClientMessage(std::move(data)); });
    logger_->info("GameServer initialized with step interval {}ms, send timeout {}ms", stepIntervalMs_.count(),
                  sendTimeoutMs_.count());
}

GameServer::~GameServer() {
    stop();
}

void GameServer::start() {
    if (running_) {
        logger_->warn("Attempted to start already running server");
        return;
    }
    running_ = true;
    logger_->info("Starting GameServer");

    try {
        webSocketServer_->start();
        gameMainThread_ = std::thread(&GameServer::gameLoop, this);
        logger_->info("GameServer started successfully");
    } catch (const std::exception& e) {
        logger_->error("Failed to start GameServer: {}", e.what());
        running_ = false;
        throw;
    }
}

void GameServer::stop() {
    if (!running_) {
        return;
    }
    logger_->info("Stopping GameServer");

    running_.store(false);
    events_.close();
    webSocketServer_->stop();

    if (gameMainThread_.joinable()) {
        gameMainThread_.join();
    }
    logger_->info("GameServer stopped");
}

void GameServer::setInitialPattern(BitField pattern) {
    simulator_->setInitialPattern(std::move(pattern));
    logger_->debug("Initial pattern set");
}

void GameServer::gameLoop() {
    logger_->info("Game loop started");
    size_t stepCount = 0;

    while (running_) {
        auto stepStartTime = std::chrono::steady_clock::now();
        simulator_->step();
        stepCount++;

        auto currentState = simulator_->getStateData();
        auto sendFuture = webSocketServer_->sendToAllClients(std::move(currentState), sendTimeoutMs_);

        size_t eventsProcessed = 0;
        auto stepEndTime = stepStartTime + std::chrono::milliseconds(stepIntervalMs_);
        while (std::chrono::steady_clock::now() < stepEndTime) {
            GameEvent event;
            if (events_.tryPop(event)) {
                event.Run(*simulator_);
                eventsProcessed++;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        try {
            sendFuture.get();
        } catch (const std::exception& e) {
            logger_->error("Failed to send game state: {}", e.what());
        }

        if (stepCount % 100 == 0) {
            logger_->debug("Step {}, processed {} events", stepCount, eventsProcessed);
        }
    }
    logger_->info("Game loop finished after {} steps", stepCount);
}

void GameServer::onClientMessage(std::vector<CellChange> changes) {
    logger_->debug("Processed client message with {} changes", changes.size());
    GameEvent event(std::move(changes));
    events_.push(std::move(event));
}

} // namespace LifeGame
