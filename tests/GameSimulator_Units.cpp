//
// Created by mike on 6/15/25.
//

#include <gtest/gtest.h>
#include "core/GameSimulator.h"
#include "core/BitField.h"
#include "ReferenceGameSimulator.h"
#include "SimulationPatterns.h"

namespace LifeGame {

class GameSimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        simulator = std::make_unique<GameSimulator>(10, 10);
        referenceSimulator = std::make_unique<Testing::ReferenceGameSimulator>(10, 10);
    }

    std::unique_ptr<GameSimulator> simulator;
    std::unique_ptr<Testing::ReferenceGameSimulator> referenceSimulator;
};

TEST_F(GameSimulatorTest, EmptyFieldStaysEmpty) {
    simulator->clearField();
    referenceSimulator->clearField();
    
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
    
    simulator->step();
    referenceSimulator->step();
    
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, SingleCellDies) {
    simulator->clearField();
    referenceSimulator->clearField();
    
    Testing::applyPatternToAll({{5, 5}}, *simulator, *referenceSimulator);
    
    simulator->step();
    referenceSimulator->step();
    
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, BlockPatternStable) {
    simulator->clearField();
    referenceSimulator->clearField();
    
    // 2x2 block pattern
    Testing::applyPatternToAll(Testing::Patterns::block(4, 4), *simulator, *referenceSimulator);
    uint32_t initialCount = simulator->getAliveCellCount();
    EXPECT_EQ(initialCount, 4);
    
    simulator->step();
    referenceSimulator->step();
    
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), initialCount);
}

TEST_F(GameSimulatorTest, BlinkerPatternOscillates) {
    simulator->clearField();
    referenceSimulator->clearField();

    Testing::applyPatternToAll(Testing::Patterns::blinker(5, 5), *simulator, *referenceSimulator);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    
    // After one step, should become horizontal
    simulator->step();
    referenceSimulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    
    // After another step, should return to vertical
    simulator->step();
    referenceSimulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, MultipleStepsConsistency) {
    simulator->clearField();
    referenceSimulator->clearField();

    std::vector<Testing::CellState> customPattern = {
        {3, 3}, {3, 4}, {3, 5}, {4, 3}, {5, 4}
    };
    Testing::applyPatternToAll(customPattern, *simulator, *referenceSimulator);
    
    // Run multiple steps and verify consistency with reference
    for (int step = 0; step < 10; ++step) {
        simulator->step();
        referenceSimulator->step();
        
        uint32_t simulatorCount = simulator->getAliveCellCount();
        uint32_t referenceCount = referenceSimulator->getAliveCellCount();
        
        EXPECT_EQ(simulatorCount, referenceCount) 
            << "Mismatch at step " << step;
    }
}

TEST_F(GameSimulatorTest, ApplyChangesAccumulation) {
    simulator->clearField();
    referenceSimulator->clearField();
    
    std::vector<Testing::CellState> changes = {
        {2, 2}, {2, 3}, {2, 4}, {3, 2}
    };
    Testing::applyPatternToAll(changes, *simulator, *referenceSimulator);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 4);
    EXPECT_EQ(referenceSimulator->getAliveCellCount(), 4);
    
    // Apply contradictory change
    Testing::applyPatternToAll({{2, 2, false}}, *simulator, *referenceSimulator);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    EXPECT_EQ(referenceSimulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, EdgeCells) {
    simulator->clearField();
    referenceSimulator->clearField();
    
    std::vector<Testing::CellState> edgeCells = {
        {0, 0},  // Top-left corner
        {9, 9},  // Bottom-right corner
        {0, 5},  // Left edge
        {9, 5}   // Right edge
    };
    Testing::applyPatternToAll(edgeCells, *simulator, *referenceSimulator);
    
    EXPECT_EQ(simulator->getAliveCellCount(), 4);
    
    // These should all die due to insufficient neighbors
    simulator->step();
    referenceSimulator->step();
    
    EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, StateDataConsistency) {
    simulator->clearField();
    referenceSimulator->clearField();

    Testing::applyPatternToAll({{3, 3}, {4, 4}}, *simulator, *referenceSimulator);
    
    auto simulatorStateData = simulator->getStateData();
    auto referenceStateData = referenceSimulator->getStateData();
    
    EXPECT_FALSE(simulatorStateData.empty());
    EXPECT_EQ(simulatorStateData, referenceStateData);
    
    // State data should be consistent across calls
    auto simulatorStateData2 = simulator->getStateData();
    EXPECT_EQ(simulatorStateData.size(), simulatorStateData2.size());
    EXPECT_EQ(simulatorStateData, simulatorStateData2);
}

TEST_F(GameSimulatorTest, ClearField) {
    // Add some cells
    simulator->clearField();
    referenceSimulator->clearField();
    Testing::applyPatternToAll({{1, 1}, {2, 2}, {3, 3}}, *simulator, *referenceSimulator);
    
    EXPECT_GT(simulator->getAliveCellCount(), 0);
    
    simulator->clearField();
    referenceSimulator->clearField();
    
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
    EXPECT_EQ(referenceSimulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, ImplementationMatchesReference) {
    // Initialize with a complex pattern
    BitField initialPattern(10, 10);
    std::vector<Testing::CellState> complexPattern = {
        {2, 2}, {3, 2}, {2, 3}, {3, 3}, {4, 4}, {5, 4}, {6, 4}
    };
    Testing::applyPattern(initialPattern, complexPattern);
    
    simulator->setInitialPattern(initialPattern);
    referenceSimulator->setInitialPattern(initialPattern);
    
    // Run multiple steps and ensure both implementations match
    for (int i = 0; i < 20; ++i) {
        simulator->step();
        referenceSimulator->step();
        
        auto simulatorState = simulator->getStateData();
        auto referenceState = referenceSimulator->getStateData();
        
        EXPECT_EQ(simulatorState, referenceState)
            << "Simulation mismatch at step " << i;
        
        EXPECT_EQ(simulator->getAliveCellCount(), referenceSimulator->getAliveCellCount())
            << "Count mismatch at step " << i;
    }
}

} // namespace LifeGame
