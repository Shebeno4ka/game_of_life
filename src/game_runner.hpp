#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

class GameRunner {
  std::shared_ptr<ThreadSafeGameState> state_thread_guard_ptr_;
  const uint32_t updates_per_second_;

  std::atomic_bool running_;
  std::atomic_bool paused_;
  std::thread game_thread_;
  mutable std::mutex pause_mutex_;
  mutable std::condition_variable pause_cond_var_;

 public:
  GameRunner(std::shared_ptr<ThreadSafeGameState> state,
             uint32_t updates_per_second);
  GameRunner(const GameRunner&) = delete;
  GameRunner& operator=(const GameRunner&) = delete;
  ~GameRunner();

  void start();
  void stop();
  void pause();
  void resume();

 private:
  void updateLoop_();
};