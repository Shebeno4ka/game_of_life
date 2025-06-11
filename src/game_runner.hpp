#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include <boost/lockfree/queue.hpp>

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
  lockfree_queue event_queue_;
  const uint32_t updates_per_second_;
  std::atomic_bool running_;
  std::atomic_bool paused_;
  std::thread game_thread_;
  mutable std::mutex pause_mutex_;
  mutable std::condition_variable pause_cond_var_;
  ThreadSafeGameState state_;

 public:
  /**
   * \brief Constructs a GameRunner instance.
   * \param state Shared pointer to the thread-safe game state.
   * \param updates_per_second Number of updates per second for the game loop.
   */
  GameRunner(ThreadSafeGameState&& state, uint32_t updates_per_second);
  ~GameRunner();

  /**
   * \brief Starts the game loop.
   */
  void start();

  /**
   * \brief Stops the game loop.
   */
  void stop();

  void addEvent(GameEvent&& event);

  void pause();

  void resume();

 private:
  /**
   * \brief The main update loop for the game.
   *
   * This function runs in a separate thread and updates the game state
   * at the specified rate.
   */
  void updateLoop_();

  struct EventVisitor {
    void operator()(const AddCellEvent& e);

    void operator()(const PauseEvent& e);

    void operator()(const UnPauseEvent& e);
  };
};