#include "game_runner.hpp"

#include <utility>

GameRunner::GameRunner(std::shared_ptr<ThreadSafeGameState> state,
                       const uint32_t updates_per_second)
    : state_thread_guard_ptr_(std::move(state)),
      updates_per_second_(updates_per_second),
      running_(false),
      paused_(true) {}

GameRunner::~GameRunner() {
  stop();
}

void GameRunner::start() {
  running_ = true;
  game_thread_ = std::thread(&GameRunner::updateLoop_, this);
}

void GameRunner::stop() {
  running_ = false;
  paused_ = false;
  pause_cond_var_.notify_all();
  if (game_thread_.joinable())
    game_thread_.join();
}

void GameRunner::pause() {
  paused_.store(true);
}

void GameRunner::resume() {
  paused_.store(false);
  pause_cond_var_.notify_one();
}

void GameRunner::updateLoop_() {
  auto last_time = std::chrono::steady_clock::now();
  while (running_.load()) {
    if (paused_.load()) {
      std::unique_lock lock(pause_mutex_);
      pause_cond_var_.wait(
          lock, [this] { return !paused_.load() || !running_.load(); });
      if (!running_.load())
        break;
      last_time = std::chrono::steady_clock::now();
    }

    {
      // Getting access to the game state
      auto field_guard = state_thread_guard_ptr_->getStateGuard();
      auto& state = field_guard.get();

      // Calculating how much updates we need to do since the last update
      auto current_time = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed_time = current_time - last_time;
      double delta_time = elapsed_time.count();
      if (delta_time >= 1.0 / updates_per_second_) {
        while (delta_time >= 1.0 / updates_per_second_) {
          state.update();
          delta_time -= 1.0 / updates_per_second_;
        }
        last_time = current_time;
      }
    }
  }
}