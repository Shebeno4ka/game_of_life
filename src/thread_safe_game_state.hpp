#pragma once

#include <memory>
#include <shared_mutex>

#include "game_state.hpp"

/**
 * \class ThreadSafeGameState
 * \brief A thread-safe wrapper for managing access to a shared GameState
 * object.
 */
class ThreadSafeGameState {
  mutable std::mutex mutex_;
  std::shared_ptr<GameState> state_;

 public:
  /**
   * \brief Constructs a ThreadSafeGameState with the given GameState.
   * \param state A shared pointer to the GameState to be managed.
   */
  explicit ThreadSafeGameState(std::shared_ptr<GameState> state);

  /**
   * \class FieldGuard
   * \brief A RAII-style guard for safely accessing the GameState.
   */
  class FieldGuard {
    std::lock_guard<std::mutex> lock_;
    std::shared_ptr<GameState> state_;

   public:
    /**
     * \brief Constructs a FieldGuard, locking the provided mutex.
     * \param mutex The mutex to lock for thread-safe access.
     * \param state A shared pointer to the GameState to be accessed.
     */
    FieldGuard(std::mutex& mutex, std::shared_ptr<GameState> state);

    /**
     * \brief Provides access to the underlying GameState.
     * \return A reference to the GameState.
     * \note This method cannot be called on an rvalue FieldGuard.
     */
    [[nodiscard]] GameState& get() &;

    /**
     * \brief Deleted rvalue overload of get() to prevent unsafe access.
     */
    GameState& get() && = delete;
  };

  /**
   * \brief Creates a FieldGuard for accessing the GameState.
   * \return A FieldGuard object for safe access to the GameState.
   */
  FieldGuard getStateGuard() const;
};