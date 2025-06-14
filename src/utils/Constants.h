#pragma once

#include <cstdint>

namespace LifeGame {

// Размер игрового поля (фиксированный)
constexpr uint32_t FIELD_WIDTH = 1024;
constexpr uint32_t FIELD_HEIGHT = 1024;
constexpr uint32_t FIELD_SIZE = FIELD_WIDTH * FIELD_HEIGHT;

// Размер в байтах для битового поля
constexpr uint32_t FIELD_BYTES = (FIELD_SIZE + 7) / 8;

// Параметры многопоточности
constexpr uint32_t DEFAULT_THREAD_COUNT = 8;
constexpr uint32_t SECTOR_SIZE = 128; // Размер сектора для параллельной обработки

// Сетевые параметры
constexpr uint32_t NETWORK_UPDATE_INTERVAL_MS = 150; // Интервал отправки обновлений

// Симуляция
constexpr uint32_t SIMULATION_STEP_MS = 50; // Шаг симуляции (20 FPS)

// Размер сообщения от клиента
constexpr int MESSAGE_BYTES_SIZE = sizeof(int32_t) + sizeof(int32_t);

// Типы данных
using Generation = uint64_t;
using CellIndex = uint32_t;

} // namespace LifeGame
