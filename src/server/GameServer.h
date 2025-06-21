#pragma once

#include "FixedStepStrategy.h"

#include <core/GameSimulator.h>
#include <utils/MPSCQueue.h>
#include "server/DriverConcepts.h"
#include "core/BitField.h"
#include "core/GameEvent.h"
#include "utils/AsyncUtils.h"

#include <atomic>
#include <chrono>
#include <sstream>
#include <thread>
#include <vector>
#include <utility>
#include <spdlog/spdlog.h>

namespace LifeGame {

/**
 * Основной игровой сервер
 */

using namespace std::chrono_literals;

template <NetworkDriver Driver, StepControlStrategy StepStrategy>
class GameServer {
    enum RunState {
        Stoped = 1,
        Running = 2
    };

    std::unique_ptr<GameSimulator> simulator_;
    std::unique_ptr<Driver> networkDriver_;
    std::unique_ptr<StepStrategy> stepStrategy_;
    std::shared_ptr<spdlog::logger> logger_;
    MPSCQueue<GameEvent> events_;
    std::chrono::milliseconds sendTimeoutMs_;
    std::atomic<uint32_t> running_;
    std::thread gameMainThread_;

   public:
    GameServer(std::unique_ptr<Driver> driver, std::unique_ptr<GameSimulator> simulator,
               std::unique_ptr<StepStrategy> stepStrategy,
               std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS)
        : networkDriver_(std::move(driver)),
          simulator_(std::move(simulator)),
          stepStrategy_(std::move(stepStrategy)),
          sendTimeoutMs_(sendTimeoutMs),
          running_(RunState::Stoped) {
        logger_ = spdlog::get("GameServer");
        if (!logger_) {
            logger_ = spdlog::default_logger();
        }

        networkDriver_->setMessageCallback([this](std::vector<CellChange> data) { onClientMessage(std::move(data)); });
        logger_->info("GameServer initialized with send timeout {}ms", sendTimeoutMs_.count());
    }

    GameServer(std::unique_ptr<Driver> driver, std::unique_ptr<GameSimulator> simulator,
               std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS)
        : GameServer(std::move(driver), std::move(simulator), std::make_unique<FixedStepStrategy<SIMULATION_STEP_MS>>(),
                     sendTimeoutMs) {}

    ~GameServer() {
        stop();
    }

    GameServer(const GameServer&) = delete;
    GameServer& operator=(const GameServer&) = delete;
    GameServer(GameServer&&) = delete;
    GameServer& operator=(GameServer&&) = delete;

    void start() {
        if (running_.load() == RunState::Running) {
            logger_->warn("Attempted to start already running server");
            return;
        }
        running_.store(RunState::Running);
        logger_->info("Starting GameServer");

        networkDriver_->start();
        gameMainThread_ = std::thread(&GameServer::gameLoop, this);
        logger_->info("GameServer started successfully");
    }

    void stop() {
        if (running_.load() == RunState::Stoped) {
            return;
        }
        logger_->info("Stopping GameServer");

        running_.store(RunState::Stoped);
        events_.close();
        networkDriver_->stop();
        stepStrategy_->stop();

        if (gameMainThread_.joinable()) {
            gameMainThread_.join();
        }
        futexWakeAll(running_);
        logger_->info("GameServer stopped");
    }

    // wait on Futex
    void waitUntilStopped() {
        while (running_.load() == RunState::Running) {
            futexWait(running_, RunState::Running);
        }
    }

    bool isRunning() const {
        return running_.load() == RunState::Running;
    }

    void setInitialPattern(BitField pattern) {
        simulator_->setInitialPattern(std::move(pattern));
        logger_->debug("Initial pattern set");
    }

    // for testing
    std::vector<std::byte> getField() {
        return simulator_->getSerializedField();
    }

   private:
    /**
     * Основной игровой цикл согласно заданному алгоритму
     *
     * stepStrategy_->onStepStart();
     * game_simulator.Step()
     * field = game_simulator().copy()
     * future = web_socket.SendAllAsync(field)
     *
     * while (!stepStrategy_->isStepComplete()):
     *    event = events.TryPop();
     *    event.Run()
     * future.Wait()
     * stepStrategy_->onStepEnd();
     */
    void gameLoop() {
        logger_->info("Game loop started");
        size_t stepCount = 0;

        while (running_.load() == RunState::Running) {
            stepStrategy_->onStepStart();
            simulator_->step();
            stepCount++;

            auto currentState = simulator_->getSerializedField();
            auto sendFuture = networkDriver_->sendToAllClients(std::move(currentState), sendTimeoutMs_);

            size_t eventsProcessed = 0;
            while (!stepStrategy_->isStepComplete()) {
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

            stepStrategy_->onStepEnd();
        }
        logger_->info("Game loop finished after {} steps", stepCount);
    }

    void onClientMessage(std::vector<CellChange> changes) {
        logger_->debug("Processed client message with {} changes", changes.size());
        GameEvent event(std::move(changes));
        events_.push(std::move(event));
    }
};

// deduction guide
template <NetworkDriver Driver>
GameServer(std::unique_ptr<Driver> driver, std::unique_ptr<GameSimulator> simulator,
           std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS)
    -> GameServer<Driver, FixedStepStrategy<SIMULATION_STEP_MS>>;

} // namespace LifeGame
