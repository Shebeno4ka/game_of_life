#pragma once

#include <vector>

#include "BitField.h"

namespace LifeGame {

/**
 * Используется для симуляции игры "Жизнь"
 */
class GameSimulator {
    BitField currentField_;
    BitField nextField_;

   public:
    explicit GameSimulator(uint32_t fieldWidth = FIELD_WIDTH, uint32_t fieldHeight = FIELD_HEIGHT);

    // Основной метод симуляции - выполняет один шаг
    void step();

    // Применение изменения от клиента
    void applySingleCellChange(uint32_t x, uint32_t y, bool alive);

    // Получение состояния для сети(копирует поле)
    std::vector<std::byte> getSerializedField() const;

    uint32_t getAliveCellCount() const;  // для тестов
    void setInitialPattern(BitField pattern);
    void clearField();
   private:
    void performSimulationStep();

    bool shouldCellLive(uint32_t x, uint32_t y, const BitField& field) const;
};

} // namespace LifeGame
