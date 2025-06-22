#include <gtest/gtest.h>
#include "server/GameServer.h"
#include "utils/SandNetworkDriver.h"
#include "utils/SandStepStrategy.h"
#include "utils/SandGameSimulator.h"
#include "core/BitField.h"
#include "core/GameSimulator.h"
#include "utils/SimulationPatterns.h"

using namespace LifeGame;
using namespace std::chrono_literals;

namespace LifeGame {

class GameServerTest : public ::testing::Test {
protected:
    uint32_t fieldWidth;
    uint32_t fieldHeight;
    std::unique_ptr<utils::SandNetworkDriver> networkDriver;
    utils::SandNetworkDriver::Handle networkHandle;
    std::unique_ptr<utils::SandStepStrategy> stepStrategy;
    utils::SandStepStrategy::Handle stepHandle;
    std::unique_ptr<GameSimulator> simulator;
    std::unique_ptr<GameServer<utils::SandNetworkDriver, utils::SandStepStrategy>> server;
    std::unique_ptr<utils::SandGameSimulator> sandSimulator;

    void SetUp() override {
        initialize(10, 10); // Default dimensions
    }

    void TearDown() override {
        if (server && server->isRunning()) {
            server->stop();
        }
        server.reset();
    }

    void initialize(uint32_t width, uint32_t height) {
        fieldWidth = width;
        fieldHeight = height;

        // Create network driver
        networkDriver = std::make_unique<utils::SandNetworkDriver>();
        networkHandle = networkDriver->getHandle();

        // Create step strategy
        stepStrategy = std::make_unique<utils::SandStepStrategy>();
        stepHandle = stepStrategy->getHandle();

        // Create a simulator with specified dimensions
        simulator = std::make_unique<GameSimulator>(fieldWidth, fieldHeight);

        // Create reference simulator with the same dimensions
        sandSimulator = std::make_unique<utils::SandGameSimulator>(fieldWidth, fieldHeight);

        // Create server
        server = std::make_unique<GameServer<utils::SandNetworkDriver, utils::SandStepStrategy>>(
            std::move(networkDriver),
            std::move(simulator),
            std::move(stepStrategy),
            100ms
        );
    }

    void startServerWithPattern(const std::vector<utils::CellState>& pattern) {
        BitField initialPattern(10, 10);
        utils::applyPattern(initialPattern, pattern);

        server->setInitialPattern(initialPattern);
        sandSimulator->setInitialPattern(initialPattern);
        server->start();
    }

    void startServerWithPattern(const BitField& field) {
        server->setInitialPattern(field);
        sandSimulator->setInitialPattern(field);
        server->start();
    }

    void makeStep() {
        stepHandle.makeSimulatorSteps(1).wait();
        sandSimulator->step();
    }
};

TEST_F(GameServerTest, StartAndStop) {
    EXPECT_FALSE(server->isRunning());
    
    server->start();
    EXPECT_TRUE(server->isRunning());
    
    server->stop();
    EXPECT_FALSE(server->isRunning());
}

TEST_F(GameServerTest, ProcessClientMessages) {
    // Send a cell change from client
    std::vector<CellChange> changes = {
        {5, 5, true},
        {6, 6, true}
    };

    BitField expectedField(10, 10);
    for (auto [x, y, alive]: changes) {
        expectedField.setAlive(x, y, alive);
    }

    server->start();
    networkHandle.sendToServer(changes);
    stepHandle.registerUserEvents(2);
    stepHandle.makeSimulatorSteps(1).wait();


    EXPECT_EQ(expectedField.serialize(), server->getField());
}

TEST_F(GameServerTest, SendsUpdatesToClients) {
    startServerWithPattern(utils::Patterns::glider(2, 2));

    makeStep();
    
    // Check that server sent updates to clients
    ASSERT_GE(networkHandle.fromServerData().size(), 1);
    
    // Verify that the data sent by the server matches what we expect from our reference simulator
    std::vector<std::byte> expectedData = sandSimulator->getStateData();
    const auto& actualData = networkHandle.fromServerData().back();
    EXPECT_EQ(actualData, expectedData);
}

TEST_F(GameServerTest, ProcessesBlinkerPattern) {
    startServerWithPattern(utils::Patterns::blinker(4, 4));

    makeStep();
    
    // Check first state
    ASSERT_GE(networkHandle.fromServerData().size(), 1);
    auto firstUpdate = networkHandle.fromServerData().back();
    auto expectedFirstUpdate = sandSimulator->getStateData();
    EXPECT_EQ(firstUpdate.size(), expectedFirstUpdate.size());
    
    // Second step
    makeStep();
    
    // Check second state (should be back to original orientation)
    ASSERT_GE(networkHandle.fromServerData().size(), 2);
    auto secondUpdate = networkHandle.fromServerData().back();
    auto expectedSecondUpdate = sandSimulator->getStateData();
    EXPECT_EQ(secondUpdate.size(), expectedSecondUpdate.size());
}

TEST_F(GameServerTest, ProcessesRandomField) {
    std::vector<double> densities = {0.1, 0.3, 0.5, 0.7, 0.9};
    for (double density : densities) {
        initialize(100, 100); // Use 100x100 field dimensions for this test
        BitField randomField(fieldWidth, fieldHeight);
        utils::fillRandom(randomField, density);

        startServerWithPattern(randomField);

        for (int step = 0; step < 100; ++step) {
            stepHandle.makeSimulatorSteps(1).wait();
            sandSimulator->step();

            ASSERT_GE(networkHandle.fromServerData().size(), step + 1);
            auto serverUpdate = networkHandle.fromServerData().back();
            auto refUpdate = sandSimulator->getStateData();

            EXPECT_EQ(serverUpdate.size(), refUpdate.size());
            EXPECT_EQ(serverUpdate, refUpdate);
        }

        server->stop();
    }
}

}