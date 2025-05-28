#include "thread_safe_game_state.hpp"

#include <utility>

ThreadSafeGameState::ThreadSafeGameState(std::shared_ptr<GameState> state)
    : state_(std::move(state)) {}

ThreadSafeGameState::FieldGuard::FieldGuard(std::mutex& mutex,
                                            std::shared_ptr<GameState> state)
    : lock_(mutex), state_(std::move(state)) {}

GameState& ThreadSafeGameState::FieldGuard::get() & {
  return *state_;
}

ThreadSafeGameState::FieldGuard ThreadSafeGameState::getStateGuard() const {
  return FieldGuard{mutex_, state_};
}