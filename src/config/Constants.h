#pragma once

#include <chrono>

namespace LifeGame {

// Размер игрового поля (фиксированный)
constexpr uint32_t FIELD_WIDTH = 10;
constexpr uint32_t FIELD_HEIGHT = 10;
constexpr uint32_t FIELD_SIZE = FIELD_WIDTH * FIELD_HEIGHT;

// Размер в байтах для битового поля
constexpr uint32_t FIELD_BYTES = (FIELD_SIZE + 7) / 8;

// Параметры многопоточности
constexpr uint32_t DEFAULT_THREAD_COUNT = 8;
constexpr uint32_t SECTOR_SIZE = 128; // Размер сектора для параллельной обработки

// Сетевые параметры
using namespace std::chrono;
constexpr milliseconds NETWORK_UPDATE_INTERVAL_MS = 1000ms; // Интервал отправки обновлений

// Симуляция
constexpr uint64_t SIMULATION_STEP_MS = 500; // Шаг симуляции (20 FPS)

// Размер сообщения от клиента
constexpr int MESSAGE_BYTES_SIZE = sizeof(int32_t) + sizeof(int32_t);

// Типы данных
using Generation = uint64_t;
using CellIndex = uint32_t;

} // namespace LifeGame
