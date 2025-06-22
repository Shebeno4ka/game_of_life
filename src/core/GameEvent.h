#pragma once

#include "GameSimulator.h"

#include <vector>

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
        for (auto [x, y, alive] : changes) {
            simulator.applySingleCellChange(x, y, alive);
        }
    }
};