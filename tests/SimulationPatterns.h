#pragma once

#include <vector>
#include <random>
#include <utility>
#include "core/GameSimulator.h"
#include "core/BitField.h"
#include "ReferenceGameSimulator.h"

namespace LifeGame::Testing {

// Represents a single cell's coordinates and state
struct CellState {
    uint32_t x;
    uint32_t y;
    bool alive;

    CellState(uint32_t x, uint32_t y, bool alive = true) : x(x), y(y), alive(alive) {}
};

// Common patterns in Game of Life
namespace Patterns {
// Returns a 2x2 block pattern at specified position
inline std::vector<CellState> block(uint32_t topLeftX, uint32_t topLeftY) {
    return {{topLeftX, topLeftY}, {topLeftX + 1, topLeftY}, {topLeftX, topLeftY + 1}, {topLeftX + 1, topLeftY + 1}};
}

// Returns a vertical blinker pattern (3 cells)
inline std::vector<CellState> blinker(uint32_t centerX, uint32_t centerY) {
    return {{centerX, centerY - 1}, {centerX, centerY}, {centerX, centerY + 1}};
}

// Returns a glider pattern at specified position
inline std::vector<CellState> glider(uint32_t topLeftX, uint32_t topLeftY) {
    return {{topLeftX + 1, topLeftY},
            {topLeftX + 2, topLeftY + 1},
            {topLeftX, topLeftY + 2},
            {topLeftX + 1, topLeftY + 2},
            {topLeftX + 2, topLeftY + 2}};
}

// Returns a R-pentomino pattern (a common high-activity pattern)
inline std::vector<CellState> rPentomino(uint32_t centerX, uint32_t centerY) {
    return {{centerX, centerY - 1},
            {centerX + 1, centerY - 1},
            {centerX - 1, centerY},
            {centerX, centerY},
            {centerX, centerY + 1}};
}
} // namespace Patterns

// Apply a pattern to a BitField
inline void applyPattern(BitField& field, const std::vector<CellState>& pattern) {
    for (const auto& cell : pattern) {
        field.setAlive(cell.x, cell.y, cell.alive);
    }
}

// Apply a pattern to a GameSimulator
inline void applyPattern(GameSimulator& simulator, const std::vector<CellState>& pattern) {
    for (const auto& cell : pattern) {
        simulator.applySingleCellChange(cell.x, cell.y, cell.alive);
    }
}

// Apply a pattern to a ReferenceGameSimulator
inline void applyPattern(ReferenceGameSimulator& simulator, const std::vector<CellState>& pattern) {
    for (const auto& cell : pattern) {
        simulator.applySingleCellChange(cell.x, cell.y, cell.alive);
    }
}

// Apply the same pattern to multiple simulators
template <typename... Simulators>
void applyPatternToAll(const std::vector<CellState>& pattern, Simulators&... simulators) {
    (applyPattern(simulators, pattern), ...);
}

// Fill a BitField with random cells based on density
inline void fillRandom(BitField& field, double density, uint32_t seed = 42) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (uint32_t y = 0; y < field.height(); ++y) {
        for (uint32_t x = 0; x < field.width(); ++x) {
            if (dist(gen) < density) {
                field.setAlive(x, y, true);
            }
        }
    }
}

// Fill a simulator with random cells based on density
template <typename SimulatorType>
void fillRandom(SimulatorType& simulator, uint32_t width, uint32_t height, double density, uint32_t seed = 42) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            if (dist(gen) < density) {
                simulator.applySingleCellChange(x, y, true);
            }
        }
    }
}

// Fill multiple simulators with the same random pattern
template <typename... Simulators>
void fillRandomToAll(uint32_t width, uint32_t height, double density, uint32_t seed, Simulators&... simulators) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dist(0.0, 1.0);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            bool alive = dist(gen) < density;
            if (alive) {
                (simulators.applySingleCellChange(x, y, true), ...);
            }
        }
    }
}

} // namespace LifeGame::Testing