#pragma once

#include <memory>
#include <shared_mutex>

#include "game_state.hpp"

class ThreadSafeGameState {
  mutable std::shared_mutex mutex_;
  std::shared_ptr<GameState> state_;

 public:
  class FieldGuard {
    std::shared_lock<std::shared_mutex> lock_;
    const GameState::field_t& field_;

   public:
    FieldGuard(std::shared_mutex& mutex,
               const std::shared_ptr<GameState>& state);

    [[nodiscard]] const GameState::field_t& getField() const;
  };

  explicit ThreadSafeGameState(std::shared_ptr<GameState> state);
  void update();
  void toggleCell(uint32_t i, uint32_t j);
  [[nodiscard]] FieldGuard getField() const;
  void reset();
};