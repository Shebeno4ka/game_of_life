//
// Created by mike on 6/15/25.
//

#include <gtest/gtest.h>
#include "core/BitField.h"
#include "utils/SimulationPatterns.h"

namespace LifeGame {

class BitFieldTest : public ::testing::Test {
protected:
    void SetUp() override {
        field = std::make_unique<BitField>(10, 10);
    }

    std::unique_ptr<BitField> field;
};

TEST_F(BitFieldTest, ConstructorInitialization) {
    EXPECT_EQ(field->width(), 10);
    EXPECT_EQ(field->height(), 10);
    EXPECT_EQ(field->getAliveCellCount(), 0);
}

TEST_F(BitFieldTest, BasicCellOperations) {
    // Initially all cells should be dead
    EXPECT_FALSE(field->isAlive(5, 5));
    
    // Set a cell alive using CellState
    utils::applyPattern(*field, {utils::CellState(5, 5)});
    EXPECT_TRUE(field->isAlive(5, 5));
    EXPECT_EQ(field->getAliveCellCount(), 1);
    
    // Set it back to dead
    field->setAlive(5, 5, false);
    EXPECT_FALSE(field->isAlive(5, 5));
    EXPECT_EQ(field->getAliveCellCount(), 0);
}

TEST_F(BitFieldTest, ToggleCell) {
    EXPECT_FALSE(field->isAlive(3, 3));
    
    field->toggleCell(3, 3);
    EXPECT_TRUE(field->isAlive(3, 3));
    EXPECT_EQ(field->getAliveCellCount(), 1);
    
    field->toggleCell(3, 3);
    EXPECT_FALSE(field->isAlive(3, 3));
    EXPECT_EQ(field->getAliveCellCount(), 0);
}

TEST_F(BitFieldTest, BoundaryConditions) {
    // Test corners using pattern application
    std::vector<utils::CellState> cornerPattern = {
        {0, 0}, {9, 0}, {0, 9}, {9, 9}
    };
    utils::applyPattern(*field, cornerPattern);
    
    EXPECT_TRUE(field->isAlive(0, 0));
    EXPECT_TRUE(field->isAlive(9, 0));
    EXPECT_TRUE(field->isAlive(0, 9));
    EXPECT_TRUE(field->isAlive(9, 9));
    EXPECT_EQ(field->getAliveCellCount(), 4);
}

TEST_F(BitFieldTest, OutOfBoundsAccess) {
    // Out of bounds access should return false and not crash
    EXPECT_FALSE(field->isAlive(10, 5));
    EXPECT_FALSE(field->isAlive(5, 10));
    EXPECT_FALSE(field->isAlive(15, 15));
    EXPECT_FALSE(field->isAlive(UINT32_MAX, UINT32_MAX));
    
    // Setting out of bounds should not crash
    field->setAlive(10, 5, true);
    field->setAlive(5, 10, true);
    field->setAlive(15, 15, true);
    
    EXPECT_EQ(field->getAliveCellCount(), 0);
}

TEST_F(BitFieldTest, NeighborCounting) {
    // Create a small L-shape pattern
    std::vector<utils::CellState> lPattern = {
        {4, 4}, {4, 5}, {5, 4}
    };
    utils::applyPattern(*field, lPattern);
    
    EXPECT_EQ(field->countNeighbors(5, 5), 3);
    EXPECT_EQ(field->countNeighbors(4, 4), 2);
    EXPECT_EQ(field->countNeighbors(3, 3), 1);
    EXPECT_EQ(field->countNeighbors(6, 6), 0);
}

TEST_F(BitFieldTest, NeighborCountingEdgeCases) {
    // Corner cell with one neighbor
    utils::applyPattern(*field, {utils::CellState(0, 1)});
    EXPECT_EQ(field->countNeighbors(0, 0), 1);
    
    // Edge cell with neighbors
    std::vector<utils::CellState> edgePattern = {
        {0, 4}, {1, 4}, {1, 5}
    };
    utils::applyPattern(*field, edgePattern);
    EXPECT_EQ(field->countNeighbors(0, 5), 3);
    
    // Cell surrounded by all neighbors
    field->clear();
    std::vector<utils::CellState> surroundPattern;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            surroundPattern.push_back({static_cast<uint32_t>(5 + dx), static_cast<uint32_t>(5 + dy)});
        }
    }
    utils::applyPattern(*field, surroundPattern);
    EXPECT_EQ(field->countNeighbors(5, 5), 8);
}

TEST_F(BitFieldTest, ClearField) {
    // Apply a blinker pattern
    auto blinkerPattern = utils::Patterns::blinker(2, 2);
    utils::applyPattern(*field, blinkerPattern);
    
    EXPECT_EQ(field->getAliveCellCount(), 3);
    
    field->clear();
    EXPECT_EQ(field->getAliveCellCount(), 0);
    
    // Verify all cells are dead
    for (uint32_t y = 0; y < field->height(); ++y) {
        for (uint32_t x = 0; x < field->width(); ++x) {
            EXPECT_FALSE(field->isAlive(x, y));
        }
    }
}

TEST_F(BitFieldTest, DataExport) {
    // Apply a block pattern
    auto blockPattern = utils::Patterns::block(0, 0);
    utils::applyPattern(*field, blockPattern);
    
    auto data = field->serialize();
    EXPECT_FALSE(data.empty());
    
    // Data should be consistent across calls
    auto data2 = field->serialize();
    EXPECT_EQ(data.size(), data2.size());
    EXPECT_EQ(data, data2);
}

TEST_F(BitFieldTest, LargeField) {
    // Test with a larger field
    BitField largeField(100, 100);
    
    EXPECT_EQ(largeField.width(), 100);
    EXPECT_EQ(largeField.height(), 100);
    EXPECT_EQ(largeField.getAliveCellCount(), 0);
    
    // Apply a glider pattern
    auto gliderPattern = utils::Patterns::glider(50, 50);
    utils::applyPattern(largeField, gliderPattern);
    
    EXPECT_TRUE(largeField.isAlive(51, 50));  // Part of glider
    EXPECT_TRUE(largeField.isAlive(52, 52));  // Part of glider
    EXPECT_EQ(largeField.getAliveCellCount(), 5); // Glider has 5 cells
}

TEST_F(BitFieldTest, SmallField) {
    // Test with minimal field size
    BitField smallField(1, 1);
    
    EXPECT_EQ(smallField.width(), 1);
    EXPECT_EQ(smallField.height(), 1);
    EXPECT_EQ(smallField.getAliveCellCount(), 0);
    
    smallField.setAlive(0, 0, true);
    EXPECT_TRUE(smallField.isAlive(0, 0));
    EXPECT_EQ(smallField.getAliveCellCount(), 1);
    EXPECT_EQ(smallField.countNeighbors(0, 0), 0);
}

TEST_F(BitFieldTest, BitPackingCorrectness) {
    // Test that bit packing works correctly by setting many cells with random fill
    utils::fillRandom(*field, 0.7, 42); // 70% density with seed 42
    
    // Count how many cells are alive
    int aliveCount = 0;
    for (uint32_t y = 0; y < field->height(); ++y) {
        for (uint32_t x = 0; x < field->width(); ++x) {
            if (field->isAlive(x, y)) {
                aliveCount++;
            }
        }
    }
    
    EXPECT_EQ(field->getAliveCellCount(), aliveCount);
    
    // With 70% density in 10x10 field, we expect around 70 cells
    EXPECT_GT(aliveCount, 30); // Should have significant number of cells
}

TEST_F(BitFieldTest, MoveConstructor) {
    // Apply an R-pentomino pattern
    auto rPentominoPattern = utils::Patterns::rPentomino(5, 5);
    utils::applyPattern(*field, rPentominoPattern);
    
    BitField movedField = std::move(*field);
    
    EXPECT_EQ(movedField.width(), 10);
    EXPECT_EQ(movedField.height(), 10);
    EXPECT_EQ(movedField.getAliveCellCount(), 5); // R-pentomino has 5 cells
    EXPECT_TRUE(movedField.isAlive(5, 4));  // Verify specific cells from the pattern
    EXPECT_TRUE(movedField.isAlive(4, 5));
    EXPECT_TRUE(movedField.isAlive(5, 6));
}

// Add a test specifically for common patterns
TEST_F(BitFieldTest, CommonPatterns) {
    // Test block pattern
    field->clear();
    auto blockPattern = utils::Patterns::block(1, 1);
    utils::applyPattern(*field, blockPattern);
    EXPECT_EQ(field->getAliveCellCount(), 4);
    EXPECT_TRUE(field->isAlive(1, 1));
    EXPECT_TRUE(field->isAlive(2, 2));
    
    // Test blinker pattern
    field->clear();
    auto blinkerPattern = utils::Patterns::blinker(5, 5);
    utils::applyPattern(*field, blinkerPattern);
    EXPECT_EQ(field->getAliveCellCount(), 3);
    EXPECT_TRUE(field->isAlive(5, 4));
    EXPECT_TRUE(field->isAlive(5, 5));
    EXPECT_TRUE(field->isAlive(5, 6));
    
    // Test glider pattern
    field->clear();
    auto gliderPattern = utils::Patterns::glider(2, 2);
    utils::applyPattern(*field, gliderPattern);
    EXPECT_EQ(field->getAliveCellCount(), 5);
    EXPECT_TRUE(field->isAlive(3, 2));
    EXPECT_TRUE(field->isAlive(4, 3));
    EXPECT_TRUE(field->isAlive(2, 4));
    EXPECT_TRUE(field->isAlive(3, 4));
    EXPECT_TRUE(field->isAlive(4, 4));
}

} // namespace LifeGame
