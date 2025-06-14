#pragma once

#include <core/GameSimulator.h>
#include <network/WebSocketServer.h>
#include <utils/MPSCQueue.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "core/GameEvent.h"

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
class GameServer {
   public:
    explicit GameServer(std::unique_ptr<network::WebSocketServer> ws_server,
                        std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS,
                        std::chrono::milliseconds stepIntervalMs = SIMULATION_STEP_MS);
    ~GameServer();

    // Управление сервером
    void start();
    void stop();
    bool isRunning() const {
        return running_;
    }

    // Настройки
    void setStepInterval(std::chrono::milliseconds intervalMs) {
        stepIntervalMs_ = intervalMs;
    }
    void setInitialPattern(BitField pattern);
    void setSendTimeout(std::chrono::milliseconds timeoutMs) {
        sendTimeoutMs_ = timeoutMs;
    }

   private:
    // Основные компоненты
    GameSimulator simulator_;
    std::unique_ptr<network::WebSocketServer> webSocketServer_;
    MPSCQueue<GameEvent> events_;

    // Настройки
    std::chrono::milliseconds stepIntervalMs_;
    std::chrono::milliseconds sendTimeoutMs_;
    std::atomic<bool> running_;

    // Главный игровой поток
    std::thread gameThread_;

    void gameLoop();

    // Обработка WebSocket событий
    void onClientMessage(std::vector<std::byte> data);

    // Парсинг сообщения клиента в события
    static std::vector<CellChange> parseClientMessage(std::vector<std::byte> message);
};

} // namespace LifeGame
