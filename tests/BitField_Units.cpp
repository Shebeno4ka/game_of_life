//
// Created by mike on 6/15/25.
//

#include <gtest/gtest.h>
#include "../src/core/BitField.h"

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
    
    // Set a cell alive
    field->setAlive(5, 5, true);
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
    // Test corners
    field->setAlive(0, 0, true);      // Top-left
    field->setAlive(9, 0, true);      // Top-right
    field->setAlive(0, 9, true);      // Bottom-left
    field->setAlive(9, 9, true);      // Bottom-right
    
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
    // Test neighbor counting in the middle
    field->setAlive(4, 4, true);
    field->setAlive(4, 5, true);
    field->setAlive(5, 4, true);
    
    EXPECT_EQ(field->countNeighbors(5, 5), 3);
    EXPECT_EQ(field->countNeighbors(4, 4), 2);
    EXPECT_EQ(field->countNeighbors(3, 3), 1);
    EXPECT_EQ(field->countNeighbors(6, 6), 0);
}

TEST_F(BitFieldTest, NeighborCountingEdgeCases) {
    // Corner cell with one neighbor
    field->setAlive(0, 1, true);
    EXPECT_EQ(field->countNeighbors(0, 0), 1);
    
    // Edge cell with neighbors
    field->setAlive(0, 4, true);
    field->setAlive(1, 4, true);
    field->setAlive(1, 5, true);
    EXPECT_EQ(field->countNeighbors(0, 5), 3);
    
    // Cell surrounded by all neighbors
    field->clear();
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            field->setAlive(5 + dx, 5 + dy, true);
        }
    }
    EXPECT_EQ(field->countNeighbors(5, 5), 8);
}

TEST_F(BitFieldTest, ClearField) {
    // Set some cells alive
    field->setAlive(1, 1, true);
    field->setAlive(2, 2, true);
    field->setAlive(3, 3, true);
    
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
    field->setAlive(0, 0, true);
    field->setAlive(1, 1, true);
    field->setAlive(2, 2, true);
    
    auto data = field->getData();
    EXPECT_FALSE(data.empty());
    
    // Data should be consistent across calls
    auto data2 = field->getData();
    EXPECT_EQ(data.size(), data2.size());
    EXPECT_EQ(data, data2);
}

TEST_F(BitFieldTest, LargeField) {
    // Test with a larger field
    BitField largeField(100, 100);
    
    EXPECT_EQ(largeField.width(), 100);
    EXPECT_EQ(largeField.height(), 100);
    EXPECT_EQ(largeField.getAliveCellCount(), 0);
    
    // Set some cells in the large field
    largeField.setAlive(50, 50, true);
    largeField.setAlive(99, 99, true);
    
    EXPECT_TRUE(largeField.isAlive(50, 50));
    EXPECT_TRUE(largeField.isAlive(99, 99));
    EXPECT_EQ(largeField.getAliveCellCount(), 2);
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
    // Test that bit packing works correctly by setting many cells
    for (uint32_t i = 0; i < 64; ++i) {
        field->setAlive(i % field->width(), i / field->width(), true);
    }
    
    EXPECT_EQ(field->getAliveCellCount(), 64);
    
    // Verify each cell is set correctly
    for (uint32_t i = 0; i < 64; ++i) {
        EXPECT_TRUE(field->isAlive(i % field->width(), i / field->width()));
    }
}

TEST_F(BitFieldTest, MoveConstructor) {
    field->setAlive(3, 3, true);
    field->setAlive(4, 4, true);
    
    BitField movedField = std::move(*field);
    
    EXPECT_EQ(movedField.width(), 10);
    EXPECT_EQ(movedField.height(), 10);
    EXPECT_EQ(movedField.getAliveCellCount(), 2);
    EXPECT_TRUE(movedField.isAlive(3, 3));
    EXPECT_TRUE(movedField.isAlive(4, 4));
}

} // namespace LifeGame
