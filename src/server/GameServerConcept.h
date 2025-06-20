#pragma once

#include <concepts>
#include <chrono>
#include <memory>
#include <future>
#include <atomic>
#include "core/GameSimulator.h"
#include "core/BitField.h"
#include "server/DriverConcepts.h"

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

    // Настройка сетевых параметров
    { server.setSendTimeout(timeout) } -> std::same_as<void>;

    // Требования к конструктору и деструктору
    requires std::is_destructible_v<T>;
    requires !std::is_copy_constructible_v<T>;
    requires !std::is_copy_assignable_v<T>;
    requires !std::is_move_constructible_v<T>;
    requires !std::is_move_assignable_v<T>;
};

} // namespace LifeGame