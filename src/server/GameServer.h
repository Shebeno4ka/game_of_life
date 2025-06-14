#pragma once

#include <core/GameSimulator.h>
#include <network/WebSocketServer.h>
#include <atomic>
#include <utils/MPSCQueue.h>
#include <thread>
#include <chrono>
#include <vector>

namespace LifeGame {

    struct CellChange {
        uint32_t x, y;
        bool alive;
    };

/**
 * Событие от клиента - изменение набора клеток
 */
struct GameEvent {
    std::vector<CellChange> changes;
    
    GameEvent(std::vector<CellChange> cellChanges)
        :  changes(std::move(cellChanges)) {}

    GameEvent() = default;

    GameEvent(GameEvent &&) = default;
    GameEvent &operator=(GameEvent &&) = default;

    void Run(GameSimulator& simulator) {
        for (const auto& change : changes) {
            simulator.applySingleCellChange(change.x, change.y, change.alive);
        }
    }
};

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
    explicit GameServer(uint16_t port = 8080, std::chrono::milliseconds stepIntervalMs = 50ms);
    ~GameServer();

    // Управление сервером
    void start();
    void stop();
    bool isRunning() const { return running_; }

    // Настройки
    void setStepInterval(uint32_t intervalMs) { stepIntervalMs_ = intervalMs; }
    void setInitialPattern(BitField pattern);
    void setSendTimeout(uint32_t timeoutMs) { sendTimeoutMs_ = timeoutMs; }

private:
    // Основные компоненты
    boost::asio::io_context ioContext_;
    GameSimulator simulator_;
    network::WebSocketServer webSocketServer_;
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
    std::vector<CellChange> parseClientMessage(std::vector<std::byte> message) const;
};

} // namespace LifeGame
