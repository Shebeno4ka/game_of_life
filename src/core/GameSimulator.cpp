#include "GameSimulator.h"

#include <algorithm>

namespace LifeGame {

GameSimulator::GameSimulator(uint32_t fieldWidth, uint32_t fieldHeight)
    : currentField_(fieldWidth, fieldHeight), nextField_(fieldWidth, fieldHeight) {}

void GameSimulator::step() {
    performSimulationStep();
}

void GameSimulator::applySingleCellChange(uint32_t x, uint32_t y, bool alive) {
    currentField_.setAlive(x, y, alive);
}

std::vector<std::byte> GameSimulator::getStateData() const {
    return currentField_.getData();
}

uint32_t GameSimulator::getAliveCellCount() const {
    return currentField_.getAliveCellCount();
}

void GameSimulator::setInitialPattern(BitField pattern) {
    currentField_ = std::move(pattern);
}

void GameSimulator::clearField() {
    currentField_.clear();
    nextField_.clear();
}

void GameSimulator::performSimulationStep() {
    nextField_.clear();
    for (uint32_t i = 0; i < nextField_.height(); ++i) {
        for (uint32_t j = 0; j < nextField_.width(); ++j) {
            if (shouldCellLive(i, j, currentField_)) {
                nextField_.setAlive(i, j, true);
            }
        }
    }
    std::swap(nextField_, currentField_);
}

bool GameSimulator::shouldCellLive(uint32_t x, uint32_t y, const BitField& field) const {
    uint8_t neighbors = field.countNeighbors(x, y);
    bool currentlyAlive = field.isAlive(x, y);

    // Правила игры "Жизнь":
    // 1. Живая клетка с 2-3 соседями выживает
    // 2. Мертвая клетка с 3 соседями оживает
    // 3. Во всех остальных случаях клетка умирает или остается мертвой

    if (currentlyAlive) {
        return neighbors == 2 || neighbors == 3;
    }
    return neighbors == 3;
}
} // namespace LifeGame
