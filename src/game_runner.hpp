#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include <boost/lockfree/queue.hpp>

#include <utility>
#include "game_event.hpp"
#include "game_state.hpp"
#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

using lockfree_queue = boost::lockfree::queue<
    GameEvent,
    boost::lockfree::allocator<std::allocator<GameEvent>>,  // стандартный
                                                            // аллокатор
    boost::lockfree::capacity<0>  // не фиксированный размер
    >;

constexpr size_t EVENT_QUEUE_SIZE = 100;

/**
 * \class GameRunner
 * \brief Manages the game loop and state updates.
 *
 * This class is responsible for running the game loop at a specified
 * update rate, handling pause and resume functionality, and ensuring
 * thread-safe access to the game state.
 */
class GameRunner {
  using FieldUpdateCallback =
      std::function<void(std::vector<std::vector<bool>>)>;

  lockfree_queue filtered_event_queue_;
  lockfree_queue event_queue_;
  const uint32_t updates_per_second_;
  std::atomic_bool running_;
  std::atomic_bool paused_;
  std::atomic_int32_t updates_send_per_second_;
  std::thread game_thread_;
  std::thread event_handler_thread_;
  mutable std::mutex pause_mutex_;
  mutable std::condition_variable pause_cond_var_;
  ThreadSafeGameState state_;
  std::vector<FieldUpdateCallback> field_update_callbacks_;

 public:
  /**
   * \brief Constructs a GameRunner instance.
   * \param state Shared pointer to the thread-safe game state.
   * \param updates_per_second Number of updates per second for the game loop.
   */
  GameRunner(GameState&& state,
             const uint32_t updates_per_second = 6,
             const uint32_t updates_send_per_second = 100);
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
   * \brief Adds an event to the event queue.
   */
  void addEvent(GameEvent&& event);

  /**
   * \brief Registers a callback to be called when game field changes.
   * \param callback The callback function to register.
   * The callback should accept a vector of booleans representing the field.
   * This callback will be called whenever the game field is updated.
   */
  void registerFieldUpdateCallback(FieldUpdateCallback callback);

 private:
  /**
   * \brief Update loop for the game.
   *
   * This function runs in a separate thread and updates the game state
   * at the specified rate.
   */
  void updateLoop_();

  /**
   * \brief Handles events in a separate thread.
   *
   * This function processes events from the event queue and updates
   * the game state accordingly.
   */
  void handleEventsLoop_();

  /**
   * \brief Processes a game event.
   *
   * This function is called by the event handler thread to process
   * events from the event queue.
   * \param event The game event to process.
   */
  void processEvent_(GameEvent&& event);
};