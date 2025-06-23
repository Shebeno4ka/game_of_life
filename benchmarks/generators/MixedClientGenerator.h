#pragma once

#include "clients/FieldPrinterClient.h"
#include "clients/RandomUpdatesClient.h"
#include <boost/random/taus88.hpp>
#include <boost/random/seed_seq.hpp>

template <uint32_t EveryNthRandomClient>
class MixedClientGenerator {
public:
    MixedClientGenerator(
        std::chrono::milliseconds genInterval,
        uint32_t width,
        uint32_t height,
        double density,
        std::chrono::seconds sendInterval)
        : genInterval_(genInterval)
        , width_(width)
        , height_(height)
        , density_(density)
        , sendInterval_(sendInterval)
        , counter_(0)
    {}

    std::unique_ptr<IClient> next(boost::asio::io_context& ioContext) {
        std::this_thread::sleep_for(genInterval_);
        ++counter_;
        static constexpr uint64_t seedsCnt = 500;
        if (counter_ % EveryNthRandomClient == 0) {
            // Стабильные сиды для воспроизводимости
            static const std::array<uint64_t, seedsCnt> seeds = []{
                std::array<uint64_t, seedsCnt> s{};
                uint64_t base = 0x9E3779B97F4A7C15ull;  // золотое число (Knuth's constant)
                for (size_t i = 0; i < seedsCnt; ++i) {
                    s[i] = base ^ (i * 0x41C64E6D) + (i << 16);
                }
                return s;
            }();

            using Generator = boost::random::taus88;

            Generator generator(seeds[counter_ % seedsCnt]);

            return std::make_unique<RandomUpdatesClient<Generator>>(
                width_, height_, ioContext, density_, sendInterval_, generator);
        } else {
            return std::make_unique<PassiveClient>(ioContext);
        }
    }

private:
    std::chrono::milliseconds genInterval_;
    uint32_t width_;
    uint32_t height_;
    double density_;
    std::chrono::seconds sendInterval_;
    uint32_t counter_;
};
