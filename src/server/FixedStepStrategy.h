#pragma once

#include <chrono>

namespace LifeGame {

/**
 * Ограничевает количество шагов до 1 в kStepDurationMs
 */
template <uint64_t kStepDurationMs>
class FixedStepStrategy {
    static constexpr std::chrono::milliseconds kDuration{kStepDurationMs};
    std::chrono::steady_clock::time_point stepStartTime_;

   public:
    void onStepStart() {
        stepStartTime_ = std::chrono::steady_clock::now();
    }

    bool isStepComplete() const {
        return (std::chrono::steady_clock::now() >= stepStartTime_ + kDuration);
    }

    void stop() {}
};

} // namespace LifeGame