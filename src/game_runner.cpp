#include "game_runner.hpp"

GameRunner::GameRunner(GameState&& state,
                       const uint32_t updates_per_second,
                       const uint32_t updates_send_per_second)
    : filtered_event_queue_(EVENT_QUEUE_SIZE),
      state_(std::move(state)),
      updates_per_second_(updates_per_second),
      updates_send_per_second_(updates_send_per_second),
      running_(false),
      paused_(true) {}

GameRunner::~GameRunner() {
  stop();
}

void GameRunner::start() {
  running_ = true;
  game_thread_ = std::thread(&GameRunner::updateLoop_, this);
  event_handler_thread_ = std::thread(&GameRunner::handleEventsLoop_, this);
}

void GameRunner::stop() {
  running_ = false;
  paused_ = false;
  pause_cond_var_.notify_all();
  if (game_thread_.joinable())
    game_thread_.join();
}

void GameRunner::addEvent(GameEvent&& event) {
  filtered_event_queue_.push(event);
}

void GameRunner::handleEventsLoop_() {
  while (running_.load()) {
    GameEvent event;
    filtered_event_queue_.consume_one(
        [this](GameEvent&& event) { processEvent_(std::move(event)); });
    for (auto& callback : field_update_callbacks_) {
      auto field_guard = state_.getStateGuard();
      callback(field_guard.get().getField());
    }
  }
}

void GameRunner::processEvent_(GameEvent&& event) {
  std::visit(
      [this](const auto& e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, AddCellEvent>) {
          auto field_guard = state_.getStateGuard();
          field_guard.get().toggleCell(e.x, e.y);
        } else if constexpr (std::is_same_v<T, PauseEvent>) {
          if (!running_.load())
            return;  // Ignore pause if not running
          paused_.store(true);
        } else if constexpr (std::is_same_v<T, UnPauseEvent>) {
          if (!running_.load())
            return;  // Ignore unpause if not running
          paused_.store(false);
          pause_cond_var_.notify_all();
        }
      },
      event);
}

void GameRunner::updateLoop_() {
  auto last_time = std::chrono::steady_clock::now();
  auto do_update = [&, this] {
    // Getting access to the game state
    auto field_guard = state_.getStateGuard();
    auto& state = field_guard.get();

    // Calculating how much updates we need to do since the last update
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = current_time - last_time;
    double delta_time = elapsed_time.count();
    if (delta_time >= 1.0 / updates_per_second_) {
      // Perform updates until we catch up with the elapsed time
      while (delta_time >= 1.0 / updates_per_second_) {
        state.update();
        delta_time -= 1.0 / updates_per_second_;
      }
      last_time = current_time;
    }
  };

  while (running_.load()) {
    if (paused_.load()) {
      std::unique_lock lock(pause_mutex_);
      pause_cond_var_.wait(
          lock, [this] { return !paused_.load() || !running_.load(); });
      if (!running_.load())
        break;
      last_time = std::chrono::steady_clock::now();
    }

    do_update();
  }
}