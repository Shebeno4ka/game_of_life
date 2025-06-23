#include "clients/IClient.h"
#include "clients/LatencyMonitoringClient.h"
#include "config/LoggerSetup.h"
#include "generators/IClientGenerator.h"
#include "generators/PassiveClientGenerator.h"
#include "generators/MixedClientGenerator.h"
#include "network/WebSocketServer.h"
#include "server/GameServer.h"

#include <spdlog/spdlog.h>
#include <boost/asio/io_context.hpp>

template <IClientGenerator ClientGenerator>
class Benchmark {
public:
    Benchmark(std::string address, ClientGenerator clientGenerator, uint32_t ioThreadCount = 4)
        : address_(std::move(address))
        , ioThreadCount_(std::max(1u, ioThreadCount))
        , ioContexts_(ioThreadCount)
        , logger_(spdlog::get("Benchmark") ? spdlog::get("Benchmark") : spdlog::default_logger())
        , clientGenerator_(std::move(clientGenerator))
        , isRunning_(false)
    {
        workGuards_.reserve(ioThreadCount_);
        for (auto& ctx : ioContexts_) {
            workGuards_.emplace_back(boost::asio::make_work_guard(ctx));
        }
    }

    std::future<void> start() {
        isRunning_.store(true);

        for (uint32_t i = 0; i < ioThreadCount_; ++i) {
            ioThreads_.emplace_back([&ctx = ioContexts_[i]]() {
                ctx.run();
            });
        }

        std::future<void> benchmarkStopEvent;
        auto monitorClient = std::make_unique<LatencyMonitoringClient>(
            ioContexts_.front(), std::chrono::milliseconds(120), benchmarkStopEvent);
        logger_->info("Created monitoring client: {}", monitorClient->name());
        monitorClient->start(address_);
        activeClients_.emplace_back(std::move(monitorClient));

        spawnerThread_ = std::thread([this]() { spawnClientsLoop(); });

        return benchmarkStopEvent;
    }

    void stop() {
        if (!isRunning_.exchange(false)) return;

        logger_->info("Stopping benchmark with {} clients", activeClients_.size());

        {
            std::lock_guard lock(activeClientsMutex_);
            for (auto& client : activeClients_) {
                client->stop();
            }
        }

        if (spawnerThread_.joinable())
            spawnerThread_.join();

        // Разрешить io_context завершиться
        for (auto& guard : workGuards_)
            guard.reset();

        for (auto& ctx : ioContexts_)
            ctx.stop();

        for (auto& thread : ioThreads_) {
            if (thread.joinable())
                thread.join();
        }

        for (auto& client : activeClients_) {
            client->stop();
        }

        logger_->info("Benchmark fully stopped");
    }

private:
    void spawnClientsLoop() {
        uint32_t index = 0;

        while (isRunning_.load()) {
            auto& context = ioContexts_[index % ioContexts_.size()];
            auto client = clientGenerator_.next(context);

            logger_->info("Created new client: {}. Clients cnt: {}", client->name(), index + 1);

            {
                std::lock_guard lock(activeClientsMutex_);
                client->start(address_);
                activeClients_.emplace_back(std::move(client));
            }

            ++index;
        }
    }

private:
    std::string address_;
    uint32_t ioThreadCount_;
    std::vector<boost::asio::io_context> ioContexts_;
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> workGuards_;
    std::vector<std::thread> ioThreads_;
    std::thread spawnerThread_;

    std::shared_ptr<spdlog::logger> logger_;
    ClientGenerator clientGenerator_;

    std::vector<std::unique_ptr<IClient>> activeClients_;
    std::mutex activeClientsMutex_;

    std::atomic<bool> isRunning_{false};
};


void startBench(uint32_t fieldSize) {
    std::string ip = "0.0.0.0";
    int port = 8080;
    auto ws_server_ptr = std::make_unique<network::WebSocketServer>(ip, port);
    auto gameSimulator = std::make_unique<LifeGame::GameSimulator>(fieldSize, fieldSize);
    auto stepStrategy = std::make_unique<LifeGame::FixedStepStrategy<100>>();
    LifeGame::GameServer gameServer(std::move(ws_server_ptr), std::move(gameSimulator), std::move(stepStrategy));

    gameServer.start();

    MixedClientGenerator<10> generator(std::chrono::milliseconds(100), fieldSize, fieldSize, 0.05, std::chrono::seconds(5));
    Benchmark benchmark("0.0.0.0:8080", generator, 1);
    auto f = benchmark.start();
    f.wait();

    benchmark.stop();
    gameServer.stop();
}

int main() {
    utils::setupLogging();
    startBench(50);
    return 0;
}


