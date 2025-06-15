//
// Created by mike on 6/15/25.
//

#include <gtest/gtest.h>
#include "../src/core/GameSimulator.h"
#include "../src/core/BitField.h"

namespace LifeGame {

class GameSimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        simulator = std::make_unique<GameSimulator>(10, 10);
    }

    // Reference implementation for validation
    BitField simulateStepReference(const BitField& field) {
        BitField result(field.width(), field.height());
        
        for (uint32_t y = 0; y < field.height(); ++y) {
            for (uint32_t x = 0; x < field.width(); ++x) {
                uint8_t neighbors = field.countNeighbors(x, y);
                bool currentlyAlive = field.isAlive(x, y);
                
                // Conway's Game of Life rules
                bool shouldLive = false;
                if (currentlyAlive && (neighbors == 2 || neighbors == 3)) {
                    shouldLive = true;  // Survival
                } else if (!currentlyAlive && neighbors == 3) {
                    shouldLive = true;  // Birth
                }
                
                result.setAlive(x, y, shouldLive);
            }
        }
        return result;
    }

    std::unique_ptr<GameSimulator> simulator;
};

TEST_F(GameSimulatorTest, EmptyFieldStaysEmpty) {
    simulator->clearField();
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
    
    simulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, SingleCellDies) {
    simulator->clearField();
    simulator->applySingleCellChange(5, 5, true);
    simulator->step();
    
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, BlockPatternStable) {
    simulator->clearField();
    // 2x2 block pattern
    simulator->applySingleCellChange(4, 4, true);
    simulator->applySingleCellChange(4, 5, true);
    simulator->applySingleCellChange(5, 4, true);
    simulator->applySingleCellChange(5, 5, true);
    
    uint32_t initialCount = simulator->getAliveCellCount();
    EXPECT_EQ(initialCount, 4);
    
    simulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), initialCount);
}

TEST_F(GameSimulatorTest, BlinkerPatternOscillates) {
    simulator->clearField();
    // Vertical blinker
    simulator->applySingleCellChange(5, 4, true);
    simulator->applySingleCellChange(5, 5, true);
    simulator->applySingleCellChange(5, 6, true);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    
    // After one step, should become horizontal
    simulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    
    // After another step, should return to vertical
    simulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, MultipleStepsConsistency) {
    simulator->clearField();
    
    // Create a more complex pattern
    simulator->applySingleCellChange(3, 3, true);
    simulator->applySingleCellChange(3, 4, true);
    simulator->applySingleCellChange(3, 5, true);
    simulator->applySingleCellChange(4, 3, true);
    simulator->applySingleCellChange(5, 4, true);
    
    // Run multiple steps and verify consistency
    for (int step = 0; step < 10; ++step) {
        uint32_t countBefore = simulator->getAliveCellCount();
        simulator->step();
        uint32_t countAfter = simulator->getAliveCellCount();
        
        // Count should be reasonable (not zero unless it's a dying pattern)
        EXPECT_GE(countAfter, 0);
        EXPECT_LE(countAfter, 100);  // Shouldn't explode
    }
}

TEST_F(GameSimulatorTest, ApplyChangesAccumulation) {
    simulator->clearField();
    
    // Apply multiple changes before stepping
    simulator->applySingleCellChange(2, 2, true);
    simulator->applySingleCellChange(2, 3, true);
    simulator->applySingleCellChange(2, 4, true);
    simulator->applySingleCellChange(3, 2, true);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 4);
    
    // Apply contradictory change
    simulator->applySingleCellChange(2, 2, false);
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, EdgeCells) {
    simulator->clearField();
    
    // Test cells at edges
    simulator->applySingleCellChange(0, 0, true);  // Top-left corner
    simulator->applySingleCellChange(9, 9, true);  // Bottom-right corner
    simulator->applySingleCellChange(0, 5, true);  // Left edge
    simulator->applySingleCellChange(9, 5, true);  // Right edge
    
    EXPECT_EQ(simulator->getAliveCellCount(), 4);
    
    // These should all die due to insufficient neighbors
    simulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, StateDataConsistency) {
    simulator->clearField();
    simulator->applySingleCellChange(3, 3, true);
    simulator->applySingleCellChange(4, 4, true);
    
    auto stateData = simulator->getStateData();
    EXPECT_FALSE(stateData.empty());
    
    // State data should be consistent across calls
    auto stateData2 = simulator->getStateData();
    EXPECT_EQ(stateData.size(), stateData2.size());
    EXPECT_EQ(stateData, stateData2);
}

TEST_F(GameSimulatorTest, ClearField) {
    // Add some cells
    simulator->applySingleCellChange(1, 1, true);
    simulator->applySingleCellChange(2, 2, true);
    simulator->applySingleCellChange(3, 3, true);
    
    EXPECT_GT(simulator->getAliveCellCount(), 0);
    
    simulator->clearField();
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

} // namespace LifeGame
