#pragma once

#include <cstdint>
#include <vector>

#include "GameSimulator.h"

struct CellChange {
    uint32_t x, y;
    bool alive;
};

/**
 * Событие от клиента - изменение набора клеток
 */
struct GameEvent {
    std::vector<CellChange> changes;

    explicit GameEvent(std::vector<CellChange> cellChanges) : changes(std::move(cellChanges)) {}

    GameEvent() = default;

    GameEvent(GameEvent &&) = default;
    GameEvent &operator=(GameEvent &&) = default;

    void Run(LifeGame::GameSimulator &simulator) {
        for (const auto &[x, y, alive] : changes) {
            simulator.applySingleCellChange(x, y, alive);
        }
    }
};