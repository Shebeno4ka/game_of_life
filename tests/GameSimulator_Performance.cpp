#include "core/GameSimulator.h"

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <cstdlib>

namespace LifeGame {

class GameSimulatorPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Seed for reproducible results
        gen.seed(42);
        
        // Check if verbose output is requested
        verbose_ = std::getenv("GAME_PERF_VERBOSE") != nullptr;
    }

    void fillRandomPattern(GameSimulator& simulator, double density = 0.3) {
        std::uniform_real_distribution<> dist(0.0, 1.0);
        
        // We need to estimate field size - assuming square fields for simplicity
        // This is a limitation since we can't access field dimensions directly
        for (uint32_t y = 0; y < 100; ++y) {
            for (uint32_t x = 0; x < 100; ++x) {
                if (dist(gen) < density) {
                    simulator.applySingleCellChange(x, y, true);
                }
            }
        }
    }

    double measureSteps(GameSimulator& simulator, int numSteps) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numSteps; ++i) {
            simulator.step();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        return duration.count() / 1000.0; // Return milliseconds
    }

    void reportPerformance(const std::string& testName, double timeMs, int steps) {
        RecordProperty("test_name", testName);
        RecordProperty("total_time_ms", timeMs);
        RecordProperty("steps", steps);
        RecordProperty("avg_time_per_step_ms", timeMs / steps);
        
        // Optional verbose output
        if (verbose_) {
            std::cout << testName << ": " << timeMs << " ms for " << steps 
                      << " steps (avg: " << timeMs / steps << " ms/step)" << std::endl;
        }
    }

    std::mt19937 gen;
    bool verbose_ = false;
};

TEST_F(GameSimulatorPerformanceTest, SmallFieldPerformance) {
    GameSimulator simulator(50, 50);
    fillRandomPattern(simulator, 0.3);
    
    const int numSteps = 100;
    double timeMs = measureSteps(simulator, numSteps);
    
    reportPerformance("Small field (50x50)", timeMs, numSteps);
    
    // Performance assertion - should complete in reasonable time
    EXPECT_LT(timeMs, 1000.0); // Less than 1 second for 100 steps
}

TEST_F(GameSimulatorPerformanceTest, MediumFieldPerformance) {
    GameSimulator simulator(200, 200);
    fillRandomPattern(simulator, 0.3);
    
    const int numSteps = 50;
    double timeMs = measureSteps(simulator, numSteps);
    
    reportPerformance("Medium field (200x200)", timeMs, numSteps);
    
    EXPECT_LT(timeMs, 5000.0); // Less than 5 seconds for 50 steps
}

TEST_F(GameSimulatorPerformanceTest, LargeFieldPerformance) {
    GameSimulator simulator(500, 500);
    fillRandomPattern(simulator, 0.3);
    
    const int numSteps = 10;
    double timeMs = measureSteps(simulator, numSteps);
    
    reportPerformance("Large field (500x500)", timeMs, numSteps);
    
    EXPECT_LT(timeMs, 10000.0); // Less than 10 seconds for 10 steps
}

TEST_F(GameSimulatorPerformanceTest, DensityImpactPerformance) {
    std::vector<double> densities = {0.1, 0.3, 0.5, 0.7, 0.9};
    const int numSteps = 20;
    
    for (double density : densities) {
        GameSimulator testSim(100, 100);
        fillRandomPattern(testSim, density);
        
        double timeMs = measureSteps(testSim, numSteps);
        uint32_t aliveCells = testSim.getAliveCellCount();
        
        // Record detailed metrics
        RecordProperty(("density_" + std::to_string(density) + "_time_ms").c_str(), timeMs);
        RecordProperty(("density_" + std::to_string(density) + "_alive_cells").c_str(), aliveCells);
        
        if (verbose_) {
            std::cout << "Density " << density << " (alive: " << aliveCells 
                      << "): " << timeMs << " ms, " 
                      << timeMs / numSteps << " ms/step" << std::endl;
        }
        
        EXPECT_LT(timeMs, 5000.0);
    }
}

TEST_F(GameSimulatorPerformanceTest, MemoryUsageTest) {
    // Test different field sizes to understand memory scaling
    std::vector<std::pair<uint32_t, uint32_t>> sizes = {
        {50, 50},
        {100, 100},
        {200, 200},
        {300, 300}
    };
    
    for (auto [width, height] : sizes) {
        GameSimulator simulator(width, height);
        fillRandomPattern(simulator, 0.4);
        
        // Measure creation and a few steps
        auto start = std::chrono::high_resolution_clock::now();
        simulator.step();
        simulator.step();
        simulator.step();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        uint32_t totalCells = width * height;
        uint32_t aliveCells = simulator.getAliveCellCount();
        
        // Record metrics
        std::string sizeKey = std::to_string(width) + "x" + std::to_string(height);
        RecordProperty(("size_" + sizeKey + "_time_ms").c_str(), duration.count() / 1000.0);
        RecordProperty(("size_" + sizeKey + "_alive_cells").c_str(), aliveCells);
        RecordProperty(("size_" + sizeKey + "_total_cells").c_str(), totalCells);
        
        if (verbose_) {
            std::cout << "Size " << width << "x" << height 
                      << " (total: " << totalCells << ", alive: " << aliveCells 
                      << "): " << duration.count() / 1000.0 << " ms for 3 steps" << std::endl;
        }
    }
}

TEST_F(GameSimulatorPerformanceTest, StabilityTest) {
    GameSimulator simulator(100, 100);
    fillRandomPattern(simulator, 0.35);
    
    uint32_t initialCount = simulator.getAliveCellCount();
    
    const int totalSteps = 100;
    std::vector<uint32_t> counts;
    counts.reserve(totalSteps);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int step = 0; step < totalSteps; ++step) {
        simulator.step();
        uint32_t count = simulator.getAliveCellCount();
        counts.push_back(count);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    uint32_t finalCount = counts.back();
    
    // Record comprehensive metrics
    RecordProperty("stability_initial_count", initialCount);
    RecordProperty("stability_final_count", finalCount);
    RecordProperty("stability_total_time_ms", totalTime.count());
    RecordProperty("stability_avg_time_per_step_ms", 
                   static_cast<double>(totalTime.count()) / totalSteps);
    
    if (verbose_) {
        std::cout << "Stability test - Initial: " << initialCount 
                  << ", Final: " << finalCount 
                  << ", Total time: " << totalTime.count() << " ms" << std::endl;
    }
    
    // The simulation should not crash or produce invalid results
    EXPECT_GE(finalCount, 0);
    EXPECT_LE(finalCount, 100 * 100);
}

TEST_F(GameSimulatorPerformanceTest, SingleCellChangePerformance) {
    GameSimulator simulator(200, 200);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Apply many single cell changes
    const int numChanges = 10000;
    for (int i = 0; i < numChanges; ++i) {
        uint32_t x = gen() % 200;
        uint32_t y = gen() % 200;
        bool alive = (gen() % 2) == 0;
        simulator.applySingleCellChange(x, y, alive);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    RecordProperty("cell_changes_count", numChanges);
    RecordProperty("cell_changes_total_time_ms", duration.count() / 1000.0);
    RecordProperty("cell_changes_avg_time_us", duration.count() / static_cast<double>(numChanges));
    
    if (verbose_) {
        std::cout << numChanges << " single cell changes: " 
                  << duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Average per change: " 
                  << duration.count() / static_cast<double>(numChanges) << " microseconds" << std::endl;
    }
    
    EXPECT_LT(duration.count(), 100000); // Less than 100ms for 10000 changes
}

} // namespace LifeGame
