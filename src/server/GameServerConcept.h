#pragma once

#include "core/BitField.h"

#include <concepts>
#include <chrono>
#include <memory>

namespace LifeGame {

/**
 * Концепт игрового сервера
 *
 * Определяет интерфейс для реализации игрового сервера согласно требованиям:
 * - Управление жизненным циклом (start/stop)
 * - Обработка игровых циклов с фиксированным временным интервалом
 * - Сетевое взаимодействие с клиентами
 * - Обработка событий от клиентов
 * - Синхронизация состояния игры
 */
template <typename T>
concept GameServerConcept = requires(T& server, BitField pattern, std::chrono::milliseconds timeout) {
    // Основные методы управления жизненным циклом
    { server.start() } -> std::same_as<void>;
    { server.stop() } -> std::same_as<void>;
    { server.isRunning() } -> std::same_as<bool>;

    // Конфигурация игрового поля
    { server.setInitialPattern(std::move(pattern)) } -> std::same_as<void>;
};

} // namespace LifeGame