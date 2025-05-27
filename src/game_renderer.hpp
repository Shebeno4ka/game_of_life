#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

#include <optional>

#include "game_runner.hpp"
#include "thread_safe_game_state.hpp"

using namespace std::chrono_literals;

constexpr uint16_t kCellPixels = 10;
constexpr SDL_Color kBackgroundColor = {0, 0, 0, 255};
constexpr SDL_Color kCellColor = {255, 255, 255, 255};
constexpr SDL_Color kHoveredCellColor = {255, 0, 0, 128};

/**
 * \class GameRenderer
 * \brief Handles rendering of the game field and user interactions.
 */
class GameRenderer {
  std::shared_ptr<ThreadSafeGameState> state_thread_guard_ptr_;
  std::shared_ptr<SDL_Renderer> renderer_ptr_;
  std::unique_ptr<GameRunner> game_runner_;
  std::optional<std::pair<uint32_t, uint32_t>> hovered_cell_coords_;

 public:
  /**
   * \brief Constructs a GameRenderer instance.
   * \param state_ptr Shared pointer to thread-safe game state.
   * \param renderer_ptr Shared pointer to SDL renderer.
   * \param game_runner Unique pointer to started game runner instance.
   */
  GameRenderer(std::shared_ptr<ThreadSafeGameState> state_ptr,
               std::shared_ptr<SDL_Renderer> renderer_ptr,
               std::unique_ptr<GameRunner> game_runner);

  /**
   * \brief Starts the rendering process.
   */
  void start();

 private:
  /**
   * \brief Handles user input events and updates the game state.
   * \param size_x Width of the game field in cells.
   * \param size_y Height of the game field in cells.
   * \param running Reference to the running state of the game loop.
   * \param paused Reference to the paused state of the game loop.
   */
  void handleEvents_(uint64_t size_x, uint64_t size_y, bool& running,
                     bool& paused);

  /**
   * \brief Renders the game field and other visual elements.
   */
  void render_();

  /**
   * \brief Draws the game field based on the current game state.
   */
  void drawField_();

  /**
   * \brief Highlights the currently hovered cell.
   */
  void drawHoveredCell_();
};