#include "core/GameSimulator.h"
#include "mock/SandGameSimulator.h"
#include "utils/SimulationPatterns.h"

#include <gtest/gtest.h>

namespace LifeGame {

class GameSimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        simulator = std::make_unique<GameSimulator>(10, 10);
        sandSimulator = std::make_unique<utils::SandGameSimulator>(10, 10);
    }

    std::unique_ptr<GameSimulator> simulator;
    std::unique_ptr<utils::SandGameSimulator> sandSimulator;
};

TEST_F(GameSimulatorTest, EmptyFieldStaysEmpty) {
    EXPECT_EQ(simulator->getAliveCellCount(), 0);

    simulator->step();
    sandSimulator->step();

    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, SingleCellDies) {
    utils::applyPatternToAll({{5, 5}}, *simulator, *sandSimulator);

    simulator->step();
    sandSimulator->step();

    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, BlockPatternStable) {
    // 2x2 block pattern
    utils::applyPatternToAll(utils::Patterns::block(4, 4), *simulator, *sandSimulator);
    uint32_t initialCount = simulator->getAliveCellCount();
    EXPECT_EQ(initialCount, 4);

    simulator->step();
    sandSimulator->step();

    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), initialCount);
}

TEST_F(GameSimulatorTest, BlinkerPatternOscillates) {
    utils::applyPatternToAll(utils::Patterns::blinker(5, 5), *simulator, *sandSimulator);

    EXPECT_EQ(simulator->getAliveCellCount(), 3);

    // After one step, should become horizontal
    simulator->step();
    sandSimulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 3);

    // After another step, should return to vertical
    simulator->step();
    sandSimulator->step();
    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, MultipleStepsConsistency) {
    std::vector<utils::CellState> customPattern = {
        {3, 3}, {3, 4}, {3, 5}, {4, 3}, {5, 4}
    };
    utils::applyPatternToAll(customPattern, *simulator, *sandSimulator);

    // Run multiple steps and verify consistency with reference
    for (int step = 0; step < 10; ++step) {
        simulator->step();
        sandSimulator->step();

        uint32_t simulatorCount = simulator->getAliveCellCount();
        uint32_t referenceCount = sandSimulator->getAliveCellCount();

        EXPECT_EQ(simulatorCount, referenceCount)
            << "Mismatch at step " << step;
    }
}

TEST_F(GameSimulatorTest, ApplyChangesAccumulation) {
    std::vector<utils::CellState> changes = {
        {2, 2}, {2, 3}, {2, 4}, {3, 2}
    };
    utils::applyPatternToAll(changes, *simulator, *sandSimulator);

    EXPECT_EQ(simulator->getAliveCellCount(), 4);
    EXPECT_EQ(sandSimulator->getAliveCellCount(), 4);

    // Apply contradictory change
    utils::applyPatternToAll({{2, 2, false}}, *simulator, *sandSimulator);

    EXPECT_EQ(simulator->getAliveCellCount(), 3);
    EXPECT_EQ(sandSimulator->getAliveCellCount(), 3);
}

TEST_F(GameSimulatorTest, EdgeCells) {
    std::vector<utils::CellState> edgeCells = {
        {0, 0},  // Top-left corner
        {9, 9},  // Bottom-right corner
        {0, 5},  // Left edge
        {9, 5}   // Right edge
    };
    utils::applyPatternToAll(edgeCells, *simulator, *sandSimulator);

    EXPECT_EQ(simulator->getAliveCellCount(), 4);

    // These should all die due to insufficient neighbors
    simulator->step();
    sandSimulator->step();

    EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount());
    EXPECT_EQ(simulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, StateDataConsistency) {
    utils::applyPatternToAll({{3, 3}, {4, 4}}, *simulator, *sandSimulator);

    auto simulatorStateData = simulator->getSerializedField();
    auto referenceStateData = sandSimulator->getStateData();

    EXPECT_FALSE(simulatorStateData.empty());
    EXPECT_EQ(simulatorStateData, referenceStateData);

    // State data should be consistent across calls
    auto simulatorStateData2 = simulator->getSerializedField();
    EXPECT_EQ(simulatorStateData.size(), simulatorStateData2.size());
    EXPECT_EQ(simulatorStateData, simulatorStateData2);
}

TEST_F(GameSimulatorTest, ClearField) {
    // add some cells
    utils::applyPatternToAll({{1, 1}, {2, 2}, {3, 3}}, *simulator, *sandSimulator);

    EXPECT_GT(simulator->getAliveCellCount(), 0);

    simulator->clearField();
    sandSimulator->clearField();

    EXPECT_EQ(simulator->getAliveCellCount(), 0);
    EXPECT_EQ(sandSimulator->getAliveCellCount(), 0);
}

TEST_F(GameSimulatorTest, ImplementationMatchesReference) {
    // Initialize with a complex pattern
    std::vector<utils::CellState> complexPattern = {
        {2, 2}, {3, 2}, {2, 3}, {3, 3}, {4, 4}, {5, 4}, {6, 4}
    };
    utils::applyPatternToAll(complexPattern, *simulator, *sandSimulator);

    // Run multiple steps and ensure both implementations match
    for (int i = 0; i < 20; ++i) {
        simulator->step();
        sandSimulator->step();

        auto simulatorState = simulator->getSerializedField();
        auto referenceState = sandSimulator->getStateData();

        EXPECT_EQ(simulatorState, referenceState)
            << "Simulation mismatch at step " << i;

        EXPECT_EQ(simulator->getAliveCellCount(), sandSimulator->getAliveCellCount())
            << "Count mismatch at step " << i;
    }
}

} // namespace LifeGame
