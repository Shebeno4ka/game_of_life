#pragma once

#include <vector>

#include "BitField.h"

namespace LifeGame {

/**
 * Многопоточный симулятор игры "Жизнь"
 * Чистый симулятор без логики игроков - только симуляция Conway's Game of Life
 */
class GameSimulator {
    BitField currentField_;
    BitField nextField_;

   public:
    explicit GameSimulator(uint32_t fieldWidth = FIELD_WIDTH, uint32_t fieldHeight = FIELD_HEIGHT);

    // Основной метод симуляции - выполняет один шаг
    void step();

    // Применение изменений от клиентов (накапливаются до следующего step())
    void applySingleCellChange(uint32_t x, uint32_t y, bool alive);

    // Получение состояния для сети
    std::vector<std::byte> getStateData() const;

    uint32_t getAliveCellCount() const;

    void setInitialPattern(BitField pattern);

    void clearField();

   private:
    void performSimulationStep();

    bool shouldCellLive(uint32_t x, uint32_t y, const BitField& field) const;
};

} // namespace LifeGame
