#pragma once
#include "IClient.h"
#include "client/Client.h"
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

// клиент который периодически отправляет рандомные изменения серверу
template <typename Generator>
class RandomUpdatesClient : public IClient {
private:
    static constexpr const char* kName_ = "PassiveClient";
    uint32_t width_;
    uint32_t height_;
    Client client_;
    Generator generator_;
    boost::random::uniform_real_distribution<float> dist_;
    double density_;
    boost::random::uniform_int_distribution<uint32_t> sleepTimeDist_;
    boost::asio::io_context& ioContext_;
    std::atomic<bool> running_{false};
public:
    RandomUpdatesClient(
        uint32_t width,
        uint32_t height,
        boost::asio::io_context& ioContext,
        double density,
        std::chrono::seconds sendInterval,
        Generator generator)
        : width_(width)
        , height_(height)
        , client_(ioContext)
        , generator_(generator)
        , dist_(0.0f, 1.0f)
        , density_(density)
        , ioContext_(ioContext)
        , sleepTimeDist_(toMillis(sendInterval), toMillis(sendInterval) * 3 / 2) {}

    ~RandomUpdatesClient() override {
        assert(!running_.load() && "RandomUpdatesClient: клиент не был остановлен перед уничтожением");
    }

    void start(std::string address) override {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            client_.connect(std::move(address));
            boost::asio::co_spawn(ioContext_, clientRoutine(), boost::asio::detached);
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
    static uint32_t toMillis(std::chrono::seconds sec) {
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(sec).count());
    }

    boost::asio::awaitable<void> clientRoutine() {
        boost::asio::steady_timer timer(ioContext_);

        while (running_.load()) {
            auto delay = std::chrono::milliseconds(sleepTimeDist_(generator_));
            timer.expires_after(delay);
            co_await timer.async_wait(boost::asio::use_awaitable);

            client_.send(generateRandomUpdates());
        }
    }

    std::vector<std::pair<uint32_t, uint32_t>> generateRandomUpdates() {
        std::vector<std::pair<uint32_t, uint32_t>> updates;
        updates.reserve(static_cast<size_t>(width_ * height_ * density_ * 1.5)); // оптимизация: примерный размер

        for (uint32_t y = 0; y < height_; ++y) {
            for (uint32_t x = 0; x < width_; ++x) {
                if (dist_(generator_) < density_) {
                    updates.emplace_back(y, x);
                }
            }
        }
        return updates;
    }
};
