#pragma once

#include "core/BitField.h"
#include <vector>
#include <cstdint>


namespace LifeGame::utils {

/**
 * A simplified reference implementation of GameSimulator
 * Used for testing and validation of the real GameSimulator
 */
class ReferenceGameSimulator {
private:
    BitField currentField_;

public:
    explicit ReferenceGameSimulator(uint32_t fieldWidth = FIELD_WIDTH, uint32_t fieldHeight = FIELD_HEIGHT)
        : currentField_(fieldWidth, fieldHeight) {}

    void step() {
        BitField nextField(currentField_.width(), currentField_.height());
        
        for (uint32_t y = 0; y < currentField_.height(); ++y) {
            for (uint32_t x = 0; x < currentField_.width(); ++x) {
                uint8_t neighbors = currentField_.countNeighbors(x, y);
                bool currentlyAlive = currentField_.isAlive(x, y);
                
                // Conway's Game of Life rules
                bool shouldLive = false;
                if (currentlyAlive && (neighbors == 2 || neighbors == 3)) {
                    shouldLive = true;  // Survival
                } else if (!currentlyAlive && neighbors == 3) {
                    shouldLive = true;  // Birth
                }
                
                nextField.setAlive(x, y, shouldLive);
            }
        }
        
        currentField_ = std::move(nextField);
    }

    void applySingleCellChange(uint32_t x, uint32_t y, bool alive) {
        currentField_.setAlive(x, y, alive);
    }

    std::vector<std::byte> getStateData() const {
        return currentField_.serialize();
    }

    uint32_t getAliveCellCount() const {
        return currentField_.getAliveCellCount();
    }

    void setInitialPattern(BitField pattern) {
        currentField_ = std::move(pattern);
    }

    void clearField() {
        currentField_.clear();
    }

    const BitField& getCurrentField() const {
        return currentField_;
    }
};

} // namespace LifeGame::Testing

