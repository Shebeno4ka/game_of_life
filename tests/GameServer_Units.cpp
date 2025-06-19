#include <gtest/gtest.h>
#include "server/GameServer.h"
#include "utils/SandNetworkDriver.h"
#include "utils/SandStepStrategy.h"
#include "utils/ReferenceGameSimulator.h"
#include "core/BitField.h"
#include "core/GameSimulator.h"
#include "utils/SimulationPatterns.h"

using namespace LifeGame;
using namespace std::chrono_literals;

namespace LifeGame {

class GameServerTest : public ::testing::Test {
protected:
    std::unique_ptr<utils::SandNetworkDriver> networkDriver;
    utils::SandNetworkDriver::Handle networkHandle;
    std::unique_ptr<utils::SandStepStrategy> stepStrategy;
    utils::SandStepStrategy::Handle stepHandle;
    std::unique_ptr<GameSimulator> simulator;
    std::unique_ptr<GameServer<utils::SandNetworkDriver, utils::SandStepStrategy>> server;
    std::unique_ptr<utils::ReferenceGameSimulator> refSimulator;

    void SetUp() override {
        // Create network driver
        networkDriver = std::make_unique<utils::SandNetworkDriver>();
        networkHandle = networkDriver->getHandle();

        // Create step strategy
        stepStrategy = std::make_unique<utils::SandStepStrategy>();
        stepHandle = stepStrategy->getHandle();

        // Create a simple simulator with 10x10 field
        simulator = std::make_unique<GameSimulator>(10, 10);
        
        // Create reference simulator with the same dimensions
        refSimulator = std::make_unique<utils::ReferenceGameSimulator>(10, 10);

        // Create server
        server = std::make_unique<GameServer<utils::SandNetworkDriver, utils::SandStepStrategy>>(
            std::move(networkDriver),
            std::move(simulator),
            std::move(stepStrategy),
            100ms
        );
    }

    void TearDown() override {
        if (server && server->isRunning()) {
            server->stop();
        }
        server.reset();
    }
};

TEST_F(GameServerTest, StartAndStop) {
    // Test that server starts and stops correctly
    EXPECT_FALSE(server->isRunning());
    
    server->start();
    EXPECT_TRUE(server->isRunning());
    
    server->stop();
    EXPECT_FALSE(server->isRunning());
}

TEST_F(GameServerTest, ProcessClientMessages) {
    server->start();
    
    // Send a cell change from client
    std::vector<CellChange> changes = {
        {5, 5, true},
        {6, 6, true}
    };
    
    // Send message to server
    networkHandle.sendToServer(changes);
    
    // Complete the step to process the event
    stepHandle.completeStep();
    
    // Wait a bit for the server to process
    std::this_thread::sleep_for(50ms);
    
    // Verify the message was received by checking the network driver's data
    ASSERT_GE(networkHandle.fromClientData().size(), 1);
    EXPECT_EQ(networkHandle.fromClientData().back().size(), 2);
    
    server->stop();
}

TEST_F(GameServerTest, SendsUpdatesToClients) {
    // Use a pattern from SimulationPatterns
    BitField initialPattern(10, 10);
    utils::applyPattern(initialPattern, utils::Patterns::glider(2, 2));

    server->setInitialPattern(initialPattern);
    
    // Initialize reference simulator with the same pattern
    refSimulator->setInitialPattern(initialPattern);
    stepHandle.completeStep(1);
    
    // Start server
    server->start();
    
    // Simulate the same step in the reference simulator
    refSimulator->step();
    
    // Wait a bit for the server to process
    std::this_thread::sleep_for(50ms);
    
    // Check that server sent updates to clients
    ASSERT_GE(networkHandle.fromServerData().size(), 1);
    EXPECT_FALSE(networkHandle.fromServerData().back().empty());
    
    // Verify that the data sent by the server matches what we expect from our reference simulator
    std::vector<std::byte> expectedData = refSimulator->getStateData();
    const auto& actualData = networkHandle.fromServerData().back();
    
    EXPECT_EQ(actualData.size(), expectedData.size());
    EXPECT_TRUE(std::equal(actualData.begin(), actualData.end(), expectedData.begin()));

    
    server->stop();
}

TEST_F(GameServerTest, ProcessesBlinkerPattern) {
    // Use the blinker pattern (oscillator)
    BitField initialPattern(10, 10);
    utils::applyPattern(initialPattern, utils::Patterns::blinker(4, 4));

    server->setInitialPattern(initialPattern);
    
    // Initialize reference simulator with the same pattern
    refSimulator->setInitialPattern(initialPattern);
    
    // Start server
    server->start();
    
    // Run two steps to see the oscillation
    stepHandle.completeStep();
    refSimulator->step();
    
    std::this_thread::sleep_for(50ms);
    
    // Check first state
    ASSERT_GE(networkHandle.fromServerData().size(), 1);
    auto firstUpdate = networkHandle.fromServerData().back();
    auto expectedFirstUpdate = refSimulator->getStateData();
    EXPECT_EQ(firstUpdate.size(), expectedFirstUpdate.size());
    
    // Second step
    stepHandle.completeStep();
    refSimulator->step();
    
    std::this_thread::sleep_for(50ms);
    
    // Check second state (should be back to original orientation)
    ASSERT_GE(networkHandle.fromServerData().size(), 2);
    auto secondUpdate = networkHandle.fromServerData().back();
    auto expectedSecondUpdate = refSimulator->getStateData();
    EXPECT_EQ(secondUpdate.size(), expectedSecondUpdate.size());
    
    server->stop();
}

}