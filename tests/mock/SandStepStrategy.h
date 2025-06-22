#pragma once

#include "sync/EventQueue.h"

namespace LifeGame::utils  {

/**
 * Токен, который позволяет дождаться выполнения сервером соответствующего шага.
 */
struct StepToken {
private:
    EventToken event_;
public:
    explicit StepToken(EventToken event) : event_(std::move(event)) {}

    // Блокирует выполнение, пока связанный шаг не будет завершён.
    void wait() {event_.wait();}
};

/**
 * Токен, который позволяет дождаться обработки пользовательского события.
 */
struct UserEventToken {
private:
    EventToken event_;
public:
    explicit UserEventToken(EventToken event) : event_(std::move(event)) {}

    // Блокирует выполнение, пока событие не будет обработано.
    void wait() {event_.wait();}
};

/**
 * Тестовая стратегия пошаговой симуляции — позволяет вручную управлять
 * выполнением шагов симуляции и пользовательских событий.
*/
class SandStepStrategy {
public:
    // Объект, чтобы управлять симуляцией.
    class Handle {
    private:
        SandStepStrategy* strategy_;
    public:
        explicit Handle(SandStepStrategy* strategy = nullptr) : strategy_(strategy) {}

        // Зарегистрировать симуляционные шаги.
        // Возвращает StepToken, который можно ждать в тесте.
        StepToken makeSimulatorSteps(uint32_t stepsCount) {  // TODO: rename to register
            if (stepsCount == 0) {
                std::future<void> readyFuture = std::async(std::launch::deferred, [] {});
                return StepToken(EventToken(std::move(readyFuture)));
            }
            for (uint32_t i = 0; i < stepsCount - 1; ++i) {
                strategy_->steps_.registerEvent();
            }
            return StepToken(strategy_->steps_.registerEvent());
        }

        // Зарегистрировать пользовательские события.
        // Возвращает UserEventToken, который можно ждать в тесте.
        UserEventToken registerUserEvents(uint32_t eventsCount) {
            if (eventsCount == 0) {
                std::future<void> readyFuture = std::async(std::launch::deferred, [] {});
                return UserEventToken(EventToken(std::move(readyFuture)));
            }
            for (uint32_t i = 0; i < eventsCount - 1; ++i) {
                strategy_->userEvents_.registerEvent();
            }
            return UserEventToken(strategy_->userEvents_.registerEvent());
        }
    };

private:
    EventQueue steps_;
    EventQueue userEvents_;
public:
    SandStepStrategy() = default;

    SandStepStrategy(const SandStepStrategy&) = delete;
    SandStepStrategy& operator=(const SandStepStrategy&) = delete;
    SandStepStrategy(SandStepStrategy&&) = delete;
    SandStepStrategy& operator=(SandStepStrategy&&) = delete;
    
    Handle getHandle() {return Handle(this);}
    
    void onStepStart() {
        steps_.waitUntilNotEmpty();
    }
    
    bool isStepComplete() {
        return !userEvents_.trySetNextEvent();
    }

    void onStepEnd() {
        steps_.trySetNextEvent();
    }

    void stop() {
        steps_.close();
        userEvents_.close();
    }
};

} // namespace LifeGame