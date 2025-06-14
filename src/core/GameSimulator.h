#pragma once

#include <vector>

#include "BitField.h"

namespace LifeGame {

/**
 * Многопоточный симулятор игры "Жизнь"
 * Чистый симулятор без логики игроков - только симуляция Conway's Game of Life
 */
class GameSimulator {
public:
    // Основной метод симуляции - выполняет один шаг
    void step();

    // Применение изменений от клиентов (накапливаются до следующего step())
    void applySingleCellChange(uint32_t x, uint32_t y, bool alive);

    // Получение состояния для сети
    std::vector<std::byte> getStateData() const;
    
    // Статистика
    uint32_t getAliveCellCount() const;

    // Установка начального состояния
    void setInitialPattern(BitField pattern);
    void clearField();

private:
    // Основные компоненты
    BitField currentField_;
    BitField nextField_;
    
    // Методы симуляции
    void performSimulationStep();
    
    // Применение правил игры "Жизнь"
    bool shouldCellLive(uint32_t x, uint32_t y, const BitField& field) const;
};

}
