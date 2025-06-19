#pragma once

#include <core/GameSimulator.h>
#include <utils/MPSCQueue.h>
#include "server/DriverConcepts.h"
#include "core/BitField.h"
#include "core/GameEvent.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>
#include <spdlog/spdlog.h>

namespace LifeGame {

/**
 * Основной игровой сервер согласно заданному алгоритму:
 *
 * timer = SetTimer(n ms)
 * game_simulator.Step()
 * field = game_simulator().copy()
 * future = web_socket.SendAllAsync(field)
 *
 * while (timer):
 *    event = events.TryPop();
 *    event.Run()
 * future.Wait()
 */
using namespace std::chrono_literals;

template<NetworkDriver Driver>
class GameServer {
    std::unique_ptr<GameSimulator> simulator_;
    std::shared_ptr<spdlog::logger> logger_;
    std::unique_ptr<Driver> networkDriver_;
    MPSCQueue<GameEvent> events_;
    std::chrono::milliseconds stepIntervalMs_;
    std::chrono::milliseconds sendTimeoutMs_;
    std::atomic<bool> running_;
    std::thread gameMainThread_;

   public:
    GameServer(std::unique_ptr<Driver> driver, std::unique_ptr<GameSimulator> simulator,
               std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS,
               std::chrono::milliseconds stepIntervalMs = SIMULATION_STEP_MS)
        : networkDriver_(std::move(driver)),
          simulator_(std::move(simulator)),
          stepIntervalMs_(stepIntervalMs),
          sendTimeoutMs_(sendTimeoutMs),
          running_(false) {
        logger_ = spdlog::get("GameServer");
        if (!logger_) {
            logger_ = spdlog::default_logger();
        }

        networkDriver_->setMessageCallback([this](std::vector<CellChange> data) { onClientMessage(std::move(data)); });
        logger_->info("GameServer initialized with step interval {}ms, send timeout {}ms", stepIntervalMs_.count(),
                      sendTimeoutMs_.count());
    }

    ~GameServer() {
        stop();
    }

    void start() {
        if (running_) {
            logger_->warn("Attempted to start already running server");
            return;
        }
        running_ = true;
        logger_->info("Starting GameServer");

        try {
            networkDriver_->start();
            gameMainThread_ = std::thread(&GameServer::gameLoop, this);
            logger_->info("GameServer started successfully");
        } catch (const std::exception& e) {
            logger_->error("Failed to start GameServer: {}", e.what());
            running_ = false;
            throw;
        }
    }

    void stop() {
        if (!running_) {
            return;
        }
        logger_->info("Stopping GameServer");

        running_.store(false);
        events_.close();
        networkDriver_->stop();

        if (gameMainThread_.joinable()) {
            gameMainThread_.join();
        }
        logger_->info("GameServer stopped");
    }

    bool isRunning() const {
        return running_;
    }

    void setStepInterval(std::chrono::milliseconds intervalMs) {
        stepIntervalMs_ = intervalMs;
    }

    void setInitialPattern(BitField pattern) {
        simulator_->setInitialPattern(std::move(pattern));
        logger_->debug("Initial pattern set");
    }

    void setSendTimeout(std::chrono::milliseconds timeoutMs) {
        sendTimeoutMs_ = timeoutMs;
    }

   private:
    void gameLoop() {
        logger_->info("Game loop started");
        size_t stepCount = 0;

        while (running_) {
            auto stepStartTime = std::chrono::steady_clock::now();
            simulator_->step();
            stepCount++;

            auto currentState = simulator_->getStateData();
            auto sendFuture = networkDriver_->sendToAllClients(std::move(currentState), sendTimeoutMs_);

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

    void onClientMessage(std::vector<CellChange> changes) {
        logger_->debug("Processed client message with {} changes", changes.size());
        GameEvent event(std::move(changes));
        events_.push(std::move(event));
    }
};

} // namespace LifeGame
