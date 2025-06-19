#pragma once

#include <atomic>

namespace LifeGame::utils  {

/**
 * Стратегия для тестов, позволяющая вручную завершать шаг через handle.
 */
class SandStepStrategy {
public:
    class Handle {
        SandStepStrategy* strategy_;

       public:
        Handle(): strategy_(nullptr) {}
        explicit Handle(SandStepStrategy* strategy) : strategy_(strategy) {}

        void completeStep(uint32_t stepCount=1) const {
            strategy_->stepsAvailable_.fetch_add(stepCount);
        }
    };
private:
    std::atomic<uint32_t> stepsAvailable_;

public:
    SandStepStrategy() : stepsAvailable_(0) {}

    void onStepStart() {}

    bool isStepComplete() {
        auto expected = stepsAvailable_.load(std::memory_order_relaxed);
        while (expected > 0) {
            if (stepsAvailable_.compare_exchange_weak(expected, expected-1)) {
                return true;
            }
        }
        return false;
    }

    void stop() {
        stepsAvailable_.store(1e9);
    }

    // Возвращает handler для управления завершением шага
    Handle getHandle() {
        return Handle(this);
    }
};

} // namespace LifeGame