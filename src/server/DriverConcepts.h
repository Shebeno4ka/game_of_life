#pragma once

#include <concepts>
#include <future>
#include <vector>
#include <chrono>
#include <functional>
#include "core/GameEvent.h"

namespace LifeGame {

template <typename T>
concept NetworkDriver = requires(T& driver, std::vector<std::byte> data, std::chrono::milliseconds timeout,
                                 std::function<void(std::vector<CellChange>)> callback) {
    // Должен иметь метод для отправки данных всем клиентам
    { driver.sendToAllClients(data, timeout) } -> std::same_as<std::future<void>>;

    // Должен иметь метод для установки callback'а сообщений
    { driver.setMessageCallback(callback) } -> std::same_as<void>;

    // Должен иметь методы start/stop
    { driver.start() } -> std::same_as<void>;
    { driver.stop() } -> std::same_as<void>;
};

template <typename T>
concept StepControlStrategy = requires(T strategy) {
    // Метод для начала шага
    { strategy.onStepStart() } -> std::same_as<void>;

    // Метод для проверки, следует ли продолжать текущий шаг.
    { strategy.isStepComplete() } -> std::same_as<bool>;

    { strategy.onStepEnd() } -> std::same_as<void>;

    { strategy.stop() } -> std::same_as<void>;
};

} // namespace LifeGame
