#include "game_runner.hpp"

GameRunner::GameRunner(std::shared_ptr<ThreadSafeGameState> state,
                       const uint32_t updates_per_second)
    : state_(state), updates_per_second_(updates_per_second) {}

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
  paused_ = true;
}

void GameRunner::resume() {
  paused_ = false;
  pause_cond_var_.notify_one();
}

void GameRunner::updateLoop_() {
  double delta_time;
  auto last_time = std::chrono::steady_clock::now();
  auto current_time = last_time;
  while (running_.load()) {
    if (paused_.load()) {
      std::unique_lock<std::mutex> lock(pause_mutex_);
      pause_cond_var_.wait(
          lock, [this] { return !paused_.load() || !running_.load(); });
      if (!running_.load())
        break;
      last_time = std::chrono::steady_clock::now();
    }

    // Calculating how much updates we need to do since the last update
    current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = current_time - last_time;
    delta_time = elapsed_time.count();
    if (delta_time >= 1.0 / updates_per_second_) {
      while (delta_time >= 1.0 / updates_per_second_) {
        state_->update();
        delta_time -= 1.0 / updates_per_second_;
      }
      last_time = current_time;
    } else {
      std::this_thread::sleep_for(3ms);
    }
  }
}