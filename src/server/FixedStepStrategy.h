#pragma once

#include <chrono>

namespace LifeGame {

/**
 * Стратегия при которой шаг симуляции выполняется каждые kStepDurationMs. (постоянный FPS)
 */
template <uint64_t kStepDurationMs>
class FixedStepStrategy {
    static constexpr std::chrono::milliseconds kDuration{kStepDurationMs};
    std::chrono::steady_clock::time_point stepStartTime_;

   public:

    // StepControlStrategy concept
    void onStepStart() {
        stepStartTime_ = std::chrono::steady_clock::now();
    }

    bool isStepComplete() const {
        return (std::chrono::steady_clock::now() >= stepStartTime_ + kDuration);
    }

    void onStepEnd() {}
    void stop() {}
};

} // namespace LifeGame