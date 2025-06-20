#include "EventQueue.h"


bool LifeGame::EventQueue::trySetNextEvent() {
    auto result = promiseQueue_.tryPop();
    if (!result.has_value()) {
        return false;
    }
    result->set_value(); // Сигналим: событие установлено
    return true;
}

LifeGame::EventToken LifeGame::EventQueue::registerEvent() {
    std::promise<void> promise;
    auto future = promise.get_future();
    promiseQueue_.push(std::move(promise));
    return EventToken(std::move(future));
}
