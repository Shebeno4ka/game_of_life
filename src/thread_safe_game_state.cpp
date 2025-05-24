#include "thread_safe_game_state.hpp"

ThreadSafeGameState::FieldGuard::FieldGuard(
    std::shared_mutex& mutex,
    const std::shared_ptr<GameState>& state)
    : lock_(mutex), field_(state->getField()) {}

ThreadSafeGameState::ThreadSafeGameState(std::shared_ptr<GameState> state)
    : state_(std::move(state)) {}

const GameState::field_t& ThreadSafeGameState::FieldGuard::getField() const {
  return field_;
}

void ThreadSafeGameState::update() {
  std::lock_guard lock(mutex_);
  state_->update();
}

void ThreadSafeGameState::toggleCell(uint32_t i, uint32_t j) {
  std::lock_guard lock(mutex_);
  state_->toggleCell(i, j);
}

const ThreadSafeGameState::FieldGuard ThreadSafeGameState::getField() const {
  return FieldGuard(mutex_, state_);
}

void ThreadSafeGameState::reset() {
  std::lock_guard lock(mutex_);
  state_->reset();
}