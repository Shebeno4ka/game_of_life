#pragma once

#include "MPSCQueue.h"

#include <future>

namespace LifeGame {

// EventToken — объект, на котором можно вызвать `wait()` и дождаться сигнала.
class EventToken {
private:
    std::future<void> future_;

public:
    explicit EventToken(std::future<void> future) : future_(std::move(future)) {}

    EventToken(const EventToken&) = delete;
    EventToken& operator=(const EventToken&) = delete;
    EventToken(EventToken&&) = default;
    EventToken& operator=(EventToken&&) = default;

    // Ждёт, пока событие не будет установлено
    void wait() { future_.wait(); }
};


// EventQueue — очередь событий: можно зарегистрировать событие, и затем его установить.
class EventQueue {
private:
    MPSCQueue<std::promise<void>> promiseQueue_;

public:
    void waitUntilNotEmpty() {promiseQueue_.waitWhileEmpty();}

    bool isActive() const {return !promiseQueue_.isClosed();}

    // Пытается взять следующее событие из очереди и установить его
    bool trySetNextEvent();

    // Добавляет новое событие в очередь, возвращает объект ожидания
    EventToken registerEvent();

    // Закрывает очередь: новые события не принимаются
    void close() {promiseQueue_.close();}
};

}