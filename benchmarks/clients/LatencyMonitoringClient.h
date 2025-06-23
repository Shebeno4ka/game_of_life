#pragma once

#include "IClient.h"
#include "client/Client.h"
#include "sync/AsyncUtils.h"

#include <atomic>
#include <chrono>
#include <vector>
#include <boost/asio.hpp>

#include <optional>

class LatencyMonitoringClient : public IClient {
    static constexpr const char* kName_ = "LatencyMonitoringClient";
public:
    LatencyMonitoringClient(
        boost::asio::io_context& ioContext,
        std::chrono::milliseconds maxAllowedLatency,
        std::future<void>& benchmarkStopEvent)
        : logger_(spdlog::get("Client")),
          client_(ioContext),
          ioContext_(ioContext),
          maxLatency_(maxAllowedLatency),
          benchmarkStopEvent_()
    {
        if (!logger_)
            logger_ = spdlog::get("Client");
        benchmarkStopEvent = benchmarkStopEvent_.get_future();
        client_.setOnServerMessageCallback(
            [this](std::vector<std::byte> msg) {
                this->onMessage();
            });
    }

    ~LatencyMonitoringClient() override {
        assert(!running_.load() && "LatencyMonitoringClient: клиент не был остановлен перед уничтожением");
    }

    void start(std::string address) override {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            client_.connect(std::move(address));
        }
    }

    void stop() override {
        bool expected = true;
        if (running_.compare_exchange_strong(expected, false)) {
            client_.disconnect();
        }
    }

    std::string_view name() const override { return {kName_}; }

private:
    void onMessage() {
        if (!running_.load()) return;

        if (!lastMessageTime_.has_value()) {
            lastMessageTime_ = std::chrono::steady_clock::now();
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto delta = now - *lastMessageTime_;
        lastMessageTime_ = now;

        if (delta > maxLatency_) {
            logger_->warn("Latency exceeded: {}ms", std::chrono::duration_cast<std::chrono::milliseconds>(delta).count());
            stop();

            // Устанавливаем сигнал и будем ждущий поток
            benchmarkStopEvent_.set_value();
        }
    }

    Client::Loger logger_;
    Client client_;
    boost::asio::io_context& ioContext_;
    std::chrono::milliseconds maxLatency_;
    std::optional<std::chrono::steady_clock::time_point> lastMessageTime_;
    std::promise<void> benchmarkStopEvent_;
    std::atomic<bool> running_{false};
};
