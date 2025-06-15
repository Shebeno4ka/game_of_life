#pragma once

#include <core/GameSimulator.h>
#include <network/WebSocketServer.h>
#include <utils/MPSCQueue.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>

#include "core/GameEvent.h"

namespace network {
class WebSocketServer;
}

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
    GameSimulator simulator_;
    std::unique_ptr<network::WebSocketServer> webSocketServer_;
    MPSCQueue<GameEvent> events_;
    std::chrono::milliseconds stepIntervalMs_;
    std::chrono::milliseconds sendTimeoutMs_;
    std::atomic<bool> running_;
    std::thread gameMainThread_;

   public:
    explicit GameServer(std::unique_ptr<network::WebSocketServer> ws_server,
                        std::chrono::milliseconds sendTimeoutMs = NETWORK_UPDATE_INTERVAL_MS,
                        std::chrono::milliseconds stepIntervalMs = SIMULATION_STEP_MS);
    ~GameServer();

    void start();

    void stop();

    bool isRunning() const {
        return running_;
    }

    void setStepInterval(std::chrono::milliseconds intervalMs) {
        stepIntervalMs_ = intervalMs;
    }

    void setInitialPattern(BitField pattern);

    void setSendTimeout(std::chrono::milliseconds timeoutMs) {
        sendTimeoutMs_ = timeoutMs;
    }

   private:
    void gameLoop();

    void onClientMessage(std::vector<CellChange>&& event);
};

} // namespace LifeGame
