#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

/**
 * \class GameRunner
 * \brief Manages the game loop and state updates.
 *
 * This class is responsible for running the game loop at a specified
 * update rate, handling pause and resume functionality, and ensuring
 * thread-safe access to the game state.
 */
class GameRunner {
  std::shared_ptr<ThreadSafeGameState> state_thread_guard_ptr_;
  const uint32_t updates_per_second_;
  std::atomic_bool running_;
  std::atomic_bool paused_;
  std::thread game_thread_;
  mutable std::mutex pause_mutex_;
  mutable std::condition_variable pause_cond_var_;

 public:
  /**
   * \brief Constructs a GameRunner instance.
   * \param state Shared pointer to the thread-safe game state.
   * \param updates_per_second Number of updates per second for the game loop.
   */
  GameRunner(std::shared_ptr<ThreadSafeGameState> state,
             uint32_t updates_per_second);
  ~GameRunner();

  /**
   * \brief Starts the game loop.
   */
  void start();

  /**
   * \brief Stops the game loop.
   */
  void stop();

  /**
   * \brief Pauses the game loop.
   */
  void pause();

  /**
   * \brief Resumes the game loop if it is paused.
   */
  void resume();

 private:
  /**
   * \brief The main update loop for the game.
   *
   * This function runs in a separate thread and updates the game state
   * at the specified rate.
   */
  void updateLoop_();
};